//
//  MySpotlightImporter.m
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 02/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import "ChiptuneMDImporter.h"
#include <stdio.h>
#include <stdint.h>


@implementation ChiptuneMDImporter


//const uint8_t nsf_header[5] = {'N','E','S','M',0x1A};


- (BOOL)importFileAtPath:(NSString *)filePath attributes:(NSMutableDictionary *)spotlightData error:(NSError **)error
{
  char buf[32+1];
  const char *fn;
  FILE *fp;
  
  fn = [filePath cStringUsingEncoding:NSUTF8StringEncoding];
  if (!(fp = fopen(fn, "rb"))) {
    *error = [NSError errorWithDomain:NSPOSIXErrorDomain code:errno userInfo:nil];
    return NO;
  }
  
  buf[32] = '\0';
  
  fseek(fp, 0x0E, SEEK_SET);
  fread(buf, sizeof(char), 32, fp);
  [spotlightData setObject:[NSString stringWithCString:buf encoding:NSWindowsCP1252StringEncoding] forKey:(NSString*)kMDItemTitle];
  
  fseek(fp, 0x2E, SEEK_SET);
  fread(buf, sizeof(char), 32, fp);
  [spotlightData setObject:[NSArray arrayWithObject:[NSString stringWithCString:buf encoding:NSWindowsCP1252StringEncoding]] forKey:(NSString*)kMDItemAuthors];

  fseek(fp, 0x4E, SEEK_SET);
  fread(buf, sizeof(char), 32, fp);
  [spotlightData setObject:[NSString stringWithCString:buf encoding:NSWindowsCP1252StringEncoding] forKey:(NSString*)kMDItemCopyright];

  fclose(fp);
  
  return YES;
}


@end
