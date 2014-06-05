/***************************************************************************************************
* Program:    Super Nintendo Entertainment System(tm) APU Emulator v3.1 for Winamp                 *
* Platform:   Win32                                                                                *
* Programmer: Anti Resonance                                                                       *
*                                                                                                  *
* "SNES" and "Super Nintendo Entertainment System" are trademarks of Nintendo Co., Limited and its *
* subsidiary companies.                                                                            *
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
* ------------------------------------------------------------------------------------------------ *
* Revision History:                                                                                *
*                                                                                                  *
* 3.1  26.1.03                                                                                     *
*      + Fields in the ID666 dialog are now readable when viewing an .spc from an .rsn archive     *
*      + Added customizable file extensions                                                        *
*      + Updated config dialog to work with new DSP options                                        *
*      - Fixed window snapping and placement issues when using the Modern skin in Winamp 5         *
*                                                                                                  *
* 3.05 29.7.03                                                                                     *
*      - Autoend feature wasn't working with new SNESAPU                                           *
*                                                                                                  *
* 3.04 18.6.03                                                                                     *
*      + Added volume reset to ChangeOutput() to work around the same balance problem              *
*      - Fixed SaveConfig() not writing out the correct value for interpolation                    *
*                                                                                                  *
* 3.03a 14.6.03                                                                                    *
*      + Reset the output plug-in volume when playing a song.  Prevents the balance control from   *
*        upsetting the soundcard's volume when using waveOut 2.0.2a                                *
*                                                                                                  *
* 3.03 22.5.03                                                                                     *
*      + Added conditional statements to the title format                                          *
*      - Fixed a bug that was preventing some RSN's from being loaded                              *
*                                                                                                  *
* 3.02 19.5.03                                                                                     *
*      + Shortened filter list to one filter                                                       *
*      - Corrected search path for unrar.dll                                                       *
*      - Fixed file extraction for files that come out of RSN's in two pieces                      *
*      - Extract SPC files while scanning RSN's so Winamp can retrieve the title for people who    *
*        have it configured to "Read title on load"                                                *
*                                                                                                  *
* 3.01 1.5.03                                                                                      *
*      - Preset loader had presets swapped                                                         *
*      - Configuration was initializing amp slider incorrectly                                     *
*      - Amp was getting reset under the wrong conditions                                          *
*                                                                                                  *
* 3.0  30.4.03                                                                                     *
*      + Added RAR support and everything that goes along with it                                  *
*      + Changed file filters (Winamp will now register .SP*, .ZS*, and .RSN instead of SP1-SP9.   *
*        This allows more filters to be used in the open dialog.)                                  *
*      + Added additional file detection (Necessary for wild cards to be used in extensions)       *
*      + Added header caching (Reduces RAR access since files were getting read nearly ten times   *
*        during the changing/loading phase)                                                        *
*      + Changed amplification scale to decibels                                                   *
*      + Removed the pitch option from the config (was anybody even using it?)                     *
*      + Added "Enable tricks" to the config incase SNESAmp isn't being used by Winamp 2.x         *
*      - Stopped subclassing main process and streamlined hooking (fixes crash-on-exit)            *
*      - Fixed dialog creep on desktops with the taskbar locked at the top                         *
*      - New dialogs appear at the same location as the Winamp main window                         *
*      - Fixed AAR being improperly displayed in configuration                                     *
*      - Improved alpha layering detection                                                         *
*                                                                                                  *
* 2.99 10.2.03                                                                                     *
*      + Improved snapping algorithm (dialogs can snap to any window in the Winamp process)        *
*      + Improved docking algorithm (dialogs can be docked to any window connected to the main)    *
*      + Control dialog doesn't automatically gain the focus when it becomes visible               *
*      + Control dialog sort of works with transparency plug-ins                                   *
*      + Tag editor and Control remember their absloute screen position                            *
*      + Improved binary tag detection                                                             *
*      + Added ability to save mixing level in the tag                                             *
*      + Added support for a negative end time in the tag                                          *
*      + Added disable echo to the configuration                                                   *
*      + Changed APR threshold scale to -/+ 3.00dB                                                 *
*      + Moved reusable functions into base files (now they can be used by other programs)         *
*      + Made the ID666 class more robust, taking some of the confusion out of Main and Tag        *
*      + Rewrote a lot of code all over (moved all subclassing code into Main, and all emulation   *
*        code into Winamp)                                                                         *
*      - Added version information to LRM's and improved LRM detection and loading (LRM's from     *
*        previous versions of SNESAmp won't get loaded and crash)                                  *
*      - Fixed another return value error in the message handler (titlebar text scrolls)           *
*      - Sometimes fade wouldn't work after seeking                                                *
*      - 16-bit samples are sent to visualization when 32-bit sample mode is set                   *
*      - Sometimes the mixing level in the config was displayed wrong                              *
*      - Forgot to save the date in the tag                                                        *
*      - Forgot to apply the mute flags from the tag                                               *
*      - Fixed the time calculation so the buttons in the tag editor get the correct time when a   *
*        song's speed has been adjusted via the control dialog                                     *
*      - Fixed a bug in the ID666 class that was causing the tag editor to crash on negative years *
*        in the dumped on field                                                                    *
*                                                                                                  *
* 2.5  2.10.01                                                                                     *
*      + Added Gaussian interpolation to the configuration                                         *
*      + Added support for the extended time fields to the tag editor                              *
*      + Switched from run-time to dynamic linking of SNESAPU                                      *
*      + Playback switches to fadeout if timer is enabled after song would've ended                *
*      + Made a lot of general improvements to the codebase                                        *
*      - Rewrote the tag editor and some other code after a hard drive mishap                      *
*      - SNESAPU was accidentally getting loaded twice                                             *
*      - Search for SNESAPU.DLL in Winamp\Plugins before throwing an error                         *
*      - Configuration loader was returning unknown values in the Inter field as value one higher  *
*        than the highest possible                                                                 *
*      - Improved emulation thread (should be less crashes)                                        *
*      - Fixed emulation thread so Winamp DSP plug-ins and equalizer work                          *
*      - Fixed something that was causing Winamp to spawn multiple instances, even if that feature *
*        was disabled in the Winamp config                                                         *
*      - Control dialog would disappear if Winamp was left of the primary monitor                  *
*      - Corrected noise clock values in tag editor                                                *
*                                                                                                  *
* 2.1  14.2.01                                                                                     *
*      + Added "What's This?" popup menu to all controls                                           *
*      - Added the ability configure the amount of silence before ending a song                    *
*      - Auto end function only works if song doesn't have an ID666 tag                            *
*      - Fixed the tab key and keyboard shortcuts in the Configuration and ID666 editor dialogs    *
*      - Stereo controls are disabled when monaural is selected                                    *
*      - Threshold setting wasn't getting saved                                                    *
*      - Apply button wasn't working right in Configuration dialog                                 *
*      - Cancel wasn't undoing changes in ID666 editor                                             *
*      - Coded a workaround for a nasty bug in Winamp that was calling Init more than once (this   *
*        should stop the playback thread from hanging and crashing the system)                     *
*                                                                                                  *
* 2.0  26.1.01 DJ Alpha Remix                                                                      *
*      + Switched to BCB 5                                                                         *
*      + Separated time button into song and fade                                                  *
*      + Changes to configuration have an immediate effect                                         *
*      + Incorporated translations                                                                 *
*      + Control dialog disappears when a non-SPC is being played                                  *
*      - Cleaned up code                                                                           *
*                                                                                                  *
* 1.2  22.5.00 Import Release with Bonus Tracks                                                    *
*      + Switched to C++ (code is mostly non-OO, though)                                           *
*      + Added auto preamplifcation reduction (APR)                                                *
*      + Added context sensitive help to dialog boxes                                              *
*      + ZST's are loadable                                                                        *
*      + Removed the ability to save tags in binary format                                         *
*      + ID666 editor has an Apply button                                                          *
*      + Added out port status to control window                                                   *
*      - Files are checked for validity                                                            *
*      - Song length must be at least 1 second                                                     *
*      - Removed an invalid line of code that was causing crashes in the ID666 editor              *
*      - ID666 editor terminates when a file handle is invalid                                     *
*      - Fade and song length are based off NumSmp instead of 576                                  *
*                                                                                                  *
* 1.0  15.4.00 Final Release                                                                       *
*      + Configuration changes are applied to the next song                                        *
*      + Changes to the time in the ID666 tag have an immediate effect                             *
*      + Command sub-window can be disabled                                                        *
*                                                                                                  *
* 0.91 1.4.00 Bugfix                                                                               *
*      - Configuration gets saved                                                                  *
*      - Coded a workaround for SD3 .SPC's that were playing too slow                              *
*                                                                                                  *
* 0.9  25.03.00 Initial test release                                                               *
*                                                      Copyright (C)2000-2003 Alpha-II Productions *
***************************************************************************************************/


//**************************************************************************************************
// Public Variables

extern	SAPUFunc	sapu;
extern	Settings	cfg;
extern	u32			emuVer;
extern	TLanguages	*pLanguages;
extern	s8			dllPath[MAX_PATH];
extern	s8			hlpFile[MAX_PATH];


//**************************************************************************************************
// Load Language Resource Module
//
// Loads the resource module of a given language ID, and reinitializes all forms
//
// In:
//    lcid = Locale ID to load (0, if user default langugage should be used)
//
// Out:
//    true, if resource module found and loaded

b8 LoadLRM(u32 lcid);


//**************************************************************************************************
// Load Configuration
//
// Retrieves the user defined configuration settings
//
// In:
//    cfg = Structure to store user defined settings
//          (cfg.INIFile must contain the path of the .ini file)

v0 LoadConfig(Settings &cfg);


//**************************************************************************************************
// Save Configuration
//
// Stores the user defined configuration settings
//
// In:
//    cfg = Structure of user defined settings to save
//          (cfg.INIFile must contain the path of the .ini file)

v0 SaveConfig(Settings &newCfg);


//**************************************************************************************************
// Reset Configuration
//
// Erases all saved settings and loads defaults

v0 ResetConfig();


//**************************************************************************************************
// Get Main Window Handle
//
// Returns the handle to the main window.  Necessary because Winamp 5 has two different main
// windows, and the active one isn't necessarily the one in inMod.hMainWindow.

HWND __fastcall GetMainWnd();


//**************************************************************************************************
// Get Relative Position to Main Window
//
// If a form is docked directly to the main window, the relative position is returned.  Otherwise
// the absolute screen position is returned.  If the form is docked, the MSB will be set.
//
// In:
//    top, left = Form position (stored in the lower 16-bits)
//
// Out:
//    true, if the form was docked

b8 __fastcall GetRelPos(TForm *pFrm, s32& top, s32& left);


//**************************************************************************************************
// Set Position Relative to Main Window
//
// The lower 16 bits of the parameters contain screen position.
// If the upper 16 bits are:
//    0      the window will be positioned at that location
//    0x8000 the window will be positioned relative to the main window
//    > 0    the window will get the same position as the main window
//    < 0    the window will be adjusted to fit next to the main window
//
// In:
//    pFrm     -> Form to position
//    top, left = Position on screen

void __fastcall SetRelPos(TForm *pFrm, s32 top, s32 left);


