//
//  CMIPSFTagDictionary.h
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 07/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CMIPSFTagDictionary : NSDictionary
{
  NSDictionary *storage;
}

- (instancetype)initWithPSFFilePointer:(FILE *)fp;

@end

#ifdef __cplusplus
extern "C" {
#endif
  NSNumber *DurationStringToSeconds(NSString *str);
#ifdef __cplusplus
}
#endif
