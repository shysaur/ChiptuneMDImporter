/***************************************************************************************************
* Header: SNESAmp Globals                                                                          *
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
*                                                      Copyright (C)2000-2003 Alpha-II Productions *
***************************************************************************************************/

#include	<stdio.h>
#include	"Types.h"							//Type redefinitions
#include	"Strings.h"							//Dynamic string indices
#include	"HelpID.h"
#include	"A2Dock.h"							//Alpha-II Docking functions
#include	"A2Date.h"							//Alpha-II Date class
#include	"A2Math.h"							//Alpha-II Math functions
#include	"A2Str.h"							//Alpha-II String functions
#include	"ID666.h"							//Extended ID666 tag class
#include	"SNESAPU.h"							//SNESAPU emulator
#include	"unrar.h"							//Unrar
												//(additional includes are at the bottom)

//**************************************************************************************************
// Defines and Macros

#define	SAPU_OLDEST	0x11000						//Oldest version of SNESAPU we can use
#define	SAPU_NEWEST	0x20000						//Newest version we're compatible with

#define	CUR_VER		310							//Current version of SNESAmp configuration
#define	LRM_OLD		((3 << 16) | 03)			//Oldest LRM version
#define	LRM_NEW		((3 << 16) | 10)			//Newest LRM version

#define	AAR_TYPE	0x03						//Mask for type of attenuation
#define	AAR_ON		0x10						//Attenuation is enabled for this song
#define	AAR_MIN		0x20						//Level has reached minimum value
#define	AAR_MAX		0x40						//Level has reached maximum value, or output clipped

#define	_Clamp(n, min, max) \
		if ((n) < (min)) n = min; \
		else \
		if ((n) > (max)) n = max;


//**************************************************************************************************
// Structures

//SNESAmp configuration settings
typedef struct
{
	u32	rate;								  	//Output sample rate
	u32	bits;								  	//Sample size
	s32	chn;								  	//Number of channels
	u8	mix;									//Mix type
	u8	inter;								  	//Interpolation type
	b8	lowPass;							  	//Use low-pass filter
	b8	surround;							  	//Enable fake surround sound
	b8	reverse;							  	//Reverse stereo
	b8	oldADPCM;							  	//Used old decompression routine
	b8	noEcho;									//Disable echo
	u32	stereo;									//Stereo separation
	s32	echo;									//Echo feedback crosstalk
} CfgDSP;

typedef struct
{
	u8	aar;								  	//see AAR_?? defines
	b8	tagAmp;									//Use amplification level from xid6 tag
	b8	reset;									//Reset amp level at the beginning of each song
	u32	amp;								  	//Initial amplification level
	u32	minAmp,maxAmp;							//Min and max amp level
	u32	threshold;							  	//Maximum amplitude allowed
} CfgMix;


typedef struct
{
	s8		preset;								//Selected preset

	CfgDSP	udsp;								//User defined DSP and mixing settings
	CfgMix	umix;

	CfgDSP	dsp;								//Current DSP and mixing settings
	CfgMix	mix;

	struct
	{
		u32	song,fade;						  	//Default song and fade length
		u32	silence;						  	//Trailing silence
		u32	autoEnd;						  	//Amount of silence before ending song
		u32	loopx;								//Number of times to play song loop
		b8	useTimer;						  	//Use ID666 song length
		b8	fastSeek;						  	//Use faster seeking method
		b8	defBin;							  	//Default to binary tags
	} time;

	u8	waVol;								  	//Use Winamp's volume control for preamp
	b8	ctrl;								  	//Display control window
	b8	tricks;									//Enable tricks

	s8	titleFmt[64];							//Title formatting string

	u32	language;								//Language module

	s32	tagTop,tagLeft;
	s32	ctrlTop,ctrlLeft;						//Control dialog's position relative to Winamp
	s32	pitch;									//Pitch
	s32	speed;									//Speed

	s8	fileExt[64];							//Associated file extensions

	s8	iniFile[MAX_PATH];						//Path to plugin.ini

} Settings;

typedef struct
{
	HANDLE	__stdcall (*OpenArchive)(struct RAROpenArchiveData *ArchiveData);
	s32		__stdcall (*CloseArchive)(HANDLE hArcData);
	s32		__stdcall (*ReadHeader)(HANDLE hArcData, struct RARHeaderData *HeaderData);
	s32		__stdcall (*ProcessFile)(HANDLE hArcData, s32 Operation, s8 *DestPath, s8 *DestName);
	void	__stdcall (*SetCallback)(HANDLE hArcData, UNRARCALLBACK Callback, LONG UserData);
	s32		__stdcall (*GetDllVersion)();
} UnrarFunc;

#include	"Main.h"							//Main program
#include	"Winamp.h"							//Winamp exported functions
