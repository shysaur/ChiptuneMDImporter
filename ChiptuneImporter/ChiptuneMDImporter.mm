//
//  MySpotlightImporter.m
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 02/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import "ChiptuneMDImporter.h"
#import "CMIPSFTagParser.h"
#include <stdio.h>
#include <stdint.h>

#import "ID666.h"


const CFStringRef kMDItemChiptuneOST =
        (CFStringRef)@"com_danielecattaneo_chiptunemdimporter_ost";
const CFStringRef kMDItemChiptuneDumper =
        (CFStringRef)@"com_danielecattaneo_chiptunemdimporter_dumper";
const CFStringRef kMDItemChiptuneExpansion =
        (CFStringRef)@"com_danielecattaneo_chiptunemdimporter_expansion";

typedef enum {
  kNSF,
  kGBS,
  kPSF,
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
const uint8_t psf_magic[] = {'P','S','F'};

const tMagic magics[] = {
  {kNSF, 5, nsf_magic},
  {kGBS, 4, gbs_magic},
  {kPSF, 3, psf_magic},
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
    [sd setObject:[NSString stringWithCString:s
                            encoding:NSWindowsCP1252StringEncoding]
        forKey:(__bridge NSString*)name];
}


void AddAttributeArray(NSMutableDictionary *sd, char *s, CFStringRef name) {
  if (*s != '\0')
    [sd setObject:[NSArray
                   arrayWithObject:[NSString
                                    stringWithCString:s
                                    encoding:NSWindowsCP1252StringEncoding]]
           forKey:(__bridge NSString*)name];
}


void AddAttributeNumber(NSMutableDictionary *sd, double n, CFStringRef name) {
  [sd setObject:[NSNumber numberWithDouble:n] forKey:(__bridge NSString*)name];
}


@implementation ChiptuneMDImporter


- (BOOL)importFileAtPath:(NSString *)filePath
              attributes:(NSMutableDictionary *)spotlightData
{
  BOOL result;
  tFileType ft;
  const char *fn;
  FILE *fp;
  
  fn = [filePath cStringUsingEncoding:NSUTF8StringEncoding];
  
  if ([self attemptToImportSPC:fn attributes:spotlightData])
    return YES;
  
  if (!(fp = fopen(fn, "rb"))) return NO;
  ft = DetectFileType(fp);
  
  switch (ft) {
    case kNSF:
      result = [self importNSF:fp attributes:spotlightData];
      break;
    case kGBS:
      result = [self importGBS:fp attributes:spotlightData];
      break;
    case kPSF:
      result = [self importPSF:fp attributes:spotlightData];
      break;
    default:
      result = NO;
  }

  fclose(fp);
  return result;
}


- (BOOL)importNSF:(FILE *)fp attributes:(NSMutableDictionary *)sd
{
  const char *exp[] = {
    "VRC6",
    "VRC7",
    "FDS",
    "MMC5",
    "Namco 106",
    "Sunsoft FME-07"
  };
  char header[0x80];
  char buf[256];
  int i, expm;
  
  if (fseek(fp, 0, SEEK_SET) < 0) return NO;
  if (fread(header, sizeof(char), 0x80, fp) < 0x80) return NO;
  
  buf[32] = '\0';
  memcpy(buf, header+0xE, 32);
  AddAttribute(sd, buf, kMDItemTitle);
  memcpy(buf, header+0x2E, 32);
  AddAttributeArray(sd, buf, kMDItemAuthors);
  memcpy(buf, header+0x4E, 32);
  AddAttribute(sd, buf, kMDItemCopyright);
  
  buf[0] = '\0';
  if (header[0x7B]) {
    expm = 1;
    for (i=0; i<6; i++, expm <<= 1) {
      if (expm & header[0x7B]) {
        if ((expm - 1) & header[0x7B])
          strcat(buf, ", ");
        strcat(buf, exp[i]);
      }
    }
    AddAttribute(sd, buf, kMDItemChiptuneExpansion);
  }
  
  return YES;
}


- (BOOL)importGBS:(FILE *)fp attributes:(NSMutableDictionary *)sd
{
  char header[0x70];
  char buf[32+1];
  
  if (fseek(fp, 0x00, SEEK_SET) < 0) return NO;
  if (fread(header, sizeof(char), 0x70, fp) < 0x70) return NO;
  
  buf[32] = '\0';
  memcpy(buf, header+0x10, 32);
  AddAttribute(sd, buf, kMDItemTitle);
  memcpy(buf, header+0x30, 32);
  AddAttributeArray(sd, buf, kMDItemAuthors);
  memcpy(buf, header+0x50, 32);
  AddAttribute(sd, buf, kMDItemCopyright);
  
  return YES;
}


- (BOOL)attemptToImportSPC:(const char *)fn
                attributes:(NSMutableDictionary *)sd
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
  AddAttribute(sd, spctag.comment, kMDItemComment);
  AddAttributeArray(sd, spctag.pub, kMDItemPublishers);
  if (spctag.copy) {
    sprintf(buf, "%d", spctag.copy);
    AddAttribute(sd, buf, kMDItemCopyright);
  }
  songlen = spctag.GetSong();
  if (songlen != spctag.defSong)
    AddAttributeNumber(sd, songlen/64000.0, kMDItemDurationSeconds);
  AddAttribute(sd, spctag.dumper, kMDItemChiptuneDumper);
  
  return YES;
}


- (BOOL)importPSF:(FILE *)fp attributes:(NSMutableDictionary *)sd
{
  NSString *key;
  NSMutableDictionary *intermdict;
  CMIPSFTagParser *tp;
  BOOL res;
  
  tp = [CMIPSFTagParser tagsWithFile:fp error:&res];
  if (!res) return NO;
  
  intermdict = [tp tagDictionary];
  if (!intermdict) return NO;
  
  for (key in intermdict) {
    CFStringRef outk = NULL;
    id val;
    
    if ([key isEqualToString:@"comment"])
      outk = kMDItemComment;
    else if ([key isEqualToString:@"title"])
      outk = kMDItemTitle;
    else if ([key isEqualToString:@"artist"])
      outk = kMDItemAuthors;
    else if ([key isEqualToString:@"game"])
      outk = kMDItemAlbum;
    else if ([key isEqualToString:@"genre"])
      outk = kMDItemGenre;
    else if ([key isEqualToString:@"copyright"])
      outk = kMDItemCopyright;
    else if ([key isEqualToString:@"length"])
      outk = kMDItemDurationSeconds;
    else if ([key hasSuffix:@"by"] && [key length] == 5)
      outk = kMDItemChiptuneDumper;
    
    if (outk) {
      if (outk == kMDItemAuthors)
        val = [NSArray arrayWithObject:[intermdict valueForKey:key]];
      else if (outk == kMDItemDurationSeconds)
        val = DurationStringToSeconds([intermdict valueForKey:key]);
      else
        val = [intermdict valueForKey:key];
      
      [sd setValue:val forKey:(__bridge NSString*)outk];
    }
  }
  
  return YES;
}


@end
