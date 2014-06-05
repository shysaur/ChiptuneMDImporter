/*
 * rewritten in C for x64 by dan-c 5/6/14
 */

/***************************************************************************************************
* String Functions                                                                                 *
*                                                           Copyright (C)2003 Alpha-II Productions *
***************************************************************************************************/

#include	"Types.h"
#include "strings.h"


//**************************************************************************************************
// Reverse Scan String for Character

s8* ScanStrR(const s8 *pStr, s8 c)
{
  return strrchr(pStr, c);
}


//**************************************************************************************************
// Compare String with Length
// return 1 if len bytes match in pDest and in pSrc. Does not check for null.

b8 CmpStrL(const s8 *pDest, const s8 *pSrc, u32 len)
{
  return memcmp(pDest, pSrc, len) ? 0 : 1;
}


//**************************************************************************************************
// Compare and Copy String with Length

b8 CmpCopyL(s8 *pDest, const s8 *pSrc, u32 len)
{
  if (strncmp(pDest, pSrc, len) == 0)
    return 1;
  else
    strncpy(pDest, pSrc, len);
  return 0;
}


//**************************************************************************************************
// Copy String

s8* CopyStr(s8 *pDest, const s8 *pSrc)
{
  return stpcpy(pDest, pSrc);
}


//**************************************************************************************************
// Copy String with Length

s8* CopyStrL(s8 *pDest, const s8 *pSrc, s32 l)
{
  return stpncpy(pDest, pSrc, l);
}


//**************************************************************************************************
// Copy String Upto Character

s8* CopyStrC(s8 *pDest, const s8 *pSrc, s8 c)
{
  while (*pSrc != c && *pSrc != '\0')
    *(pDest++) = *(pSrc++);
  return pDest-1;
}


//**************************************************************************************************
// Find String End

s8* StrEnd(const s8 *pStr)
{
  return (s8*)pStr + strlen(pStr);
}


//**************************************************************************************************
// Convert Integer to Hex

s8*	Int2Hex(u32 i, u32 d)
{
	static s8	hex[16] = {"0123456789ABCDEF"};
	static s8	str[9];

	str[d] = 0;
	while(d--)
	{
		str[d] = hex[i & 0xF];
		i >>= 4;
	}

	return str;
}
      
