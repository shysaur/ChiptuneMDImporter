//
//  CMIPSFTagDictionary.h
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 07/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import <Foundation/Foundation.h>


#ifdef __cplusplus
extern "C" {
#endif

NSDictionary *PSFTagsDictionaryFromFile(FILE *fp);
NSDictionary *PSFTagsDictionaryFromTagData(NSData *raw);
NSNumber *PSFDurationStringToSeconds(NSString *str);
  
#ifdef __cplusplus
}
#endif
