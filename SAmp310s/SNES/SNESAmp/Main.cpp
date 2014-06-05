/***************************************************************************************************
* Super Nintendo Entertainment System(tm) Audio Processing Unit Emulator v3.1 for Winamp           *
*                                                                                                  *
*                                                      Copyright (C)2000-2003 Alpha-II Productions *
***************************************************************************************************/

#include	<vcl.h>
#include	<windows.h>
#include	"SNESAmp.h"
#pragma	hdrstop
#include	"About.h"
#include	"Config.h"
#include	"Control.h"
#include	"Tag.h"
#include	"Reinit.hpp"

#pragma	inline


////////////////////////////////////////////////////////////////////////////////////////////////////
// Global Items

//**************************************************************************************************
// Variables

static const s8	itmCtrlTxt[] = "SNESAmp &Control";
static const s8	interList[][10] = {"none","linear","cubic","gaussian"};


SAPUFunc	sapu;								//Function pointers to SNESAPU.DLL
Settings	cfg;				  				//User defined settings
u32			emuVer;								//SNESAPU.DLL version
HINSTANCE 	hSAPU;								//Handle to SNESAPU.DLL
TLanguages	*pLanguages;						//Pointer to object containing system language info
s8			dllPath[MAX_PATH];					//Path of in_snes.dll
s8			hlpFile[MAX_PATH];					//Path of SNESAmp.hlp

UnrarFunc	rar;
HINSTANCE	hUnrar;

//Winamp window interface ----------------------
static HOOKPROC	hpCallWnd,
				hpMsgProc;

static DockTest	dockTest;						//Coordinates of windows created under Winamp
static HWND		hMoving;						//Handle to moving window
static enum
{
	M_NOTMOVING = 0,							//A window is not moving
	M_PREPARE,									//A window may be about to move
	M_PREPARE_TB,								//A window may be about to move via the titlebar
	M_MOVING,									//A window is moving
} wndMoving;									//Status of the main window moving
static b8		tagSticky;
static b8		ctrlSticky;

static u32		itmCtrl;						//ID of menu item


//**************************************************************************************************
// Function Prototypes

extern void	__fastcall SongChange(s8*);

static void	CtrlItem(HMENU hmenu);
static s8*	GetWAFile(HWND);
static void	UnhookWinamp();


////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions

//**************************************************************************************************
// DLL Initialization
//
// Desc:
//    A standard function that must appear in the DLL.  This function is called by Windows when the
//    DLL is loaded to perform any user defined construction and initialization.  SNESAmp performs
//    all necessary processes in Init(), which gets called by Winamp.

s32 WINAPI DllEntryPoint(HINSTANCE hinst, u32 reason, v0* lpReserved)
{
	s8	libFile[MAX_PATH];
	u32	minVer;


	switch (reason)
	{
	case 1:	//DLL_PROCESS_ATTACH
		DisableThreadLibraryCalls(hinst);		//Disables some unnecessary system resources

		emuVer = SAPU_NEWEST;
		minVer = SAPU_OLDEST;

		GetModuleFileName(hinst, dllPath, MAX_PATH);	//Get path to in_snes.dll
		strcpy(libFile, dllPath);
		strcpy(ScanStrR(libFile, '\\'), "SNESAPU.DLL");

		hSAPU = LoadLibrary(libFile);
		if (!hSAPU) hSAPU = LoadLibrary("SNESAPU.DLL");	//Try to load SNESAPU

		//SNESAPU couldn't be loaded -----------
		if (hSAPU == NULL)
		{
			Application->MessageBox("Could not load SNESAPU.DLL.  This file needs to be in your " \
									"plugins directory with in_snes.dll in order for SNESAmp to " \
									"run.  Please reinstall the latest version of SNESAmp.  " \
									"http://www.alpha-ii.com",
									"SNESAmp", MB_OK | MB_ICONERROR);
			return 0;
		}

		(void*)sapu.SNESAPUInfo = (void*)GetProcAddress(hSAPU, "SNESAPUInfo");
		sapu.SNESAPUInfo(&emuVer, &minVer, NULL);

		//Check version requirments ------------
		if (emuVer < SAPU_OLDEST)
		{
/*			Application->MessageBox("Your copy of SNESAPU.DLL is too old to work with this " \
									"version of SNESAmp.  Please get the latest version from " \
									"http://www.alpha-ii.com",
									"SNESAmp", MB_OK | MB_ICONERROR);
*/
			return 0;
		}
		else
		if (minVer > SAPU_NEWEST)
		{
/*			Application->MessageBox("This version of SNESAmp is too old to work with your copy of "\
									"SNESAPU.DLL.  Please get the latest version from "\
									"http://www.alpha-ii.com",
									"SNESAmp", MB_OK | MB_ICONERROR);
*/
			return 0;
		}

		//Get pointers to SNESAPU functions ----
		memset(&sapu, 0, sizeof(sapu));
		(void*)sapu.GetAPUData		= (void*)GetProcAddress(hSAPU, "GetAPUData");
		(void*)sapu.ResetAPU		= (void*)GetProcAddress(hSAPU, "ResetAPU");
		(void*)sapu.FixAPU			= (void*)GetProcAddress(hSAPU, "FixAPU");
		(void*)sapu.LoadSPCFile		= (void*)GetProcAddress(hSAPU, "LoadSPCFile");
		(void*)sapu.SetAPUOpt		= (void*)GetProcAddress(hSAPU, "SetAPUOpt");
		(void*)sapu.SetAPUSmpClk	= (void*)GetProcAddress(hSAPU, "SetAPUSmpClk");
		(void*)sapu.SetAPULength	= (void*)GetProcAddress(hSAPU, "SetAPULength");
		(void*)sapu.EmuAPU			= (void*)GetProcAddress(hSAPU, "EmuAPU");
		(void*)sapu.SeekAPU			= (void*)GetProcAddress(hSAPU, "SeekAPU");
		(void*)sapu.SetSPCDbg		= (void*)GetProcAddress(hSAPU, "SetSPCDbg");
		(void*)sapu.EmuSPC			= (void*)GetProcAddress(hSAPU, "EmuSPC");
		(void*)sapu.GetProcInfo		= (void*)GetProcAddress(hSAPU, "GetProcInfo");
		(void*)sapu.SetDSPPitch		= (void*)GetProcAddress(hSAPU, "SetDSPPitch");
		(void*)sapu.SetDSPAmp		= (void*)GetProcAddress(hSAPU, "SetDSPAmp");
		(void*)sapu.SetDSPStereo	= (void*)GetProcAddress(hSAPU, "SetDSPStereo");
		(void*)sapu.SetDSPEFBCT		= (void*)GetProcAddress(hSAPU, "SetDSPEFBCT");

		sapu.GetAPUData(&sapu.ram, &sapu.xram, NULL, &sapu.t64Cnt,
						&sapu.dsp, &sapu.voice, &sapu.vMMaxL, &sapu.vMMaxR);

		//Get pointers to UNRAR functions ------
		memset(&rar, 0, sizeof(rar));
		strcpy(ScanStrR(libFile, '\\'), "unrar.dll");
		hUnrar = LoadLibrary(libFile);
		if (hUnrar)
		{
			(void*)rar.OpenArchive		= (void*)GetProcAddress(hUnrar, "RAROpenArchive");
			(void*)rar.CloseArchive		= (void*)GetProcAddress(hUnrar, "RARCloseArchive");
			(void*)rar.ReadHeader		= (void*)GetProcAddress(hUnrar, "RARReadHeader");
			(void*)rar.ProcessFile		= (void*)GetProcAddress(hUnrar, "RARProcessFile");
			(void*)rar.SetCallback		= (void*)GetProcAddress(hUnrar, "RARSetCallback");
			(void*)rar.GetDllVersion	= (void*)GetProcAddress(hUnrar, "RARGetDllVersion");
		}

		hpCallWnd = NULL;
		hpMsgProc = NULL;
		wndMoving = M_NOTMOVING;				//Main window is not moving
		tagSticky = 0;
		ctrlSticky = 0;
		itmCtrl = 0;

		dockTest.thread = NULL;
		break;

	case 0:	//DLL_PROCESS_DETACH
		if (hUnrar) FreeLibrary(hUnrar);
		if (hSAPU) FreeLibrary(hSAPU);
	}

	return 1;
}


//**************************************************************************************************
// Winamp Message Hook
//
// Used for detecting song changes, adding SNESAmp to Winamp's system menu, and keeping SNESAmp
// windows docked to other windows
//
// Whenever the user moves a window via the titlebar, a WM_ENTERSIZEMOVE message is sent to notify a
// move is about to take place.  Then WM_WINDOWPOSCHANGED messages are sent as the window moves.
// This makes it easy to keep windows docked together.  However, Winamp windows are skinned, which
// means they have no titlebar, and thus never receive notification that a move is about to take
// place.  See "Mouse Messages" in MessageProc(). 

#define	_hWnd	pCWP->hwnd
#define	_uMsg	pCWP->message
#define	_wParam pCWP->wParam
#define	_lParam pCWP->lParam

u32 __stdcall CallWndProc(s32 n, WPARAM w, CWPSTRUCT *pCWP)
{
	if (n == HC_ACTION)
	{
		switch(_uMsg)
		{
		//The song title is being updated ------
		// Check if the next song being played is playable by SNESAmp
		case(WM_SETTEXT):
			if (_hWnd == inMod.hMainWindow)
				SongChange(GetWAFile(_hWnd));
			break;

		//A popup menu is being displayed ------
		// Add an item to the popup menu to hide the Control dialog
		case(WM_INITMENUPOPUP):
			CtrlItem((HMENU)_wParam);
			break;

		//A window is about to be resized or moved
		case(WM_ENTERSIZEMOVE):
			hMoving = _hWnd;					//Save handle to window that's moving
			wndMoving = M_PREPARE_TB;			//A window may be about to move
			break;

		//A window has finished being resized or moved
		case(WM_EXITSIZEMOVE):
			tagSticky = 0;						//Flag SNESAmp dialogs as no longer sticky
			ctrlSticky = 0;
			wndMoving = M_NOTMOVING;			//No window is moving
			break;

		//A window's position is changing ------
		//Prepare for move by checking if any of the SNESAmp dialogs are visible and docked to the
		//moving window.
		case(WM_WINDOWPOSCHANGING):
			switch (wndMoving)
			{
			case(M_PREPARE):
				if (frmTag)
				{
					if (hMoving == GetMainWnd())
					{
						dockTest.hWindow = frmTag->Handle;
						dockTest.hRoot = hMoving;

						tagSticky = IsDockedPath(&dockTest);

						if (tagSticky)
						{
							frmTag->relTop = frmTag->Top - dockTest.rect[0].top;
							frmTag->relLeft = frmTag->Left - dockTest.rect[0].left;
						}
					}
				}

			//The tag editor can only dock to the main Winamp window.  If the moving window has a
			//title bar, we know it's a window the tag editor can't be docked to.
			case(M_PREPARE_TB):
				if (frmCtrl)
				{                            	//Set window handles in the DockTest structure
					dockTest.hWindow = frmCtrl->Handle;
					dockTest.hRoot = hMoving;

					if (hMoving == GetMainWnd())
						ctrlSticky = IsDockedPath(&dockTest);
					else
					if (hMoving != frmCtrl->Handle)
						ctrlSticky = IsDocked(&dockTest);

					if (ctrlSticky)
					{		  					//Save the relative position to the moving window
						frmCtrl->relTop = frmCtrl->Top - dockTest.rect[0].top;
						frmCtrl->relLeft = frmCtrl->Left - dockTest.rect[0].left;
					}
				}

				if (tagSticky || ctrlSticky)
					wndMoving = M_MOVING;   	//Start moving the docked dialogs
				else
				{
					hMoving = NULL;				//Neither window is docked
					wndMoving = M_NOTMOVING;
				}
			}
			break;

		//A window's position has changed ------
		case(WM_WINDOWPOSCHANGED):
			if (wndMoving != M_MOVING || hMoving != _hWnd) break;

			if (tagSticky)
			{
				frmTag->Top = frmTag->relTop + ((WINDOWPOS*)_lParam)->y;
				frmTag->Left = frmTag->relLeft + ((WINDOWPOS*)_lParam)->x;
			}

			if (ctrlSticky)
			{
				frmCtrl->Top = frmCtrl->relTop + ((WINDOWPOS*)_lParam)->y;
				frmCtrl->Left = frmCtrl->relLeft + ((WINDOWPOS*)_lParam)->x;
			}
			break;
			
		//Application is closing ---------------
		case(WM_CLOSE):
			if (_hWnd == inMod.hMainWindow) UnhookWinamp();	//Unhook from Winamp before going away
			break;
		}
	}

	return CallNextHookEx((HHOOK)hpCallWnd, n, w, (s32)pCWP);
}


//**************************************************************************************************
// Catch messages not passed to CallWndProc

u32 __stdcall MessageProc(s32 code, WPARAM w, MSG *pMsg)
{
	if (code == HC_ACTION)
	{
		switch(pMsg->message)
		{
		//======================================
		// Menu Messages

		//A hotkey was pressed or a menu item was selected
		// Hide or show the Control dialog, if the SNESAmp item was clicked in the popup menu
		case(WM_COMMAND):
			if (!(pMsg->wParam >> 16) && (u16)pMsg->wParam == (u16)itmCtrl)
			{
				if (frmCtrl->minimized)	   		//If the dialog is minimized, restore it
				{
					frmCtrl->minimized = 0;
					frmCtrl->ShowN();
				}
				else
				{
					frmCtrl->minimized = 1;
					frmCtrl->Hide();
				}
			}
			break;

		//======================================
		// Keyboard Messages

		//A key was pressed
		// Give the Control dialog the focus if the tab key was pressed
		case(WM_KEYDOWN):
			if ((s32)pMsg->lParam > 0 && pMsg->wParam == VK_TAB)
			{
				if (frmCtrl && frmCtrl->Visible && !frmCtrl->Active)
				{
					frmCtrl->hFocus = pMsg->hwnd;
					frmCtrl->SetFocus();
				}
			}
			break;

		//======================================
		// Mouse Messages
		//
		// Prepare for windows moving that don't have a titlebar
		//
		// Since there's no way to know when a Winamp window is about to move, we have to guess.
		// The only way a user can move a Winamp window is by clicking on it.  So each time the user
		// left clicks on a window, set the flag to prepare for a move.

		//The left mouse button is being pressed
		// The user may be about to move a window.
		case(WM_LBUTTONDOWN):
			hMoving = pMsg->hwnd;
			wndMoving = M_PREPARE;				//Window may be about to move
			break;

		//The left mouse button is being released
		// If the user moved a main window, set the status to no longer moving
		case(WM_LBUTTONUP):
			tagSticky = 0;
			ctrlSticky = 0;
			wndMoving = M_NOTMOVING;
			break;
		}
	}

	return CallNextHookEx((HHOOK)hpMsgProc, code, w, (u32)pMsg);
}


//**************************************************************************************************
// "SNESAmp Control" Menu Item
//
// Adds, removes, or sets the state of the popup menu item based on the Control dialog's state.
//
// In:
//    hmenu = Handle to popup menu

void CtrlItem(HMENU hmenu)
{
	MENUITEMINFO	mii;
	u32	i,count;
	s32	id;


	mii.cbSize = sizeof(MENUITEMINFO);

	//Verify menu ------------------------------
	mii.fMask = MIIM_ID;
	if (!GetMenuItemInfo(hmenu,0,1,&mii)) return;

	if (mii.wID != WINAMP_HELP_ABOUT) return;	//If first item isn't "Nullsoft Winamp...", this
												// isn't the popup menu we want
	//Set item ---------------------------------
	if (!itmCtrl)								//If our item doesn't exist in the menu...
	{
		if (frmCtrl)							//If the Control dialog is active, add our item
		{
			//Find a unique ID -----------------
			count = GetMenuItemCount(hmenu);

			for (i=0;i<count;i++)
			{
				id = GetMenuItemID(hmenu,i);
				if ((s32)itmCtrl < id) itmCtrl = id;
			}

			//Add a separator ------------------
			mii.fMask = MIIM_FTYPE |
						MIIM_ID;
			mii.fType = MFT_SEPARATOR;
			mii.wID = ++itmCtrl;

			InsertMenuItem(hmenu,count-2,1,&mii);

			//Add SNESAmp item -----------------
			mii.fMask = MIIM_FTYPE |
						MIIM_STATE |
						MIIM_ID |
						MIIM_STRING;
			mii.fType = MFT_STRING;
			mii.fState = (frmCtrl->showable ? MFS_ENABLED : MFS_DISABLED) |
						 (frmCtrl->minimized ? MFS_UNCHECKED : MFS_CHECKED);
			mii.wID = ++itmCtrl;
			mii.dwTypeData = (s8*)itmCtrlTxt;
			mii.cch = sizeof(itmCtrlTxt) - 1;

			InsertMenuItem(hmenu,count-1,1,&mii);
		}
	}
	else
	{
		if (frmCtrl)							//If the Control dialog is active, set the state of
		{										// the item
			mii.fMask =	MIIM_STATE;

			mii.fState = (frmCtrl->showable ? MFS_ENABLED : MFS_DISABLED) |
						 (frmCtrl->minimized ? MFS_UNCHECKED : MFS_CHECKED);

			SetMenuItemInfo(hmenu,itmCtrl,0,&mii);
		}
		else
		{
			DeleteMenu(hmenu,itmCtrl-1,0);		//If the Control dialog isn't active, remove the
			DeleteMenu(hmenu,itmCtrl,0);		// item from the menu
			itmCtrl = 0;
		}
	}
}


//**************************************************************************************************
// Get Winamp File
//
// Retrieves the path and filename of the current song in the playlist

s8* GetWAFile(HWND hwnd)
{
	u32		idx;
	s8		*fn;


	fn = NULL;
	if (SendMessage(hwnd, WM_WA_IPC, 0, IPC_GETLISTLENGTH))
	{
		idx = SendMessage(hwnd, WM_WA_IPC, 0, IPC_GETLISTPOS);
		fn = (s8*)SendMessage(hwnd, WM_WA_IPC, idx, IPC_GETPLAYLISTFILE);
	}

	return fn;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Exported Functions

//**************************************************************************************************
// Hook into Winamp Process

void HookWinamp()
{
	if (inMod.hMainWindow && !dockTest.thread)
	{
		dockTest.thread = GetWindowThreadProcessId(inMod.hMainWindow, NULL);
		Application->Handle = GetMainWnd();		//So we don't create more buttons on the taskbar
	}

	if (!cfg.tricks) return;
	
	if (!hpCallWnd)
		hpCallWnd = (HOOKPROC)SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProc, NULL, GetCurrentThreadId());

	if (!hpMsgProc)
		hpMsgProc = (HOOKPROC)SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)MessageProc, NULL, GetCurrentThreadId());
}


//**************************************************************************************************
// Unhook from Winamp Process

void UnhookWinamp()
{
	if (hpMsgProc)
	{
		UnhookWindowsHookEx((HHOOK)hpMsgProc);
		hpMsgProc = NULL;
	}

	if (hpCallWnd)
	{
		UnhookWindowsHookEx((HHOOK)hpCallWnd);
		hpCallWnd = NULL;
	}
}


//**************************************************************************************************
// Load Language Resource Module
//
// This code was adapted from Reinit.pas of the Richedit sample in the Demos directory of CBuilder5

b8 LoadLRM(u32 lcid)
{
	s8	fn[MAX_PATH];							//Path to in_snes
	s8	ln[4]="";								//Abbreviated language name
	s8	*p;										//Generic character pointer
	u32	iSize;
	u32	newInst;

	s32	idxBits,idxChn,idxInter;				//Values that don't get remembered
	s32	tagLeft,tagTop,idxEmu;
	s32	ctrlLeft,ctrlTop;

	PLibModule			curMod;
	VS_FIXEDFILEINFO	*pInfo;


	GetLocaleInfo(lcid ? lcid : LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, ln, 4);

	//Search for and load LRM ------------------
	if (ln[0])									//Was a name retreived?
	{
		strcpy(fn,dllPath);						//Get path to in_snes.dll
		p = strrchr(fn,'.');					//Append language to "in_snes.dll"
		p[0] = '_';
		p[4] = '.';
		strcpy(p+5, ln);

		iSize = GetFileVersionInfoSize(fn, 0);	//Get size of file information

		if (iSize)								//Was a file found that contained version info?
		{
			p = (s8*)malloc(iSize);
			GetFileVersionInfo(fn, 0, iSize, p);	//Get file version information
			VerQueryValue(p, "\\", (void**)&pInfo, &iSize);	//Search for binary data
			iSize = pInfo->dwFileVersionMS;	 		//Save major version
			free(p);

			if (iSize >= LRM_OLD && iSize <= LRM_NEW)	//If version matches SNESAmp, load LRM
			{
				newInst = (u32)LoadLibraryEx(fn, 0, LOAD_LIBRARY_AS_DATAFILE);

				if (newInst)
				{
					curMod = LibModuleList;
					while (curMod)
					{
						if (curMod->instance == (s32)HInstance)
						{
							if (curMod->resinstance != curMod->instance)
								FreeLibrary((void*)curMod->resinstance);

							curMod->resinstance = newInst;
							goto NewInstance;
						}
						curMod = curMod->next;
					}
				}
			}
		}
	}

	//Use default (English) help files ---------
	strcpy(fn, dllPath);
	strcpy(ScanStrR(fn,'.'), "hlp");
	Application->HelpFile = fn;

	strcpy(hlpFile, dllPath);
	strcpy(ScanStrR(hlpFile,'\\'), "SNESAmp.hlp");

	return 0;

NewInstance:
	//Reinitialize forms with new settings -----
	if (frmConfig)
	{
		idxBits		= frmConfig->cboBits->ItemIndex;
		idxChn		= frmConfig->cboChn->ItemIndex;
		idxInter	= frmConfig->cboInter->ItemIndex;
	}

	if (frmTag)
	{
		tagLeft		= frmTag->Left;
		tagTop		= frmTag->Top;
		idxEmu		= frmTag->cboEmu->ItemIndex;
	}

	if (frmCtrl)
	{
		ctrlLeft	= frmCtrl->Left;
		ctrlTop		= frmCtrl->Top;

		frmCtrl->Hide();						//Hide the control dialog to avoid problems
	}

	ReinitializeForms();						//Reinitialize forms with new properties

	//Set context help file --------------------
	p = ScanStrR(fn, '_');
	p[0] = 'h';
	p[1] = 'l';
	p[2] = 'p';
	if (GetFileAttributes(fn) == (u32)~0)		//Search for translated help file
	{
		p[-1] = '.';							//If not there, revert to in_snes.hlp
		p[3] = 0;
	}
	Application->HelpFile = fn;

	//Set main help file -----------------------
	strcpy(hlpFile, dllPath);
	p = ScanStrR(hlpFile, '\\');
	strcpy(p, "SNESAmp_hlp.");
	strcat(p, ln);
	if (GetFileAttributes(hlpFile) == (u32)~0)	//Search for translated help file
	{
		p[7] = '.';								//If not there, revert to SNESAmp.hlp
		p[11] = 0;
	}

	//Perform additional reinitialization ------
	if (frmConfig)
	{
		frmConfig->Reinit();
		frmConfig->cboBits->ItemIndex	= idxBits;
		frmConfig->cboChn->ItemIndex	= idxChn;
		frmConfig->cboInter->ItemIndex	= idxInter;
	}

	if (frmTag)
	{
		frmTag->Reinit();
		frmTag->Left = tagLeft;
		frmTag->Top = tagTop;
		frmTag->cboEmu->ItemIndex = idxEmu;
	}

	if (frmAbout) frmAbout->Reinit();

	if (frmCtrl)
	{
		frmCtrl->Reinit();
		SetAmp(-1);
		frmCtrl->SetMute(0);					//Restore mute checkboxes
		frmCtrl->Left = ctrlLeft;
		frmCtrl->Top = ctrlTop;
		frmCtrl->ShowN();
	}

	return 1;
}


//**************************************************************************************************
// Copy Preset

v0 CopyPreset(Settings &cfg)
{
	switch(cfg.preset)
	{
	case(-1):
		cfg.dsp.rate		= cfg.udsp.rate;
		cfg.dsp.bits		= cfg.udsp.bits;
		cfg.dsp.chn			= 2;
		cfg.dsp.mix			= MIX_FLOAT;
		cfg.dsp.inter		= 2;
		cfg.dsp.lowPass		= 0;
		cfg.dsp.surround	= 0;
		cfg.dsp.reverse		= 0;
		cfg.dsp.oldADPCM	= 0;
		cfg.dsp.noEcho		= 0;
		cfg.dsp.stereo		= 50;
		cfg.dsp.echo		= -50;

		cfg.mix.aar			= 2;
		cfg.mix.tagAmp		= 1;
		cfg.mix.reset		= 1;
		cfg.mix.amp			= 131072;
		cfg.mix.minAmp		= 65536;
		cfg.mix.maxAmp		= 524288;
		cfg.mix.threshold	= 33148;
		break;

	case(-2):
		cfg.dsp.rate		= cfg.udsp.rate;
		cfg.dsp.bits		= cfg.udsp.bits;
		cfg.dsp.chn			= 2;
		cfg.dsp.mix			= MIX_INT;
		cfg.dsp.inter		= 3;
		cfg.dsp.lowPass		= 1;
		cfg.dsp.surround	= 0;
		cfg.dsp.reverse		= 0;
		cfg.dsp.oldADPCM	= 0;
		cfg.dsp.noEcho		= 0;
		cfg.dsp.stereo		= 50;
		cfg.dsp.echo		= 100;

		cfg.mix.aar			= 0;
		cfg.mix.tagAmp		= 0;
		cfg.mix.reset		= 1;
		cfg.mix.amp			= 65536;
		cfg.mix.minAmp		= 65536;
		cfg.mix.maxAmp		= 524288;
		cfg.mix.threshold	= 32768;
		break;

	case(0):
		memcpy(&cfg.dsp, &cfg.udsp, sizeof(cfg.dsp));
		memcpy(&cfg.mix, &cfg.umix, sizeof(cfg.mix));
		break;
	}
}


//**************************************************************************************************
// Get Configuration

v0 LoadConfig(Settings &cfg)
{
	s8	temp[32];


	cfg.preset			= GetPrivateProfileInt("SNESAmp", "Preset", -1, cfg.iniFile);
	if (cfg.preset != -1 &&
		cfg.preset != -2 &&
		cfg.preset != 0) cfg.preset = -1;

	//DSP emulation ----------------------------
	cfg.udsp.rate		= GetPrivateProfileInt("SNESAmp", "Rate", 48000, cfg.iniFile);
	cfg.udsp.bits		= GetPrivateProfileInt("SNESAmp", "Bits", 16, cfg.iniFile);
	cfg.udsp.chn		= GetPrivateProfileInt("SNESAmp", "Channels", 2, cfg.iniFile);
	cfg.udsp.mix		= GetPrivateProfileInt("SNESAmp", "Quality", MIX_FLOAT, cfg.iniFile);
						  GetPrivateProfileString("SNESAmp","Inter", "Cubic", temp, 10, cfg.iniFile);
	cfg.udsp.lowPass	= GetPrivateProfileInt("SNESAmp", "Hardware", 0,cfg.iniFile);
	cfg.udsp.surround	= GetPrivateProfileInt("SNESAmp", "Surround", 0,cfg.iniFile);
	cfg.udsp.reverse	= GetPrivateProfileInt("SNESAmp", "Reverse", 0,cfg.iniFile);
	cfg.udsp.oldADPCM	= GetPrivateProfileInt("SNESAmp", "OldADPCM", 0,cfg.iniFile);
	cfg.udsp.noEcho		= GetPrivateProfileInt("SNESAmp", "NoEcho", 0,cfg.iniFile);
	cfg.udsp.stereo		= GetPrivateProfileInt("SNESAmp", "Stereo", 50,cfg.iniFile);;
	cfg.udsp.echo		= GetPrivateProfileInt("SNESAmp", "EFBCT", -50,cfg.iniFile);

	_Clamp(cfg.udsp.rate, (u32)(cfg.preset ? 32000 : 8000), 192000);

	if (cfg.udsp.bits != 8 &&
		cfg.udsp.bits != 16 &&
		cfg.udsp.bits != 24 &&
		cfg.udsp.bits != 32) cfg.udsp.bits = 16;
	if (cfg.udsp.chn != 1 && cfg.udsp.chn != 2) cfg.udsp.chn = 2;
	if (cfg.udsp.mix != MIX_INT && cfg.udsp.mix != MIX_FLOAT) cfg.udsp.mix = MIX_FLOAT;
	
	if (cfg.udsp.lowPass)
	{
		if (cfg.udsp.bits < 16) cfg.udsp.bits = 16;
		if (cfg.udsp.chn != 2) cfg.udsp.chn = 2;
		if (cfg.udsp.mix != MIX_INT) cfg.udsp.mix = MIX_INT;
	}
	else
	if (cfg.udsp.bits == 8 || cfg.udsp.chn == 1) cfg.udsp.mix = MIX_INT;

	cfg.udsp.inter = 0;
	while ((cfg.udsp.inter < 3) && (strcmpi(temp, interList[cfg.udsp.inter]))) cfg.udsp.inter++;
	if (cfg.udsp.bits == 8 || cfg.udsp.chn != 2) cfg.udsp.lowPass = 0;
	if (cfg.udsp.stereo > 100) cfg.udsp.stereo = 100;
	_Clamp(cfg.udsp.echo, -100, 100);

	//Mixing -----------------------------------
	cfg.umix.aar		= GetPrivateProfileInt("SNESAmp", "AAR", 2, cfg.iniFile);
	cfg.umix.tagAmp		= GetPrivateProfileInt("SNESAmp", "UseTagLevel", 1, cfg.iniFile);
	cfg.umix.reset		= GetPrivateProfileInt("SNESAmp", "ResetLevel", 1, cfg.iniFile);
	cfg.umix.amp		= GetPrivateProfileInt("SNESAmp", "Amp", 131072, cfg.iniFile);
	cfg.umix.minAmp		= GetPrivateProfileInt("SNESAmp", "MinAmp", 65536, cfg.iniFile);
	cfg.umix.maxAmp		= GetPrivateProfileInt("SNESAmp", "MaxAmp", 524288, cfg.iniFile);
	cfg.umix.threshold	= GetPrivateProfileInt("SNESAmp", "Threshold", 33149, cfg.iniFile);
	cfg.waVol			= GetPrivateProfileInt("SNESAmp", "WAVol", 0, cfg.iniFile) != 0;

	//381 = (2 ^ (0.1dB / 6)) * 32768) - 32768

	if (cfg.umix.aar > 2) cfg.umix.aar = 2;
	_Clamp(cfg.umix.minAmp, 32768, 524288);
	_Clamp(cfg.umix.maxAmp, cfg.umix.minAmp, 524288);
	_Clamp(cfg.umix.amp, cfg.umix.minAmp, cfg.umix.maxAmp);
	_Clamp(cfg.umix.threshold, 0x4000, 0x10000);	//+/- 6dB

	//Time options -----------------------------
	cfg.time.song		= GetPrivateProfileInt("SNESAmp", "Intro", 170000, cfg.iniFile);
	cfg.time.fade		= GetPrivateProfileInt("SNESAmp", "Fade", 10000, cfg.iniFile);
	cfg.time.silence 	= GetPrivateProfileInt("SNESAmp", "Silence", 2500, cfg.iniFile);
	cfg.time.autoEnd 	= GetPrivateProfileInt("SNESAmp", "AutoEnd", 3000, cfg.iniFile);
	cfg.time.loopx		= GetPrivateProfileInt("SNESAmp", "LoopCount", 0, cfg.iniFile);
	cfg.time.defBin		= GetPrivateProfileInt("SNESAmp", "BinTags", 0, cfg.iniFile);
	cfg.time.useTimer	= GetPrivateProfileInt("SNESAmp", "NoTime", 0, cfg.iniFile) == 0;
	cfg.time.fastSeek	= GetPrivateProfileInt("SNESAmp", "FastSeek", 0, cfg.iniFile);

	_Clamp(cfg.time.song, 1, 959000);
	cfg.time.song <<= 6;
	_Clamp(cfg.time.fade, 1, 59999);
	cfg.time.fade <<= 6;
	_Clamp(cfg.time.silence, 1, 59999);
	cfg.time.silence <<= 6;
	if (cfg.time.autoEnd > 59999) cfg.time.autoEnd = 59999;
	cfg.time.autoEnd <<= 6;
	if (cfg.time.loopx > 9) cfg.time.loopx = 9;

	//Title format -----------------------------
	GetPrivateProfileString("SNESAmp", "Title", "%9[%8[%8-]%9 ]%3 - %2", cfg.titleFmt, 63, cfg.iniFile);

	//Language resource module -----------------
	cfg.language	= GetPrivateProfileInt("SNESAmp", "Language", 0, cfg.iniFile);

	//Additional control -----------------------
	cfg.ctrl		= GetPrivateProfileInt("SNESAmp", "Control", 0, cfg.iniFile);
	cfg.tricks		= GetPrivateProfileInt("SNESAmp", "Tricks", 1, cfg.iniFile);
	cfg.pitch		= GetPrivateProfileInt("SNESAmp", "Pitch", 0, cfg.iniFile);
	cfg.speed		= GetPrivateProfileInt("SNESAmp", "Speed", 0, cfg.iniFile);
	_Clamp(cfg.pitch, -32768, 32768);
	_Clamp(cfg.speed, -32768, 32768);

	GetPrivateProfileString("SNESAmp", "FileExt", "SPC;SP*;RSN;ZST;ZS*", cfg.fileExt, 63, cfg.iniFile);

	//Window positions -------------------------
	cfg.ctrlTop		= GetPrivateProfileInt("SNESAmp", "CtrlTop", 0x10000, cfg.iniFile);
	cfg.ctrlLeft	= GetPrivateProfileInt("SNESAmp", "CtrlLeft", -1, cfg.iniFile);
	cfg.tagTop		= GetPrivateProfileInt("SNESAmp", "TagTop", -1, cfg.iniFile);
	cfg.tagLeft		= GetPrivateProfileInt("SNESAmp", "TagLeft", 0x10000, cfg.iniFile);

	//Select preset settings -------------------
	CopyPreset(cfg);
}


//**************************************************************************************************
// Set Configuration

#define	_WriteCfgI(v,k) \
		if (cfg.v != newcfg.v) \
		{ \
			cfg.v = newcfg.v; \
			sprintf(str, "%i", cfg.v); \
			WritePrivateProfileString("SNESAmp", k, str, cfg.iniFile); \
		}

v0 SaveConfig(Settings &newcfg)
{
	s8		str[8];
	b8		newDSP=false;						//New DSP settings need to take effect
	b8		newWA=false;						//New DSP settings need to reset Winamp output


	//==========================================
	// Save configuration changes to plugins.ini

	_WriteCfgI(preset,			"Preset");

	//DSP Emulator -----------------------------
	_WriteCfgI(udsp.rate,		"Rate");
	_WriteCfgI(udsp.bits,		"Bits");
	_WriteCfgI(udsp.chn,		"Channels");
	_WriteCfgI(udsp.mix,		"Quality");
	_WriteCfgI(udsp.lowPass,	"Hardware");
	_WriteCfgI(udsp.surround,	"Surround");
	_WriteCfgI(udsp.reverse,	"Reverse");
	_WriteCfgI(udsp.oldADPCM,	"OldADPCM");
	_WriteCfgI(udsp.noEcho,		"NoEcho");
	_WriteCfgI(udsp.stereo,		"Stereo");
	_WriteCfgI(udsp.echo,		"EFBCT");

	if (cfg.udsp.inter != newcfg.udsp.inter)
	{
		cfg.udsp.inter = newcfg.udsp.inter;
		WritePrivateProfileString("SNESAmp", "Inter", interList[cfg.udsp.inter], cfg.iniFile);
	}

	//Mixing Volume ----------------------------
	_WriteCfgI(umix.aar,		"AAR");
	_WriteCfgI(umix.tagAmp,		"UseTagLevel");
	_WriteCfgI(umix.reset,		"ResetLevel");
	_WriteCfgI(umix.minAmp,		"MinAmp");
	_WriteCfgI(umix.maxAmp,		"MaxAmp");
	_WriteCfgI(umix.amp,		"Amp");
	_WriteCfgI(umix.threshold,	"Threshold");

	if ((cfg.waVol & 1) != newcfg.waVol)
	{
		cfg.waVol &= ~1;
		cfg.waVol |= newcfg.waVol;
		SetAmp(-1);
		WritePrivateProfileString("SNESAmp", "WAVol", (cfg.waVol & 1) ? "1" : "0", cfg.iniFile);
	}

	//Timer Options ----------------------------
	if (cfg.time.song != newcfg.time.song)
	{
		cfg.time.song = newcfg.time.song;
		ID666::defSong = cfg.time.song;
		wsprintf(str, "%i", cfg.time.song >> 6);
		WritePrivateProfileString("SNESAmp", "Intro", str, cfg.iniFile);
	}

	if (cfg.time.fade != newcfg.time.fade)
	{
		cfg.time.fade = newcfg.time.fade;
		ID666::defFade = cfg.time.fade;
		wsprintf(str, "%i", cfg.time.fade >> 6);
		WritePrivateProfileString("SNESAmp", "Fade", str, cfg.iniFile);
	}

	if (cfg.time.silence != newcfg.time.silence)
	{
		cfg.time.silence = newcfg.time.silence;
		wsprintf(str, "%i", cfg.time.silence >> 6);
		WritePrivateProfileString("SNESAmp", "Silence", str, cfg.iniFile);
	}

	if (cfg.time.autoEnd != newcfg.time.autoEnd)
	{
		cfg.time.autoEnd = newcfg.time.autoEnd;
		wsprintf(str, "%i", cfg.time.autoEnd >> 6);
		WritePrivateProfileString("SNESAmp", "AutoEnd", str,cfg.iniFile);
	}

	_WriteCfgI(time.loopx,	"LoopCount");

	if (cfg.time.defBin != newcfg.time.defBin)
	{
		cfg.time.defBin = newcfg.time.defBin;
		ID666::preferBin = cfg.time.defBin;
		WritePrivateProfileString("SNESAmp", "BinTags", cfg.time.defBin ? "1" : "0", cfg.iniFile);
	}

	if (cfg.time.useTimer != newcfg.time.useTimer)
	{
		cfg.time.useTimer = newcfg.time.useTimer;
		SetTimer(0, cfg.time.useTimer);
		WritePrivateProfileString("SNESAmp", "NoTime", cfg.time.useTimer ? "0" : "1", cfg.iniFile);
		if (frmCtrl) frmCtrl->itmTimer->Enabled = cfg.time.useTimer;
		if (frmTag) frmTag->chkTimer->Enabled = cfg.time.useTimer;
	}

	if (cfg.time.fastSeek != newcfg.time.fastSeek)
	{
		cfg.time.fastSeek = newcfg.time.fastSeek;
		WritePrivateProfileString("SNESAmp", "FastSeek", cfg.time.fastSeek ? "1" : "0", cfg.iniFile);
	}

	//Title format -----------------------------
	if (strcmp(cfg.titleFmt, newcfg.titleFmt))
	{
		strcpy(cfg.titleFmt, newcfg.titleFmt);
		WritePrivateProfileString("SNESAmp", "Title", cfg.titleFmt, cfg.iniFile);
	}

	//Lanugage resource module -----------------
	if (newcfg.language != -1 && cfg.language != newcfg.language)
	{
		cfg.language = newcfg.language;
		str[0] = 0;
		if (cfg.language)						//Check if we should force a module to load
		{
			if (LoadLRM(cfg.language))
				wsprintf(str, "%i", cfg.language);
		}
		else
		{
			if (!LoadLRM(0))
				Application->MessageBox("The default language resource module for your system " \
										"could not be found.", "SNESAmp", MB_OK|MB_ICONEXCLAMATION);
		}

		if (str[0] != 0)
			WritePrivateProfileString("SNESAmp", "Language", str,cfg.iniFile);
		else
			WritePrivateProfileString("SNESAmp", "Language", NULL, cfg.iniFile); //Erase entry
	}

	//Associated file extensions ---------------
	if (strcmp(cfg.fileExt, newcfg.fileExt))
	{
		strcpy(cfg.fileExt, newcfg.fileExt);
		WritePrivateProfileString("SNESAmp", "FileExt", cfg.fileExt, cfg.iniFile);
		
		*(1 + CopyStr(1 + CopyStr(inMod.fileExt, cfg.fileExt), "SNES music files (*.SPC, *.RSN, *.ZST)")) = 0;
	}

	//Display Control window? ------------------
	if (cfg.ctrl != newcfg.ctrl)
	{
		cfg.ctrl = newcfg.ctrl;
		if (!cfg.ctrl)
		{
			WritePrivateProfileString("SNESAmp", "CtrlTop", NULL, cfg.iniFile);
			WritePrivateProfileString("SNESAmp", "CtrlLeft", NULL, cfg.iniFile);
			if (frmCtrl) frmCtrl->Close();
		}
		WritePrivateProfileString("SNESAmp", "Control", cfg.ctrl ? "1" : "0", cfg.iniFile);
	}

	//Enable tricks ----------------------------
	if (cfg.tricks != newcfg.tricks)
	{
		cfg.tricks = newcfg.tricks;

		if (!cfg.tricks)
		{
			UnhookWinamp();
			ShutDock();
		}
		else
		{
			if (frmTag || frmCtrl) HookWinamp();
			InitDock();
		}

		if (frmTag) frmTag->chkUpdate->Enabled = cfg.tricks;
		if (frmCtrl) frmCtrl->itmClose->Visible = !cfg.tricks;

		WritePrivateProfileString("SNESAmp", "Tricks", cfg.tricks ? "1" : "0", cfg.iniFile);
	}


	//==========================================
	// Update emulator settings

	CopyPreset(newcfg);

	if (cfg.dsp.rate != newcfg.dsp.rate ||
		cfg.dsp.bits != newcfg.dsp.bits ||
		cfg.dsp.chn != newcfg.dsp.chn)
	{
		newWA = true;

		if (frmCtrl)
		{
			frmCtrl->trkStereo->Enabled = (newcfg.dsp.chn == 2);
			frmCtrl->lblStereo->Enabled = (newcfg.dsp.chn == 2);
			frmCtrl->trkEcho->Enabled = (newcfg.dsp.chn == 2);
			frmCtrl->lblEcho->Enabled = (newcfg.dsp.chn == 2);
		}
	}

	if (cfg.dsp.mix != newcfg.dsp.mix ||
		cfg.dsp.inter != newcfg.dsp.inter ||
		cfg.dsp.lowPass != newcfg.dsp.lowPass ||
		cfg.dsp.surround != newcfg.dsp.surround ||
		cfg.dsp.reverse != newcfg.dsp.reverse ||
		cfg.dsp.noEcho != newcfg.dsp.noEcho ||
		cfg.dsp.oldADPCM != newcfg.dsp.oldADPCM)
	{
		newDSP = true;
		if (frmCtrl) frmCtrl->itmADPCM->Checked = newcfg.dsp.oldADPCM;
	}

	if (cfg.dsp.stereo != newcfg.dsp.stereo)
	{
		if (frmCtrl) frmCtrl->trkStereo->Position = 100 - newcfg.dsp.stereo;
		else sapu.SetDSPStereo((newcfg.dsp.stereo << 16) / 100);
	}

	if (cfg.dsp.echo != newcfg.dsp.echo)
	{
		if (frmCtrl) frmCtrl->trkEcho->Position = -newcfg.dsp.echo;
		else sapu.SetDSPEFBCT((newcfg.dsp.echo << 15) / 100);
	}

	newcfg.mix.aar |= cfg.mix.aar & ~AAR_TYPE;

	if (frmCtrl)
	{
		if (cfg.mix.minAmp != newcfg.mix.minAmp)
			frmCtrl->trkAmp->SelEnd = 180 - F2I(YLog2(60.0, newcfg.mix.minAmp / 65536.0));

		if (cfg.mix.maxAmp != newcfg.mix.maxAmp)
			frmCtrl->trkAmp->SelStart = 180 - F2I(YLog2(60.0, newcfg.mix.maxAmp / 65536.0));
	}

	if (cfg.mix.amp != newcfg.mix.amp)
	{
		if (!newcfg.preset && (newcfg.mix.aar & AAR_TYPE) == 0) SetAmp(newcfg.mix.amp);
	}

	memcpy(&cfg.dsp, &newcfg.dsp, sizeof(cfg.dsp));
	memcpy(&cfg.mix, &newcfg.mix, sizeof(cfg.mix));

	if (newDSP || newWA) ChangeOutput(newWA);
}


//**************************************************************************************************
// Reset Configuration

v0 ResetConfig()
{
	Settings	newCfg;

	WritePrivateProfileString("SNESAmp", NULL, NULL, cfg.iniFile);	//Erase all settings
	LoadConfig(newCfg);							//LoadConfig will load defaults
	SaveConfig(newCfg);							//Update SNESAmp with new settings
}


//**************************************************************************************************
// Get Main Window Handle

HWND __fastcall GetMainWnd()
{
	WINDOWPLACEMENT	wndpl;


	if (!inMod.hMainWndNew)
	{
		inMod.hMainWndNew = FindWindow("BaseWindow_RootWnd", "Player Window");
		if (!inMod.hMainWndNew) inMod.hMainWndNew = inMod.hMainWindow;
	}

	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(inMod.hMainWindow, &wndpl);

	if (wndpl.rcNormalPosition.left == 3000 &&
		wndpl.rcNormalPosition.top == 3000)
	{
		return inMod.hMainWndNew;
	}

	return inMod.hMainWindow;
}


//**************************************************************************************************
// Get Relative Position to Winamp's Main Window

b8 __fastcall GetRelPos(TForm *pFrm, s32 &top, s32 &left)
{
	WINDOWPLACEMENT	wp;
	RECT			frm,wa;
	b8				sticky;


	sticky = 0;

	frm = pFrm->BoundsRect;

	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(GetMainWnd(), &wp);
	wa = wp.rcNormalPosition;

	if ( (frm.left   == wa.right || frm.right  == wa.left) &&
		((frm.top    >= wa.top   && frm.top    <= wa.bottom) ||
		 (frm.bottom >= wa.top   && frm.bottom <= wa.bottom)) ) sticky = 1;
	else
	if ( (frm.top   == wa.bottom || frm.bottom == wa.top) &&
		((frm.left  >= wa.left   && frm.left   <= wa.right) ||
		 (frm.right >= wa.left   && frm.right  <= wa.right)) ) sticky = 1;

	if (sticky)
	{
		top = 0x80000000 | ((frm.top - wa.top) & 0xFFFF);
		left = 0x80000000 | ((frm.left - wa.left) & 0xFFFF);

		return 1;
	}

	top = frm.top & 0xFFFF;
	left = frm.left & 0xFFFF;

	return 0;
}


//**************************************************************************************************
// Set Position Relative to Winamp's Main Window

void __fastcall SetRelPos(TForm *pFrm, s32 top, s32 left)
{
	WINDOWPLACEMENT	wp;
	s16	pos;


	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(GetMainWnd(), &wp);

	//Select top position ----------------------
	pos = top >> 16;

	if (pos == 0)
		pFrm->Top = (s16)top;
	else
	if (pos == -32768)
		pFrm->Top = (s16)top + wp.rcNormalPosition.top;
	else
	if (pos > 0)
		pFrm->Top = wp.rcNormalPosition.top;
	else
	{
		if ((s32)wp.rcNormalPosition.top > 0)
			pFrm->Top = wp.rcNormalPosition.top - pFrm->Height;
		else
			pFrm->Top = wp.rcNormalPosition.bottom;
	}

	//Select left position ---------------------
	pos = left >> 16;

	if (pos == 0)
		pFrm->Left = (s16)left;
	else
	if (pos == -32768)
		pFrm->Left = (s16)left + wp.rcNormalPosition.left;
	else
	if (pos > 0)
		pFrm->Left = wp.rcNormalPosition.left;
	else
	{
		if ((s32)wp.rcNormalPosition.left > 0)
			pFrm->Left = wp.rcNormalPosition.left - pFrm->Width;
		else
			pFrm->Left = wp.rcNormalPosition.right;
	}
}
