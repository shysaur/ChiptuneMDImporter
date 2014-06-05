/***************************************************************************************************
* Form: Control Dialog                                                                             *
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
*                                                      Copyright (C)2001-2003 Alpha-II Productions *
***************************************************************************************************/

#ifndef ControlH
#define ControlH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <Menus.hpp>


//**************************************************************************************************
class TfrmCtrl:public TForm
{
__published:	// IDE-managed Components
	TPopupMenu	*popHelp;
	TMenuItem	*itmWhat;
	TMenuItem	*itmSolo;

	TPopupMenu	*popMenu;
	TMenuItem	*itmCfg;
	TMenuItem	*itmAbout;
	TMenuItem	*sep1;
	TMenuItem	*itmDisable;
	TMenuItem	*itmClose;
	TMenuItem	*sep2;
	TMenuItem	*itmADPCM;
	TMenuItem	*sep3;
	TMenuItem	*itmHelp;
	TMenuItem	*itmUnmute;
	TMenuItem	*itmTimer;

	TImage		*imgBanner;
	TCheckBox	*chk1;
	TCheckBox	*chk2;
	TCheckBox	*chk3;
	TCheckBox	*chk4;
	TCheckBox	*chk5;
	TCheckBox	*chk6;
	TCheckBox	*chk7;
	TCheckBox	*chk8;
	TTrackBar	*trkAmp;
	TTrackBar	*trkPitch;
	TTrackBar	*trkSpeed;
	TTrackBar	*trkStereo;
	TTrackBar	*trkEcho;
	TLabel		*lblAmp;
	TLabel		*lblPitch;
	TLabel		*lblSpeed;
	TLabel		*lblStereo;
	TLabel		*lblEcho;
	TImage		*imgPitch;
	TImage		*imgStereo;
	TImage		*imgAmp;
	TImage		*imgSpeed;
	TImage		*imgEcho;
	TImage		*imgMute;

	void __fastcall FormCreate(TObject *sender);
	void __fastcall FormKeyDown(TObject *sender, WORD &key, TShiftState shift);
	void __fastcall itmCfgClick(TObject *sender);
	void __fastcall itmAboutClick(TObject *sender);
	void __fastcall itmADPCMClick(TObject *sender);
	void __fastcall itmTimerClick(TObject *sender);
	void __fastcall itmDisableClick(TObject *sender);
	void __fastcall itmCloseClick(TObject *sender);
	void __fastcall itmHelpClick(TObject *sender);
	void __fastcall itmSoloClick(TObject *sender);
	void __fastcall itmUnmuteClick(TObject *sender);
	void __fastcall itmWhatClick(TObject *sender);
	void __fastcall popHelpPopup(TObject *sender);
	void __fastcall imgBannerMouseDown(TObject *sender, TMouseButton button, TShiftState shift, int x, int y);
	void __fastcall imgBannerMouseUp(TObject *sender, TMouseButton button, TShiftState shift, int x, int y);
	void __fastcall imgBannerMouseMove(TObject *sender, TShiftState shift, int x, int y);
	void __fastcall chkMuteChange(TObject *sender);
	void __fastcall trkAmpChange(TObject *sender);
	void __fastcall trkPitchChange(TObject *sender);
	void __fastcall trkSpeedChange(TObject *sender);
	void __fastcall trkStereoChange(TObject *sender);
	void __fastcall trkEchoChange(TObject *sender);
	void __fastcall SetAlpha();

private:	// User declarations
	b8			setAmp;							//Whether trkAmpChange should set amp globally
	b8			moving;							//Dialog is moving

	u32			curMute;						//Current mute flags (could be set from ID666 tag)
	u32			usrMute;						//Mute flags set by user
	u32			setMute;						//1 if mute is being modified by user, 0 otherwise
	TCheckBox	*chkMute[8];					//Pointer to each mute checkbox
	s32			echo;							//Echo feedback crosstalk

	DockTest	dockTest;
	TPoint		adjust;							//Used for moving dialog when tricks are disabled


	void		__fastcall CreateParams(TCreateParams &params);
	static b8	__stdcall EnumThreadWndProc(HWND hwnd, LPARAM lParam);

public:
	u32			speed;							//Current speed adjustment
	s32			relTop,relLeft;					//Position relative to moving window
	b8			minimized;
	b8			showable;
	HANDLE		hFocus;							//Window that had the focus before


	__fastcall TfrmCtrl(TComponent* owner);


	//**********************************************************************************************
	// Reinitialize Form
	//
	// Initializes data affected by changing the language resource module

	void __fastcall Reinit();


	//**********************************************************************************************
	// Show Form without Focus
	//
	// Same as Show(), but the input focus is returned to the window that previously had it

	void __fastcall ShowN();


	//**********************************************************************************************
	// Set Muted Voices
	//
	// Sets the mute state of each voice without changing the user state
	//
	// In:
	//    mute = Corresponding bit is set if voice is muted
	//           Pass 0 to return to the user defined mute states

	void __fastcall SetMute(u32 mute);


	//**********************************************************************************************
	// Set Amplification Level
	//
	// Call this to update the amp slider
	//
	// In:
	//    amp = Amplification level

	void __fastcall SetAmp(u32 amp);
};

extern PACKAGE TfrmCtrl *frmCtrl;
#endif
