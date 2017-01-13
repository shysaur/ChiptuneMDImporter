//
//  MySpotlightImporter.m
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 02/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#import "ChiptuneMDImporter.h"
#import "CMIPSFTagParser.h"
#import "NSMutableDictionary+CMI.h"
#import "ID666.h"


const CFStringRef kMDItemChiptuneOST =
        (CFStringRef)@"com_danielecattaneo_chiptunemdimporter_ost";
const CFStringRef kMDItemChiptuneDumper =
        (CFStringRef)@"com_danielecattaneo_chiptunemdimporter_dumper";
const CFStringRef kMDItemChiptuneExpansion =
        (CFStringRef)@"com_danielecattaneo_chiptunemdimporter_expansion";
const CFStringRef kMDItemChiptuneSIDRevision =
        (CFStringRef)@"com_danielecattaneo_chiptunemdimporter_sidrevision";

typedef enum {
  kNSF,
  kGBS,
  kPSF,
  kSID,
  kUnsupported,
  kUnreadable
} CMIFileType;


BOOL CMIImportNSF(FILE *fp, NSMutableDictionary *sd)
{
  char header[0x80];
  char buf[32+1];
  int i, expm;
  NSMutableString *expstr;
  NSArray * const exp =
    @[@"VRC6", @"VRC7", @"FDS", @"MMC5", @"Namco 106", @"Sunsoft FME-07"];
  
  if (fseek(fp, 0, SEEK_SET) < 0) return NO;
  if (fread(header, sizeof(char), 0x80, fp) < 0x80) return NO;
  
  buf[32] = '\0';
  memcpy(buf, header+0xE, 32);
  [sd CMI_setCStringAttribute:buf forKey:kMDItemTitle];
  memcpy(buf, header+0x2E, 32);
  [sd CMI_setArrayAttributeWithCString:buf forKey:kMDItemAuthors];
  memcpy(buf, header+0x4E, 32);
  [sd CMI_setCStringAttribute:buf forKey:kMDItemCopyright];
  
  expstr = [NSMutableString string];
  if (header[0x7B]) {
    expm = 1;
    for (i=0; i<6; i++) {
      if (expm & header[0x7B]) {
        if ((expm - 1) & header[0x7B])
          [expstr appendString:@", "];
        [expstr appendString:[exp objectAtIndex:i]];
      }
      expm <<= 1;
    }
    [sd CMI_setAttribute:expstr forKey:kMDItemChiptuneExpansion];
  }
  
  return YES;
}


BOOL CMIImportGBS(FILE *fp, NSMutableDictionary *sd)
{
  char header[0x70];
  char buf[32+1];
  
  if (fseek(fp, 0x00, SEEK_SET) < 0) return NO;
  if (fread(header, sizeof(char), 0x70, fp) < 0x70) return NO;
  
  buf[32] = '\0';
  memcpy(buf, header+0x10, 32);
  [sd CMI_setCStringAttribute:buf forKey:kMDItemTitle];
  memcpy(buf, header+0x30, 32);
  [sd CMI_setArrayAttributeWithCString:buf forKey:kMDItemAuthors];
  memcpy(buf, header+0x50, 32);
  [sd CMI_setCStringAttribute:buf forKey:kMDItemCopyright];
  
  return YES;
}


BOOL CMIImportSID(FILE *fp, NSMutableDictionary *sd)
{
  NSString *sidrev;
  char header[0x7C];
  char buf[32+1];
  
  if (fseek(fp, 0x00, SEEK_SET) < 0) return NO;
  if (fread(header, sizeof(char), 0x7C, fp) < 0x7C) return NO;
  
  buf[32] = '\0';
  memcpy(buf, header+0x16, 32);
  [sd CMI_setCStringAttribute:buf forKey:kMDItemTitle];
  memcpy(buf, header+0x36, 32);
  [sd CMI_setArrayAttributeWithCString:buf forKey:kMDItemAuthors];
  memcpy(buf, header+0x56, 32);
  [sd CMI_setCStringAttribute:buf forKey:kMDItemCopyright];
  
  if ((((unsigned)header[4])*0x100 + header[5]) >= 2) {
    switch ((header[0x77] & 0b110000) >> 4) {
      case 1:
        sidrev = @"6581";
        break;
      case 2:
        sidrev = @"8580";
        break;
      case 3:
        sidrev = @"6581, 8580";
        break;
    }
    if (sidrev)
      [sd CMI_setAttribute:sidrev forKey:kMDItemChiptuneSIDRevision];
  }
  
  return YES;
}


BOOL CMIAttemptToImportSPC(const char *fn, NSMutableDictionary *sd)
{
  NSString *tmp;
  int songlen;
  ID666 spctag;
  ID6Type res;
  
  res = spctag.LoadTag(fn, 0);
  if (res == ID6_UNK || res == ID6_ERR)
    return NO;
  
  [sd CMI_setCStringAttribute:spctag.song forKey:kMDItemTitle];
  [sd CMI_setCStringAttribute:spctag.game forKey:kMDItemAlbum];
  [sd CMI_setCStringAttribute:spctag.ost forKey:kMDItemChiptuneOST];
  [sd CMI_setArrayAttributeWithCString:spctag.artist forKey:kMDItemAuthors];
  [sd CMI_setCStringAttribute:spctag.comment forKey:kMDItemComment];
  [sd CMI_setArrayAttributeWithCString:spctag.pub forKey:kMDItemPublishers];
  if (spctag.copy) {
    tmp = [NSString stringWithFormat:@"%d", spctag.copy];
    [sd CMI_setAttribute:tmp forKey:kMDItemCopyright];
  }
  songlen = spctag.GetSong();
  if (songlen != spctag.defSong)
    [sd CMI_setAttribute:@(songlen/64000.0) forKey:kMDItemDurationSeconds];
  [sd CMI_setCStringAttribute:spctag.dumper forKey:kMDItemChiptuneDumper];
  
  return YES;
}


BOOL CMIImportPSF(FILE *fp, NSMutableDictionary *sd)
{
  NSString *key;
  NSDictionary *tp;
  
  tp = PSFTagsDictionaryFromFile(fp);
  if (!tp) return NO;
  
  for (key in tp) {
    CFStringRef outk = NULL;
    id val;
    
    if ([key isEqual:@"comment"])
      outk = kMDItemComment;
    else if ([key isEqual:@"title"])
      outk = kMDItemTitle;
    else if ([key isEqual:@"artist"])
      outk = kMDItemAuthors;
    else if ([key isEqual:@"game"])
      outk = kMDItemAlbum;
    else if ([key isEqual:@"genre"])
      outk = kMDItemGenre;
    else if ([key isEqual:@"copyright"])
      outk = kMDItemCopyright;
    else if ([key isEqual:@"length"])
      outk = kMDItemDurationSeconds;
    else if ([key hasSuffix:@"by"] && [key length] == 5)
      outk = kMDItemChiptuneDumper;
    
    if (outk) {
      if (outk == kMDItemAuthors)
        val = [NSArray arrayWithObject:[tp objectForKey:key]];
      else if (outk == kMDItemDurationSeconds)
        val = PSFDurationStringToSeconds([tp objectForKey:key]);
      else
        val = [tp objectForKey:key];
      
      [sd CMI_setAttribute:val forKey:outk];
    }
  }
  
  return YES;
}


CMIFileType CMIDetectFileType(FILE *fp) {
  int i;
  uint8_t buf[16];
  CMIFileType ft;
  
  const uint8_t  nsf_magic[] = {'N','E','S','M', 0x1A};
  const uint8_t  gbs_magic[] = {'G','B','S', 0x01};
  const uint8_t  psf_magic[] = {'P','S','F'};
  const uint8_t psid_magic[] = {'P','S','I', 'D'};
  const uint8_t rsid_magic[] = {'R','S','I', 'D'};
  
  const struct {
    CMIFileType ft;
    size_t size;
    const uint8_t *data;
  } magics[] = {
    {kNSF, 5, nsf_magic},
    {kGBS, 4, gbs_magic},
    {kPSF, 3, psf_magic},
    {kSID, 4, psid_magic},
    {kSID, 4, rsid_magic},
    {kUnsupported, 0, NULL}
  };
  
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


BOOL CMIImportFile(NSString *filePath, NSMutableDictionary *spotlightData)
{
  BOOL result;
  CMIFileType ft;
  const char *fn;
  FILE *fp;
  
  fn = [filePath fileSystemRepresentation];
  
  if (CMIAttemptToImportSPC(fn, spotlightData))
    return YES;
  
  if (!(fp = fopen(fn, "rb"))) return NO;
  ft = CMIDetectFileType(fp);
  
  switch (ft) {
    case kNSF:
      result = CMIImportNSF(fp, spotlightData);
      break;
    case kGBS:
      result = CMIImportGBS(fp, spotlightData);
      break;
    case kPSF:
      result = CMIImportPSF(fp, spotlightData);
      break;
    case kSID:
      result = CMIImportSID(fp, spotlightData);
      break;
    default:
      result = NO;
  }
  
  fclose(fp);
  return result;
}



