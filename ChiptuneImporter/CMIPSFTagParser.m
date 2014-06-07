//
//  CMIPSFTagParser.m
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 07/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import "CMIPSFTagParser.h"
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


@implementation CMIPSFTagParser


+ alloc
{
  return [super alloc];
}


+ tagsWithFile:(FILE *)fp error:(BOOL*)err
{
  CMIPSFTagParser *tp;
  tp = [[CMIPSFTagParser alloc] initWithFile:fp error:err];
  return tp;
}


- init
{
  tagdict = NULL;
  tagend = NULL;
  enc = NSWindowsCP1252StringEncoding;
  return [super init];
}


- initWithFile:(FILE *)fp error:(BOOL*)err
{
  self = [self init];
  *err = [self setTagsFromFile:fp];
  return self;
}


- (void)dealloc {
  free(tagdict);
}


- (BOOL)setTagsFromFile:(FILE *)fp
{
  off_t tag_size;
  char temp[5];
  uint32_t header[4];
  
  rewind(fp);
  if (fread(header, 4, 4, fp) < 4) return NO;
  if (fseek(fp, (size_t)header[1]+(size_t)header[2]+16, SEEK_SET)) return NO;
  
  if (fread(temp, 1, 5, fp) < 5) return NO;
  if (memcmp(temp, "[TAG]", 5) != 0) return NO;
  
  tag_size = ftello(fp);
  fseek(fp, 0, SEEK_END);
  tag_size = ftello(fp) - tag_size;
  if (!(tagdict = (char*)malloc(tag_size))) return NO;
  fseek(fp, -tag_size, SEEK_END);
  fread(tagdict, 1, tag_size, fp);
  tagend = tagdict + tag_size;
  
  return YES;
}


- (NSMutableDictionary*)tagDictionary
{
  typedef enum {
    kKeyLeadSpaceSkip,
    kKeySkip,
    kGetEquals,
    kValueLeadSpaceSkip,
    kValueSkip,
    kGetNewline
  } tPsfParserState;
  
  NSMutableDictionary *intermdict;
  NSMutableDictionary *outdict;
  tPsfParserState state;
  char *tp = tagdict;
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
              key = [NSString stringWithCString:tag_namestart encoding:NSASCIIStringEncoding];
              if ((td = [intermdict valueForKey:key])) {
                [td appendBytes:"0x0A" length:1];
                [td appendBytes:tag_datastart length:tag_dataend-tag_datastart];
              } else
                [intermdict setValue:[NSMutableData dataWithBytes:tag_datastart length:tag_dataend-tag_datastart] forKey:key];
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
  
  outdict = [[NSMutableDictionary alloc] initWithCapacity:[intermdict count]];
  
  for (key in intermdict) {
    NSData *vin;
    NSString *vout;
    
    vin = [intermdict valueForKey:key];
    vout = [[NSString alloc] initWithData:vin encoding:enc];
    [outdict setValue:vout forKey:key];
  }
  
  return outdict;
}


@end
