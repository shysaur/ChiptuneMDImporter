//
//  MySpotlightImporter.h
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 02/06/14.
//  Copyright (c) 2014 Daniele Cattaneo. All rights reserved.
//

#import <Cocoa/Cocoa.h>


#ifdef __cplusplus
extern "C" {
#endif
  
BOOL importFile(NSString *filePath, NSMutableDictionary *attributes);

#ifdef __cplusplus
}
#endif
