//
//  CMIPSFTagParser.h
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 07/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CMIPSFTagParser : NSObject
{
  NSStringEncoding enc;
  char *tagend;
  char *tagdict;
}

+ alloc;
+ tagsWithFile:(FILE *)fp error:(BOOL*)err;
- init;
- initWithFile:(FILE *)fp error:(BOOL*)err;
- (void)dealloc;
- (BOOL)setTagsFromFile:(FILE *)fp;
- (NSMutableDictionary*)tagDictionary;

@end

#ifdef __cplusplus
extern "C" {
#endif
  NSNumber *DurationStringToSeconds(NSString *str);
#ifdef __cplusplus
}
#endif
