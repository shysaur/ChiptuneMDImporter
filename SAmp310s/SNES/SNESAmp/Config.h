/***************************************************************************************************
* Form: Configuration Dialog                                                                       *
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

#ifndef ConfigH
#define ConfigH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Menus.hpp>


//**************************************************************************************************
class TfrmConfig:public TForm
{
__published:	// IDE-managed Components
	TPopupMenu	*popHelp;
	TMenuItem	*itmWhat;

	TButton		*btnHelp;
	TButton		*btnOK;
	TButton		*btnCancel;
	TButton		*btnApply;
	TPageControl	*pgcCfg;
	TTabSheet	*tabDSP;
	TTabSheet	*tabTime;
	TTabSheet	*tabMixing;
	TTabSheet	*tabTitle;
	TTabSheet	*tabOther;

	TRadioGroup	*grpPreset;
	TLabel		*lblRate;
	TLabel *lblBits;
	TLabel		*lblChn;
	TLabel		*lblInter;
	TLabel		*lblTStereo;
	TLabel		*lblStereo;
	TLabel		*lblTEcho;
	TLabel		*lblEcho;
	TComboBox	*cboRate;
	TComboBox *cboBits;
	TComboBox	*cboChn;
	TComboBox	*cboInter;
	TCheckBox	*chkLow;
	TCheckBox	*chkSurround;
	TCheckBox	*chkReverse;
	TCheckBox	*chkADPCM;
	TCheckBox	*chkNoEcho;
	TTrackBar	*trkStereo;
	TTrackBar	*trkEcho;

	TBevel		*bvlMixing;
	TLabel		*lblMixing;
	TLabel		*lblTAmp;
	TLabel		*lblAmp;
	TLabel		*lblTThresh;
	TLabel		*lblThresh;
	TRadioGroup *grpAAR;
	TTrackBar	*trkAmp;
	TEdit		*txtMinAmp;
	TEdit		*txtMaxAmp;
	TTrackBar	*trkThresh;
	TCheckBox	*chkTagAmp;
	TCheckBox	*chkReset;
	TCheckBox	*chkWAVol;

	TBevel		*bvlTime;
	TLabel		*lblTime;
	TLabel		*lblSong;
	TLabel		*lblColon;
	TLabel		*lblFade;
	TLabel		*lblSilence;
	TLabel		*lblEnd;
	TLabel		*lblLoopx;
	TEdit		*txtSongMin;
	TEdit		*txtSongSec;
	TEdit		*txtFade;
	TEdit		*txtSilence;
	TEdit		*txtEnd;
	TEdit		*txtLoopX;
	TCheckBox	*chkTimer;
	TCheckBox	*chkFastSeek;
	TCheckBox	*chkBinTag;

	TBevel		*bvlTitle;
	TLabel		*lblTitle;
	TEdit		*txtTitle;
	TLabel		*lbl0;
	TLabel		*lbl1;
	TLabel		*lbl2;
	TLabel		*lbl3;
	TLabel		*lbl4;
	TLabel		*lbl5;
	TLabel		*lbl6;
	TLabel		*lbl7;
	TLabel		*lbl8;
	TLabel		*lbl9;
	TLabel		*lblA;
	TLabel		*lblB;
	TLabel		*lblC;
	TLabel		*lblD;
	TLabel		*lblCondition;
	TLabel		*lblCopyL;
	TLabel		*lblCopyE;
	TLabel		*lblCopyG;
	TLabel		*lblTLang;
	TLabel		*lblTLangd;
	TComboBox	*cboLang;
	TLabel		*lblA2Link;
	TBevel		*bvlCtrl;
	TLabel		*lblCtrl;
	TCheckBox	*chkCtrl;
	TCheckBox	*chkTricks;
	TLabel		*lblReset;
	TButton		*btnReset;
	TEdit *txtExt;
	TLabel *lblExt;
	TButton *btnExt;
	TBevel *Bevel1;

	void __fastcall FormCreate(TObject *sender);
	void __fastcall FormShow(TObject *sender);
	void __fastcall FormMouseMove(TObject *sender, TShiftState shift, int x, int y);
	void __fastcall lblLinkMouseMove(TObject *sender, TShiftState shift, int x, int y);
	void __fastcall itmWhatClick(TObject *sender);
	void __fastcall btnHelpClick(TObject *sender);
	void __fastcall btnOKClick(TObject *sender);
	void __fastcall btnCancelClick(TObject *sender);
	void __fastcall btnApplyClick(TObject *sender);
	void __fastcall btnResetClick(TObject *sender);
	void __fastcall tabOtherShow(TObject *sender);
	void __fastcall ItemChange(TObject *sender);
	void __fastcall grpPresetClick(TObject *sender);
	void __fastcall cboRateExit(TObject *sender);
	void __fastcall cboBitsChange(TObject *sender);
	void __fastcall cboChnChange(TObject *sender);
	void __fastcall trkStereoChange(TObject *sender);
	void __fastcall trkEchoChange(TObject *sender);
	void __fastcall txtEndExit(TObject *sender);
	void __fastcall txtLoopXExit(TObject *sender);
	void __fastcall grpAARClick(TObject *sender);
	void __fastcall trkAmpChange(TObject *sender);
	void __fastcall trkThreshChange(TObject *sender);
	void __fastcall txtMinAmpExit(TObject *sender);
	void __fastcall txtMaxAmpExit(TObject *sender);
	void __fastcall txtSongMinExit(TObject *sender);
	void __fastcall CheckTime(TObject *sender);
	void __fastcall lblSMLinkClick(TObject *sender);
	void __fastcall lblA2LinkClick(TObject *sender);
	void __fastcall btnExtClick(TObject *Sender);
	void __fastcall cboRateChange(TObject *Sender);


private:	// User declarations
	static b8	searched;						//Language modules have already been searched for
	TCursor		oldCursor;

	CfgDSP		udsp;
	CfgMix		umix;

	void	__fastcall SaveCfg();
	void	__fastcall FindLRM();

public:		// User declarations
	__fastcall TfrmConfig(TComponent* owner);

	//**********************************************************************************************
	// Reinitialize Form
	//
	// Initializes data affected by changing the language resource module

	void	__fastcall Reinit();
};

extern PACKAGE TfrmConfig *frmConfig;
#endif

