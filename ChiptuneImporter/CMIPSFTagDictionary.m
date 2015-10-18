//
//  CMIPSFTagDictionary.m
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 07/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import "CMIPSFTagDictionary.h"
#include <stdio.h>
#include <limits.h>


NSNumber *DurationStringToSeconds(NSString *str) {
  long h=0, m=0;
  double secs=0;
  char *cs;
  char *cp;
  
  cs = strdup([str cStringUsingEncoding:NSASCIIStringEncoding]);
  cp = cs;
  while (*cp) {
    char *ce;
    
    ce = strpbrk(cp, ":.,");
    if (ce == NULL)
      secs = strtod(cp, &cp);
    else if (*ce == ':') {
      h = m;
      m = strtol(cp, NULL, 10);
      cp = ce+1;
    } else {
      *ce = '.';
      secs = strtod(cp, &cp);
    }
  }
  secs += (float)(h * 3600 + m * 60);
  free(cs);
  
  return [NSNumber numberWithDouble:secs];
}


@implementation CMIPSFTagDictionary


- (instancetype)initWithObjects:(const id _Nonnull __unsafe_unretained *)objects forKeys:(const id<NSCopying> _Nonnull __unsafe_unretained *)keys count:(NSUInteger)cnt
{
  self = [super init];
  storage = [[NSDictionary alloc] initWithObjects:objects forKeys:keys count:cnt];
  return storage ? self : nil;
}


- (instancetype)initWithPSFFilePointer:(FILE *)fp
{
  NSData *raw;
  
  self = [super init];
  
  raw = [self rawTagDataFromFile:fp];
  if (!raw)
    return nil;
  
  storage = [self tagDictionaryFromRawTagData:raw];
  if (!storage)
    return nil;
  
  return self;
}


- (NSUInteger)count
{
  return [storage count];
}


- (id)objectForKey:(id)aKey
{
  return [storage objectForKey:aKey];
}


- (NSEnumerator *)keyEnumerator
{
  return [storage keyEnumerator];
}


- (NSData *)rawTagDataFromFile:(FILE *)fp
{
  NSData *res;
  off_t tag_size;
  char temp[5];
  uint32_t header[4];
  char *tagdict;
  
  rewind(fp);
  if (fread(header, 4, 4, fp) < 4) return nil;
  if (fseek(fp, (size_t)header[1]+(size_t)header[2]+16, SEEK_SET)) return nil;
  
  if (fread(temp, 1, 5, fp) < 5) return nil;
  if (memcmp(temp, "[TAG]", 5) != 0) return nil;
  
  tag_size = ftello(fp);
  fseek(fp, 0, SEEK_END);
  tag_size = ftello(fp) - tag_size;
  if (!(tagdict = (char*)malloc(tag_size))) return nil;
  fseek(fp, -tag_size, SEEK_END);
  fread(tagdict, 1, tag_size, fp);
  
  res = [[NSData alloc] initWithBytesNoCopy:tagdict length:tag_size freeWhenDone:YES];
  return res;
}


- (NSDictionary *)tagDictionaryFromRawTagData:(NSData *)raw
{
  typedef enum {
    kKeyLeadSpaceSkip,
    kKeySkip,
    kGetEquals,
    kValueLeadSpaceSkip,
    kValueSkip,
    kGetNewline
  } tPsfParserState;
  
  NSStringEncoding enc = NSWindowsCP1252StringEncoding;
  NSMutableDictionary *intermdict;
  NSArray *keys;
  tPsfParserState state;
  char *tp = (char*)[raw bytes];
  char *tagend = tp + [raw length];
  char *tag_namestart = NULL;
  char *tag_nameend = NULL;
  char *tag_datastart = NULL;
  char *tag_dataend = NULL;
  
  NSString *key;
  
  state = kKeyLeadSpaceSkip;
  intermdict = [[NSMutableDictionary alloc] init];
  
  /* This is quite slow actually, but it can't crash */
  while (tp < tagend) {
    switch (state) {
      case kKeyLeadSpaceSkip:
        if (*tp > ' ') {
          state = kKeySkip;
          tag_namestart = tp;
        } else
          tp++;
        break;
        
      case kKeySkip:
        if (*tp <= ' ' || *tp == '=') {
          if (*tp == 0x0A) {
            state = kKeyLeadSpaceSkip;
            tp++;
          } else {
            state = kGetEquals;
            tag_nameend = tp;
          }
        } else
          tp++;
        break;
        
      case kGetEquals:
        if (*tp > ' ') {
          if (*tp == '=') {
            state = kValueLeadSpaceSkip;
            tp++;
          } else
            state = kKeySkip;
        } else if (*tp++ == 0x0A)
          state = kKeyLeadSpaceSkip;
        break;
        
      case kValueLeadSpaceSkip:
        if (*tp > ' ') {
          state = kValueSkip;
          tag_datastart = tp;
        } else
          tp++;
        break;
        
      case kValueSkip:
        if (*tp <= ' ') {
          state = kGetNewline;
          tag_dataend = tp;
        } else
          tp++;
        break;
        
      case kGetNewline:
        if (*tp <= ' ') {
          if (*tp == 0x0A) {
            *tag_nameend = '\0';
            *tag_dataend = '\0';
            if (strcmp(tag_namestart , "utf8") == 0)
              enc = NSUTF8StringEncoding;
            else {
              NSMutableData *td;
              key = [NSString stringWithUTF8String:tag_namestart];
              if ((td = [intermdict objectForKey:key])) {
                [td appendBytes:"0x0A" length:1];
                [td appendBytes:tag_datastart length:tag_dataend-tag_datastart];
              } else {
                td = [NSMutableData dataWithBytes:tag_datastart length:tag_dataend-tag_datastart];
                [intermdict setObject:td forKey:key];
              }
            }
            state = kKeyLeadSpaceSkip;
          }
          tp++;
        } else
          state = kValueSkip;
        break;
        
      default:
        return nil;
    }
  }
  
  keys = [intermdict allKeys];
  for (key in keys) {
    NSData *vin;
    NSString *vout;
    
    vin = [intermdict objectForKey:key];
    vout = [[NSString alloc] initWithData:vin encoding:enc];
    [intermdict setObject:vout forKey:key];
  }
  
  return [intermdict copy];
}


@end
