//
//  MySpotlightImporter.h
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 02/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ChiptuneMDImporter : NSObject

- (BOOL)importFileAtPath:(NSString *)filePath attributes:(NSMutableDictionary *)attributes;

@end
