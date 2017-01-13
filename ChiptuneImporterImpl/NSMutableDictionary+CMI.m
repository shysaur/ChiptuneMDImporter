//
//  NSMutableDictionary+CMI.m
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 18/10/15.
//  Copyright © 2015 Daniele Cattaneo. All rights reserved.
//

#import "NSMutableDictionary+CMI.h"


@implementation NSMutableDictionary (CMI)


- (void)CMI_setCStringAttribute:(const char*)s forKey:(CFStringRef)k
{
  NSString *t;
  
  if (*s != '\0') {
    t = [NSString stringWithCString:s encoding:NSWindowsCP1252StringEncoding];
    [self CMI_setAttribute:t forKey:k];
  }
}


- (void)CMI_setArrayAttributeWithCString:(const char*)s forKey:(CFStringRef)k
{
  NSString *t;
  
  if (*s != '\0') {
    t = [NSString stringWithCString:s encoding:NSWindowsCP1252StringEncoding];
    [self CMI_setAttribute:@[t] forKey:k];
  }
}


- (void)CMI_setAttribute:(id)a forKey:(CFStringRef)k
{
  [self setObject:a forKey:(__bridge NSString*)k];
}


@end
