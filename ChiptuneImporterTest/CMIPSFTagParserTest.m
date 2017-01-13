//
//  CMIPSFTagParserTest.m
//  ChiptuneImporter
//
//  Created by Daniele Cattaneo on 13/01/17.
//  Copyright © 2017 Daniele Cattaneo. All rights reserved.
//

#import <XCTest/XCTest.h>
#import "CMIPSFTagParser.h"


@interface CMIPSFTagParserTest : XCTestCase

@end


@implementation CMIPSFTagParserTest


- (void)executeTestCaseData:(NSDictionary<NSString*, NSDictionary*> *)cases
{
  [cases enumerateKeysAndObjectsUsingBlock:
  ^(NSString * _Nonnull tags, NSDictionary * _Nonnull exp, BOOL * _Nonnull stop) {
    NSData *d = [tags dataUsingEncoding:NSUTF8StringEncoding];
    NSDictionary *res = PSFTagsDictionaryFromTagData(d);
    res = [NSDictionary dictionaryWithDictionary:res];
    XCTAssert([res isEqual:exp], "%@ =/=> %@ !", tags, exp);
  }];
}


- (void)testEmptyTags
{
  [self executeTestCaseData:@{
    @"": @{}
  }];
}


- (void)testOneTag
{
  [self executeTestCaseData:@{
    @"tagname=hello\n": @{@"tagname": @"hello"}
  }];
}


- (void)testOneUTF8Tag
{
  [self executeTestCaseData:@{
    @"utf8=1\ntagname=\U0001F530!\n": @{@"utf8": @"1", @"tagname": @"\U0001F530!"}
  }];
}


- (void)testIgnoreInvalidLines
{
  [self executeTestCaseData:@{
    @"\n\n\n": @{},
    @"mooo\n\n\n": @{},
    @"foo": @{},
    @"bar\n": @{},
    @"\nbar": @{},
    @"\nbar\n": @{},
    @"\n\nbar\n\n": @{},
    @"tag=1\nfoo\nother=2\n": @{@"tag": @"1", @"other": @"2"}
  }];
}


- (void)testMultilineVariables
{
  [self executeTestCaseData:@{
    @"var=hello\nvar=world\n": @{@"var": @"hello\nworld"},
    @"m=a\nvar=hello\nvar=world\n": @{@"m": @"a", @"var": @"hello\nworld"},
    @"var=hello\nvar=world\nm=a\n": @{@"m": @"a", @"var": @"hello\nworld"},
  }];
}


- (void)testWhitespaceIgnore
{
  NSString *test;
  NSString *sa, *sb, *sc, *sd;
  NSArray<NSString *> *strings = @[@"", @" ", @"  "];
  
  for (int a=0; a<3; a++) {
    sa = strings[a];
    for (int b=0; b<3; b++) {
      sb = strings[b];
      for (int c=0; c<3; c++) {
        sc = strings[c];
        for (int d=0; d<3; d++) {
          sd = strings[d];
          test = [NSString stringWithFormat:@"%@a%@=%@b%@\n", sa, sb, sc, sd];
          [self executeTestCaseData:@{test: @{@"a": @"b"}}];
        }
      }
    }
  }
}


- (void)testCrLf
{
  [self executeTestCaseData:@{
    @"var=hello\r\nvar=world\r\n": @{@"var": @"hello\nworld"},
  }];
}


- (void)testMultipleUTF8Tags
{
  const char dump[] = {
0x75,0x74,0x66,0x38,0x3D,0x31,0x0A,0x74,0x69,0x74,0x6C,0x65,0x3D,0xE6,0x84,0x9F,
0xE8,0xAC,0x9D,0xE7,0xA5,0xAD,0xE3,0x81,0xAE,0xE8,0xB8,0x8A,0xE3,0x82,0x8A,0x0A,
0x67,0x61,0x6D,0x65,0x3D,0x53,0x75,0x6D,0x6D,0x6F,0x6E,0x20,0x4E,0x69,0x67,0x68,
0x74,0x20,0x32,0x0A,0x63,0x6F,0x70,0x79,0x72,0x69,0x67,0x68,0x74,0x3D,0x46,0x6C,
0x69,0x67,0x68,0x74,0x2D,0x50,0x6C,0x61,0x6E,0x2C,0x20,0x42,0x61,0x6E,0x70,0x72,
0x65,0x73,0x74,0x6F,0x0A,0x79,0x65,0x61,0x72,0x3D,0x32,0x30,0x30,0x31,0x2D,0x30,
0x38,0x2D,0x30,0x32,0x0A,0x61,0x72,0x74,0x69,0x73,0x74,0x3D,0xE8,0x97,0xA4,0xE7,
0x94,0xB0,0xE5,0x8D,0x83,0xE7,0xAB,0xA0,0x0A,0x67,0x65,0x6E,0x72,0x65,0x3D,0x53,
0x69,0x6D,0x75,0x6C,0x61,0x74,0x69,0x6F,0x6E,0x20,0x52,0x50,0x47,0x0A,0x76,0x6F,
0x6C,0x75,0x6D,0x65,0x3D,0x31,0x2E,0x30,0x36,0x34,0x31,0x0A,0x63,0x6F,0x6D,0x6D,
0x65,0x6E,0x74,0x3D,0x43,0x61,0x69,0x74,0x53,0x69,0x74,0x68,0x32,0x2F,0x4D,0x61,
0x72,0x6B,0x20,0x47,0x72,0x61,0x73,0x73,0x27,0x20,0x47,0x65,0x6E,0x65,0x72,0x69,
0x63,0x20,0x50,0x53,0x46,0x20,0x44,0x72,0x69,0x76,0x65,0x72,0x20,0x42,0x75,0x69,
0x6C,0x64,0x20,0x31,0x2F,0x31,0x39,0x2F,0x30,0x37,0x0A,0x6C,0x65,0x6E,0x67,0x74,
0x68,0x3D,0x33,0x3A,0x31,0x36,0x0A,0x66,0x61,0x64,0x65,0x3D,0x31,0x30,0x0A,0x72,
0x65,0x70,0x6C,0x61,0x79,0x67,0x61,0x69,0x6E,0x5F,0x74,0x72,0x61,0x63,0x6B,0x5F,
0x67,0x61,0x69,0x6E,0x3D,0x2D,0x30,0x2E,0x31,0x32,0x20,0x64,0x42,0x0A,0x72,0x65,
0x70,0x6C,0x61,0x79,0x67,0x61,0x69,0x6E,0x5F,0x74,0x72,0x61,0x63,0x6B,0x5F,0x70,
0x65,0x61,0x6B,0x3D,0x30,0x2E,0x37,0x34,0x34,0x0A,0x72,0x65,0x70,0x6C,0x61,0x79,
0x67,0x61,0x69,0x6E,0x5F,0x61,0x6C,0x62,0x75,0x6D,0x5F,0x67,0x61,0x69,0x6E,0x3D,
0x2B,0x30,0x2E,0x35,0x34,0x20,0x64,0x42,0x0A,0x72,0x65,0x70,0x6C,0x61,0x79,0x67,
0x61,0x69,0x6E,0x5F,0x61,0x6C,0x62,0x75,0x6D,0x5F,0x70,0x65,0x61,0x6B,0x3D,0x31,
0x2E,0x30,0x35,0x33,0x34,0x0A,0x00
  };
  NSString *str = [NSString stringWithUTF8String:dump];
  [self executeTestCaseData:@{
    str: @{
      @"utf8": @"1",
      @"title": @"\u611F\u8B1D\u796D\u306E\u8E0A\u308A",
      @"game": @"Summon Night 2",
      @"copyright": @"Flight-Plan, Banpresto",
      @"year": @"2001-08-02",
      @"artist": @"\u85E4\u7530\u5343\u7AE0",
      @"genre": @"Simulation RPG",
      @"volume": @"1.0641",
      @"comment": @"CaitSith2/Mark Grass' Generic PSF Driver Build 1/19/07",
      @"length": @"3:16",
      @"fade": @"10",
      @"replaygain_track_gain": @"-0.12 dB",
      @"replaygain_track_peak": @"0.744",
      @"replaygain_album_gain": @"+0.54 dB",
      @"replaygain_album_peak": @"1.0534"
    },
  }];
}


@end
