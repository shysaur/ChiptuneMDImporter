//
//  NSMutableDictionary+CMI.h
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 18/10/15.
//  Copyright © 2015 Daniele Cattaneo. All rights reserved.
//

#import <Foundation/Foundation.h>


@interface NSMutableDictionary (CMI)

- (void)CMI_setCStringAttribute:(const char*)s forKey:(CFStringRef)k;
- (void)CMI_setArrayAttributeWithCString:(const char*)s forKey:(CFStringRef)k;
- (void)CMI_setAttribute:(id)a forKey:(CFStringRef)k;

@end
