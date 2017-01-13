//**************************************************************************************************
// Type Redefinitions

// fixed for x64 by dan-c 4/6/14

#include <stdint.h>

typedef void v0;

#ifdef	__cplusplus
#if defined __BORLANDC__
typedef bool b8;
#else
typedef unsigned char b8;
#endif
#else
typedef	char b8;
#endif

typedef unsigned char u8;
typedef uint16_t u16;
typedef uint32_t u32;
#if defined _MSC_VER || defined __BORLANDC__
typedef unsigned __int64 u64;
#else
typedef	uint64_t u64;
#endif

typedef char s8;
typedef int16_t s16;
typedef int32_t s32;
#if defined _MSC_VER || defined __BORLANDC__
typedef __int64 s64;
#else
typedef	int64_t s64;
#endif

typedef float f32;
typedef double f64;
typedef long double f80;
