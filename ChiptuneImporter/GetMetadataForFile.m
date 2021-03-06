//
//  GetMetadataForFile.m
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 02/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#import <CoreData/CoreData.h>
#import "ChiptuneMDImporter.h"

Boolean GetMetadataForFile(void *thisInterface, CFMutableDictionaryRef attributes, CFStringRef contentTypeUTI, CFStringRef pathToFile);

//==============================================================================
//
//	Get metadata attributes from document files
//
//	The purpose of this function is to extract useful information from the
//	file formats for your document, and set the values into the attribute
//  dictionary for Spotlight to include.
//
//==============================================================================

Boolean GetMetadataForFile(void *thisInterface, CFMutableDictionaryRef attributes, CFStringRef contentTypeUTI, CFStringRef pathToFile)
{
  Boolean ok = FALSE;
  
  @autoreleasepool {
    ok = CMIImportFile((__bridge NSString *)pathToFile, (__bridge NSMutableDictionary *)attributes);
  }
  
	// Return the status
  return ok;
}
