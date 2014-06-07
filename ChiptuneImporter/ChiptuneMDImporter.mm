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

#import "ID666.h"


const CFStringRef kMDItemChiptuneOST = (CFStringRef)@"com_danielecattaneo_chiptunemdimporter_ost";
const CFStringRef kMDItemChiptuneDumper = (CFStringRef)@"com_danielecattaneo_chiptunemdimporter_dumper";

typedef enum {
  kNSF,
  kGBS,
  kUnsupported,
  kUnreadable
} tFileType;

typedef struct {
  tFileType ft;
  size_t size;
  const uint8_t *data;
} tMagic;


const uint8_t nsf_magic[] = {'N','E','S','M', 0x1A};
const uint8_t gbs_magic[] = {'G','B','S', 0x01};

const tMagic magics[] = {
  {kNSF, 5, nsf_magic},
  {kGBS, 4, gbs_magic},
  {kUnsupported, 0, NULL}
};


tFileType DetectFileType(FILE *fp) {
  int i;
  uint8_t buf[16];
  tFileType ft;
  
  errno = 0;
  rewind(fp);
  if (errno) return kUnreadable;
  
  if (fread(buf, sizeof(uint8_t), 16, fp) < 16)
    return kUnreadable;
  
  for (i=0; magics[i].ft != kUnsupported; i++)
    if (memcmp(buf, magics[i].data, magics[i].size) == 0)
      break;
  return ft = magics[i].ft;
}


void AddAttribute(NSMutableDictionary *sd, char *s, CFStringRef name) {
  if (*s != '\0')
    [sd setObject:[NSString stringWithCString:s encoding:NSWindowsCP1252StringEncoding] forKey:(__bridge NSString*)name];
}


void AddAttributeArray(NSMutableDictionary *sd, char *s, CFStringRef name) {
  if (*s != '\0')
    [sd setObject:[NSArray arrayWithObject:[NSString stringWithCString:s encoding:NSWindowsCP1252StringEncoding]] forKey:(__bridge NSString*)name];
}


void AddAttributeNumber(NSMutableDictionary *sd, double n, CFStringRef name) {
  [sd setObject:[NSNumber numberWithDouble:n] forKey:(__bridge NSString*)name];
}


@implementation ChiptuneMDImporter


- (BOOL)importFileAtPath:(NSString *)filePath attributes:(NSMutableDictionary *)spotlightData
{
  BOOL result;
  tFileType ft;
  const char *fn;
  FILE *fp;
  
  fn = [filePath cStringUsingEncoding:NSUTF8StringEncoding];
  if (!(fp = fopen(fn, "rb"))) return NO;
  
  if ([self attemptToImportSPC:fn attributes:spotlightData])
    return YES;
  
  ft = DetectFileType(fp);
  
  switch (ft) {
    case kNSF:
      result = [self importNSF:fp attributes:spotlightData];
      break;
    case kGBS:
      result = [self importGBS:fp attributes:spotlightData];
      break;
    default:
      result = NO;
  }

  fclose(fp);
  return result;
}


- (BOOL)importNSF:(FILE *)fp attributes:(NSMutableDictionary *)sd
{
  char buf[32+1];
  
  buf[32] = '\0';
  
  if (fseek(fp, 0x0E, SEEK_SET) < 0) return NO;
  if (fread(buf, sizeof(char), 32, fp) < 32) return NO;
  AddAttribute(sd, buf, kMDItemTitle);
  
  if (fseek(fp, 0x2E, SEEK_SET) < 0) return NO;
  if (fread(buf, sizeof(char), 32, fp) < 32) return NO;
  AddAttributeArray(sd, buf, kMDItemAuthors);
  
  if (fseek(fp, 0x4E, SEEK_SET) < 0) return NO;
  if (fread(buf, sizeof(char), 32, fp) < 32) return NO;
  AddAttribute(sd, buf, kMDItemCopyright);
  
  return YES;
}


- (BOOL)importGBS:(FILE *)fp attributes:(NSMutableDictionary *)sd
{
  char buf[32+1];
  
  buf[32] = '\0';
  
  if (fseek(fp, 0x10, SEEK_SET) < 0) return NO;
  if (fread(buf, sizeof(char), 32, fp) < 32) return NO;
  AddAttribute(sd, buf, kMDItemTitle);
  
  if (fseek(fp, 0x30, SEEK_SET) < 0) return NO;
  if (fread(buf, sizeof(char), 32, fp) < 32) return NO;
  AddAttributeArray(sd, buf, kMDItemAuthors);
  
  if (fseek(fp, 0x50, SEEK_SET) < 0) return NO;
  if (fread(buf, sizeof(char), 32, fp) < 32) return NO;
  AddAttribute(sd, buf, kMDItemCopyright);
  
  return YES;
}


- (BOOL)attemptToImportSPC:(const char *)fn attributes:(NSMutableDictionary *)sd
{
  char buf[80];
  int songlen;
  ID666 spctag;
  ID6Type res;
  
  res = spctag.LoadTag(fn, 0);
  if (res == ID6_UNK || res == ID6_ERR)
    return NO;
  
  AddAttribute(sd, spctag.song, kMDItemTitle);
  AddAttribute(sd, spctag.game, kMDItemAlbum);
  AddAttribute(sd, spctag.ost, kMDItemChiptuneOST);
  AddAttributeArray(sd, spctag.artist, kMDItemAuthors);
  AddAttribute(sd, spctag.comment, kMDItemInformation);
  AddAttributeArray(sd, spctag.pub, kMDItemPublishers);
  if (spctag.copy) {
    sprintf(buf, "%d", spctag.copy);
    AddAttribute(sd, buf, kMDItemCopyright);
  }
  songlen = spctag.GetSong();
  if (songlen != spctag.defSong) AddAttributeNumber(sd, songlen/64000.0, kMDItemDurationSeconds);
  AddAttribute(sd, spctag.dumper, kMDItemChiptuneDumper);
  
  return YES;
}


@end
