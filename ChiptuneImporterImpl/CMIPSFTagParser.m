//
//  CMIPSFTagDictionary.m
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 07/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import "CMIPSFTagParser.h"
#include <stdio.h>
#include <limits.h>


NSNumber *CMIPSFDurationStringToSeconds(NSString *str) {
  long h=0, m=0;
  double secs=0;
  char *cs;
  char *cp;
  
  cs = strdup([str UTF8String]);
  cp = cs;
  while (*cp) {
    char *ce;
    
    ce = strpbrk(cp, ":.,");
    if (ce == NULL) {
      secs = strtod(cp, &cp);
      if (*cp != '\0')
        goto fail;
    } else if (*ce == ':') {
      h = m;
      m = strtol(cp, &cp, 10);
      if (cp != ce)
        goto fail;
      cp = ce+1;
    } else {
      *ce = '.';
      secs = strtod(cp, &cp);
      if (*cp != '\0')
        goto fail;
    }
  }
  secs += (double)(h * 3600 + m * 60);
  free(cs);
  
  return @(secs);

fail:
  free(cs);
  return nil;
}


NSData *PSFExtractTagDataFromFile(FILE *fp)
{
  NSData *res;
  off_t tag_size;
  char temp[5];
  uint32_t header[4];
  char *tagdict;
  
  rewind(fp);
  
  if (fread(header, 4, 4, fp) < 4)
    return nil;
  if (fseek(fp, (size_t)header[1]+(size_t)header[2]+16, SEEK_SET))
    return nil;
  
  if (fread(temp, 1, 5, fp) < 5)
    return nil;
  if (memcmp(temp, "[TAG]", 5) != 0)
    return nil;
  
  tag_size = ftello(fp);
  if (fseek(fp, 0, SEEK_END))
    return nil;
  tag_size = ftello(fp) - tag_size;
  if (!(tagdict = (char*)malloc(tag_size)))
    return nil;
  
  if (fseek(fp, -tag_size, SEEK_END))
    goto fail2;
  if (fread(tagdict, 1, tag_size, fp) < tag_size)
    goto fail2;
  
  res = [[NSData alloc] initWithBytesNoCopy:tagdict length:tag_size freeWhenDone:YES];
  return res;
  
fail2:
  free(tagdict);
  return nil;
}


NSDictionary *CMIPSFTagsDictionaryFromTagData(NSData *raw)
{
  unsigned char *tag_namestart = NULL, *tag_nameend = NULL;
  unsigned char *tag_datastart = NULL, *tag_dataend = NULL;
  unsigned char *tp, *tagend;
  NSMutableDictionary *intermdict;
  NSStringEncoding enc;
  NSMutableData *td;
  NSArray *keys;
  NSString *key, *val;
  enum {
    kKeyLeadSpaceSkip, kKeySkip, kGetEquals,
    kValueLeadSpaceSkip, kValueSkip, kGetNewline
  } state;
  
  tp = (unsigned char*)[raw bytes];
  tagend = tp + [raw length];
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
            key = [NSString stringWithUTF8String:(char*)tag_namestart];
            if ((td = [intermdict objectForKey:key])) {
              [td appendBytes:"\n" length:1];
            } else {
              td = [NSMutableData data];
              [intermdict setObject:td forKey:key];
            }
            [td appendBytes:tag_datastart length:tag_dataend-tag_datastart];
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
  
  if ([intermdict objectForKey:@"utf8"])
    enc = NSUTF8StringEncoding;
  else
    enc = NSWindowsCP1252StringEncoding;
  
  keys = [intermdict allKeys];
  for (key in keys) {
    td = [intermdict objectForKey:key];
    val = [[NSString alloc] initWithData:td encoding:enc];
    [intermdict setObject:val forKey:key];
  }
  
  return [intermdict copy];
}


NSDictionary *CMIPSFTagsDictionaryFromFile(FILE *fp)
{
  NSData *d = PSFExtractTagDataFromFile(fp);
  return CMIPSFTagsDictionaryFromTagData(d);
}

