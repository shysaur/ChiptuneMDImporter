/***************************************************************************************************
* File:       Math Functions                                                                       *
* Platform:   x86                                                                                  *
* Programmer: Anti Resonance                                                                       *
*                                                                                                  *
* This program is free software; you can redistribute it and/or modify it under the terms of the   *
* GNU General Public License as published by the Free Software Foundation; either version 2 of     *
* the License, or (at your option) any later version.                                              *
*                                                                                                  *
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;        *
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.        *
* See the GNU General Public License for more details.                                             *
*                                                                                                  *
* You should have received a copy of the GNU General Public License along with this program;       *
* if not, write to the Free Software Foundation, Inc.                                              *
* 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.                                        *
*                                                                                                  *
*                                                           Copyright (C)2003 Alpha-II Productions *
***************************************************************************************************/

#ifdef	__cplusplus
extern "C"
{
#endif

//**************************************************************************************************
// Bit Scan Forward
//
// Scans an integer right to left, starting with bit 0, for the first bit that's set
//
// In:
//    i = Integer
//
// Out:
//    bit position (0-31, -1 if no bit found)

s32 BitScanF(u32 i);


//**************************************************************************************************
// Bit Scan Reversed
//
// Scans an integer left to right, starting with bit 31, for the first bit that's set
//
// In:
//    i = Integer
//
// Out:
//    bit position (0-31, -1 if no bit found)

s32 BitScanR(u32 i);


//**************************************************************************************************
// Bit Count

s32 BitCount(u32 i);


//**************************************************************************************************
// Round Float to Integer
//
// Uses the FPU to round a float to an integer quantity.  Provides better results than the compiler
// for some reason.
//
// In:
//    f = Value to convert to int
//
// Out:
//    Integer quantity

s32 F2I(f64 f);


//**************************************************************************************************
// Logarithm
//
// Desc:
//    Calculates the arbitrary log of any value
//
// In:
//    base  = Base of log to caluclate
//    value = Value to derrive exponent from
//
// Out:
//    Exponent

f80 Logx(f64 base, f64 value);


//**************************************************************************************************
// Scaled Log 2 of x
//
// Computes Y*log2(x)
//
// In:
//    scale = Amount to multiply intermediate result by
//    x     = Value to derrive exponent from
//
// Out:
//    Exponent

f64 YLog2(f64 scale, f64 x);


//**************************************************************************************************
// Power of 2
//
// Computes 2^x  (This function was created to elimnate the need to link in the math libraries)
//
// In:
//    exp  = Exponent to raise 2 to (must be between -1.0 and +1.0)
//
// Out:
//    2^exp

f64 Pow2(f64 exp);

#ifdef	__cplusplus
}
#endif
