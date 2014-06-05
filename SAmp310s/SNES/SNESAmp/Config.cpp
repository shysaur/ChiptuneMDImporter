/***************************************************************************************************
* Configuration Dialog                                                                             *
*                                                      Copyright (C)2001-2003 Alpha-II Productions *
***************************************************************************************************/

#include	<vcl.h>
#include	"SNESAmp.h"
#pragma	hdrstop
#include	<dir.h>
#include	<math.h>
#include	"Config.h"


//**************************************************************************************************
#pragma	package(smart_init)
#pragma	resource "*.dfm"

b8	TfrmConfig::searched;


TfrmConfig	*frmConfig;


//**************************************************************************************************
__fastcall TfrmConfig::TfrmConfig(TComponent* owner) : TForm(Owner)
{
}


//**************************************************************************************************
void __fastcall TfrmConfig::Reinit()
{
	grpPreset->HelpContext		= Cfg_Preset;
	cboRate->HelpContext		= Cfg_Rate;
	cboBits->HelpContext		= Cfg_Bits;
	cboChn->HelpContext			= Cfg_Chn;
	cboInter->HelpContext		= Cfg_Inter;
	chkLow->HelpContext			= Cfg_Analog;
	chkSurround->HelpContext	= Cfg_Wide;
	chkReverse->HelpContext		= Cfg_Reverse;
	chkADPCM->HelpContext		= Cfg_ADPCM;
	chkNoEcho->HelpContext		= Cfg_NoEcho;
	trkStereo->HelpContext		= Cfg_Stereo;
	trkEcho->HelpContext		= Cfg_EFBCT;

	grpAAR->HelpContext			= Cfg_AAR;
	trkAmp->HelpContext			= Cfg_Amp;
	trkThresh->HelpContext		= Cfg_Thresh;
	txtMinAmp->HelpContext		= Cfg_Min;
	txtMaxAmp->HelpContext		= Cfg_Max;
	chkTagAmp->HelpContext		= Cfg_TagAmp;
	chkReset->HelpContext		= Cfg_RstAmp;
	chkWAVol->HelpContext		= Cfg_WAVol;

	txtSongMin->HelpContext		= Cfg_Length;
	txtSongSec->HelpContext		= Cfg_Length;
	txtFade->HelpContext		= Cfg_Length;
	txtSilence->HelpContext		= Cfg_Silence;
	txtEnd->HelpContext			= Cfg_End;
	txtLoopX->HelpContext		= Cfg_LoopX;
	chkTimer->HelpContext		= Cfg_Timer;
	chkFastSeek->HelpContext	= Cfg_Fast;
	chkBinTag->HelpContext		= Cfg_Bin;

	txtTitle->HelpContext		= Cfg_Title;

	cboLang->HelpContext		= Cfg_Lang;
	txtExt->HelpContext			= Cfg_FileExt;
	chkCtrl->HelpContext		= Cfg_Ctrl;
	chkTricks->HelpContext		= Cfg_Tricks;
	btnReset->HelpContext		= Cfg_Reset;

	HelpFile = Application->HelpFile;

	cboBits->Items->Clear();
	cboBits->Items->AddObject("8-bit", (TObject*)8);
	cboBits->Items->AddObject("16-bit", (TObject*)16);
	cboBits->Items->AddObject("24-bit", (TObject*)24);
	cboBits->Items->AddObject("32-bit", (TObject*)32);
}


//**************************************************************************************************
void __fastcall TfrmConfig::FormCreate(TObject *sender)
{
	Reinit();

	searched = 0;								//LRM list is empty
}


//**************************************************************************************************
void __fastcall TfrmConfig::FormShow(TObject *sender)
{
	u32	i;


	//Copy user defined settings ---------------
	memcpy(&udsp, &cfg.udsp, sizeof(udsp));
	memcpy(&umix, &cfg.umix, sizeof(umix));

	//Set preset and initialize controls -------
	cboRate->Text			= udsp.rate;
	cboBits->ItemIndex		= cboBits->Items->IndexOfObject((TObject*)udsp.bits);

	switch(cfg.preset)
	{
	case(0): grpPreset->ItemIndex = 2; break;
	case(-1): grpPreset->ItemIndex = 0; break;
	case(-2): grpPreset->ItemIndex = 1; break;
	}

	//Set only mixing option -------------------
	chkWAVol->Checked		= (cfg.waVol & 1) != 0;

	//Set time options -------------------------
	txtSongMin->Text		= cfg.time.song / XID6_TICKSMIN;
	txtSongSec->Text		= FloatToStrF((cfg.time.song % XID6_TICKSMIN) / XID6_TICKSSEC, ffFixed, 5, 2);
	txtFade->Text			= FloatToStrF(cfg.time.fade / XID6_TICKSSEC, ffFixed, 5, 2);
	txtSilence->Text		= FloatToStrF(cfg.time.silence / XID6_TICKSSEC, ffFixed, 5, 2);
	if (cfg.time.autoEnd)
		txtEnd->Text		= FloatToStrF(cfg.time.autoEnd / XID6_TICKSSEC, ffFixed, 5, 2);
	else
		txtEnd->Text		= "";
	if (cfg.time.loopx)
		txtLoopX->Text		= cfg.time.loopx;
	else
		txtLoopX->Text		= "";
	chkTimer->Checked		= cfg.time.useTimer;
	chkFastSeek->Checked	= cfg.time.fastSeek;
	chkBinTag->Checked		= cfg.time.defBin;

	//Fill in title format ---------------------
	txtTitle->Text			= cfg.titleFmt;

	//Reset language list ----------------------
	cboLang->Text			= "";
	cboLang->ItemIndex		= -1;

	txtExt->Text			= cfg.fileExt;

	chkCtrl->Checked		= cfg.ctrl;
	chkTricks->Checked		= cfg.tricks;

	btnApply->Enabled		= false;
}


//**************************************************************************************************
void __fastcall TfrmConfig::FormMouseMove(TObject *sender, TShiftState shift, int x, int y)
{
	if (Screen->Cursor == crHandPoint) Screen->Cursor = oldCursor;
}

void __fastcall TfrmConfig::lblLinkMouseMove(TObject *sender, TShiftState shift, int x, int y)
{
	if (Screen->Cursor != crHandPoint)
	{
		oldCursor = Screen->Cursor;
		Screen->Cursor = crHandPoint;
	}
}


//**************************************************************************************************
// Displays context help when user clicks on "What's this?" in the popup menu

void __fastcall TfrmConfig::itmWhatClick(TObject *sender)
{
	TWinControl *pWinControl = static_cast<TWinControl*>(popHelp->PopupComponent);

	Application->HelpCommand(HELP_CONTEXTPOPUP,pWinControl->HelpContext);
}


//**************************************************************************************************
void __fastcall TfrmConfig::btnHelpClick(TObject *sender)
{
	WinHelp(frmConfig->Handle,hlpFile,HELP_CONTEXT,9999);
}


//**************************************************************************************************
void __fastcall TfrmConfig::btnOKClick(TObject *sender)
{
	SaveCfg();
	Close();
}


//**************************************************************************************************
void __fastcall TfrmConfig::btnCancelClick(TObject *sender)
{
	Close();
}


//**************************************************************************************************
void __fastcall TfrmConfig::btnApplyClick(TObject *sender)
{
	SaveCfg();
	FormShow(NULL);								//Restore current settings incase user changed lang
	btnApply->Enabled = false;
}


//**************************************************************************************************
// Search for Language Resource Modules (LRM) when the user selects the "Other" tab

void __fastcall TfrmConfig::tabOtherShow(TObject *sender)
{
	if (searched) return;						//Only search once per session of Winamp

	FindLRM();
	searched = 1;
}


//**************************************************************************************************
// Find Language Resource Modules
//
// Searches the plug-ins directory for "in_snes_dll.*".  Any files with a version matching SNESAmp's
// will be added to the language list.

void __fastcall TfrmConfig::FindLRM()
{
	s8	   	path[MAX_PATH],*t;
	ffblk  	find;
	u32		bSize,iSize;
	u16		*pInfoBlk,*pLCID;
	VS_FIXEDFILEINFO	*pInfo;
	s8		*pInternal;
	s8		SubBlock[256];
                                                      

	cboLang->Text = "Searching for languages...";

	strcpy(path, dllPath);						//Create search path for LRM's
	strcpy(strrchr(path, '.'), "_dll.*");
	t = ScanStrR(path,'\\');					//t -> filename in path

	if (!findfirst(path,&find,0))
	{
		//Repeat for each LRM file found -------
		do
		{
			strcpy(t,find.ff_name);    			//Copy filename to path
			bSize = GetFileVersionInfoSize(path,0);	//Get size of version information

			if (bSize)							//Does version information exist?
			{
				//Read in version information --
				pInfoBlk = (u16*)malloc(bSize);
				GetFileVersionInfo(path, 0, bSize, pInfoBlk);

				//Get the LCID -----------------
				VerQueryValue(pInfoBlk, "\\", (void**)&pInfo, &iSize);
				VerQueryValue(pInfoBlk, "\\VarFileInfo\\Translation", (void**)&pLCID, &iSize);

				//LRM version must match current version of SNESAmp
				if (pInfo->dwFileVersionMS >= LRM_OLD && pInfo->dwFileVersionMS <= LRM_NEW)
				{
					//Get the internal name of the file
					wsprintf(SubBlock, TEXT("\\StringFileInfo\\%04x%04x\\InternalName"), pLCID[0], pLCID[1]);
					VerQueryValue(pInfoBlk, SubBlock, (void**)&pInternal, &iSize);

					//If the internal name is "SAMPLRM" then this is a valid LRM
					if (pInternal && !strcmp(pInternal, "SAMPLRM"))
						cboLang->Items->AddObject(pLanguages->NameFromLocaleID[*pLCID], (TObject*)*pLCID);
				}

				free(pInfoBlk);
			}
		}
		while (!findnext(&find));
	}

	findclose(&find);

	cboLang->Text = "";
}


//**************************************************************************************************
// Enable the "Apply" button if any item changes

void __fastcall TfrmConfig::ItemChange(TObject *sender)
{
	if (!btnApply->Enabled) btnApply->Enabled = true;
}


//**************************************************************************************************
void __fastcall TfrmConfig::grpPresetClick(TObject *sender)
{
	b8	state;


	//En/Disable controls based on preset selected
	state = (grpPreset->ItemIndex == 2);

	lblChn->Enabled			= state;
	cboChn->Enabled			= state;
	lblInter->Enabled		= state;
	cboInter->Enabled		= state;
	chkLow->Enabled			= state;
	chkSurround->Enabled	= state;
	chkReverse->Enabled		= state;
	chkADPCM->Enabled		= state;
	chkNoEcho->Enabled		= state;
	lblTStereo->Enabled		= state;
	lblStereo->Enabled		= state;
	trkStereo->Enabled		= state;
	lblTEcho->Enabled		= state;
	lblEcho->Enabled		= state;
	trkEcho->Enabled		= state;

	grpAAR->Enabled			= state;
	lblTAmp->Enabled		= state;
	lblAmp->Enabled			= state;
	trkAmp->Enabled			= state;
	txtMinAmp->Enabled		= state;
	txtMaxAmp->Enabled		= state;
	lblTThresh->Enabled		= state;
	lblThresh->Enabled		= state;
	trkThresh->Enabled		= state;
	chkTagAmp->Enabled		= state;
	chkReset->Enabled		= state;

	//Save current settings if preset was "Custom"
	if (!state && grpPreset->Tag == 2)
	{
		udsp.chn	   	= cboChn->ItemIndex ? 2 : 1;
		udsp.mix		= (cboChn->ItemIndex == 2) ? MIX_FLOAT : MIX_INT;
		udsp.inter		= cboInter->ItemIndex;
		udsp.stereo		= trkStereo->Position;
		udsp.echo	   	= trkEcho->Position;
		udsp.lowPass   	= chkLow->Checked;
		udsp.surround  	= chkSurround->Checked;
		udsp.reverse   	= chkReverse->Checked;
		udsp.oldADPCM  	= chkADPCM->Checked;
		udsp.noEcho		= chkNoEcho->Checked;

		umix.aar	   	= (s32)grpAAR->ItemIndex;
		umix.amp	   	= trkAmp->Tag;
		umix.minAmp		= txtMinAmp->Tag;
		umix.maxAmp		= txtMaxAmp->Tag;
		umix.threshold	= trkThresh->Tag;
		umix.tagAmp		= chkTagAmp->Checked;
		umix.reset		= chkReset->Checked;
	}

	cboRate->Items->Clear();

	if (grpPreset->ItemIndex < 2)
	{
		cboRate->Tag = 32000;
		cboRate->Items->Append("32000");
		cboRate->Items->Append("44100");
		cboRate->Items->Append("48000");
		cboRate->Items->Append("88200");
		cboRate->Items->Append("96000");
		cboRate->Items->Append("176400");
		cboRate->Items->Append("192000");
		cboRateExit(NULL);
	}
	else
	{
		cboRate->Tag = 8000;
		cboRate->Items->Append("8000");
		cboRate->Items->Append("11025");
		cboRate->Items->Append("16000");
		cboRate->Items->Append("22050");
		cboRate->Items->Append("32000");
		cboRate->Items->Append("44100");
		cboRate->Items->Append("48000");
		cboRate->Items->Append("88200");
		cboRate->Items->Append("96000");
		cboRate->Items->Append("176400");
		cboRate->Items->Append("192000");
	}

	//Set controls -----------------------------
	switch(grpPreset->ItemIndex)
	{
	case(0):
		cboChn->ItemIndex		= 2;
		cboInter->ItemIndex		= 2;
		chkLow->Checked			= false;
		chkSurround->Checked	= false;
		chkReverse->Checked		= false;
		chkADPCM->Checked		= false;
		chkNoEcho->Checked		= false;
		trkStereo->Position		= 50;
		trkEcho->Position		= -50;

		grpAAR->ItemIndex		= 2;
		trkAmp->SelStart		= 0;
		trkAmp->SelEnd			= 180;
		trkAmp->Position		= 60;
		trkAmp->Tag				= 131072;
		txtMinAmp->Text			= "0.0";
		txtMaxAmp->Text			= "18.0";
		txtMinAmp->Tag			= 65536;
		txtMaxAmp->Tag			= 524288;
		trkThresh->Position		= 2;
		trkThresh->Tag			= 33148;
		chkTagAmp->Checked		= true;
		chkReset->Checked		= true;

		grpPreset->ItemIndex	= 0;
		grpPreset->Tag			= 0;
		break;

	case(1):
		cboChn->ItemIndex		= 1;
		cboInter->ItemIndex		= 3;
		chkLow->Checked			= true;
		chkSurround->Checked	= false;
		chkReverse->Checked		= false;
		chkADPCM->Checked		= false;
		chkNoEcho->Checked		= false;
		trkStereo->Position		= 50;
		trkEcho->Position		= 100;

		grpAAR->ItemIndex		= 0;
		trkAmp->SelStart		= 0;
		trkAmp->SelEnd			= 180;
		trkAmp->Position		= 0;
		trkAmp->Tag				= 65536;
		txtMinAmp->Text			= "0.0";
		txtMaxAmp->Text			= "18.0";
		txtMinAmp->Tag			= 65536;
		txtMaxAmp->Tag			= 524288;
		trkThresh->Position		= 0;
		trkThresh->Tag			= 32768;
		chkTagAmp->Checked		= false;
		chkReset->Checked		= true;

		grpPreset->ItemIndex	= 1;
		grpPreset->Tag			= 1;
		break;

	case(2):
		//Set DSP emulation options ------------
		cboChn->ItemIndex		= (udsp.mix == MIX_FLOAT) ? 2 : BitScanF(udsp.chn);
		cboInter->ItemIndex		= udsp.inter;
		chkLow->Checked			= udsp.lowPass;
		chkSurround->Checked	= udsp.surround;
		chkReverse->Checked		= udsp.reverse;
		chkADPCM->Checked		= udsp.oldADPCM;
		chkNoEcho->Checked		= udsp.noEcho;
		trkStereo->Position		= udsp.stereo;
		trkEcho->Position		= udsp.echo;

		cboChnChange(cboChn);					//Update certain fields affected by settings

		//Set mixing options -------------------
		grpAAR->ItemIndex		= umix.aar & AAR_TYPE;
		trkAmp->SelStart		= F2I(YLog2(60.0, (s32)umix.minAmp / 65536.0));
		trkAmp->SelEnd			= F2I(YLog2(60.0, (s32)umix.maxAmp / 65536.0));
		trkAmp->Position		= F2I(YLog2(60.0, (s32)umix.amp / 65536.0));
		trkAmp->Tag				= umix.amp;
		txtMinAmp->Text			= FloatToStrF(trkAmp->SelStart / 10.0, ffFixed, 4, 1);
		txtMaxAmp->Text			= FloatToStrF(trkAmp->SelEnd / 10.0, ffFixed, 4, 1);
		txtMinAmp->Tag			= umix.minAmp;
		txtMaxAmp->Tag			= umix.maxAmp;
		trkThresh->Position		= F2I(YLog2(120.0, (s32)umix.threshold / 32768.0));
		trkThresh->Tag			= umix.threshold;
		chkTagAmp->Checked		= umix.tagAmp;
		chkReset->Checked		= umix.reset;

		grpPreset->Tag			= 2;
		break;
	}

	cboChnChange(sender);
}


//**************************************************************************************************
// Disable chkLow if rate < 32kHz, otherwise enable it if normal stereo is selected in the custom
// preset

void __fastcall TfrmConfig::cboRateChange(TObject *sender)
{
	s32	i;


	i = cboRate->Text.ToIntDef(0);

	if (i < 32000)
	{
		chkLow->Checked = false;
		chkLow->Enabled = false;
	}
	else
	if (cboChn->ItemIndex == 1 && grpPreset->ItemIndex == 2)
	{
		chkLow->Enabled = true;
	}

	ItemChange(sender);
}

void __fastcall TfrmConfig::cboRateExit(TObject *sender)
{
	s32	i;


	i = cboRate->Text.ToIntDef(48000);
	_Clamp(i, cboRate->Tag, 192000);
	cboRate->Text = i;

	if (i < 32000)
	{
		chkLow->Checked = false;
		chkLow->Enabled = false;
	}
	else
	if (cboChn->ItemIndex == 1 && grpPreset->ItemIndex == 2)
	{
		chkLow->Enabled = true;
	}
}


//**************************************************************************************************
// Disable chkLow if bits == 8, otherwise enable it if normal stereo is selected in the custom
// preset

void __fastcall TfrmConfig::cboBitsChange(TObject *sender)
{
	if ((s32)cboBits->Items->Objects[cboBits->ItemIndex] == 8)
	{
		chkLow->Checked = false;
		chkLow->Enabled = false;
	}
	else
	if (cboChn->ItemIndex == 1 && grpPreset->ItemIndex == 2)
	{
		chkLow->Enabled = true;
	}

	ItemChange(sender);
}


//**************************************************************************************************
void __fastcall TfrmConfig::cboChnChange(TObject *sender)
{
	s32	bits;


	bits = (s32)cboBits->Items->Objects[cboBits->ItemIndex];

	if (cboChn->ItemIndex == 0)					//If output is monaural, disable stereo options
	{
		if (bits > 16) bits = 16;

		cboBits->Items->Clear();
		cboBits->Items->AddObject("8-bit", (TObject*)8);
		cboBits->Items->AddObject("16-bit", (TObject*)16);

		cboBits->ItemIndex = cboBits->Items->IndexOfObject((TObject*)bits);

		chkLow->Checked = false;
		chkLow->Enabled = false;
		chkSurround->Checked = false;
		chkSurround->Enabled = false;
		chkReverse->Checked = false;
		chkReverse->Enabled = false;
		trkStereo->Enabled = false;
		lblTStereo->Enabled = false;
		lblStereo->Enabled = false;
		trkEcho->Enabled = false;
		lblTEcho->Enabled = false;
		lblEcho->Enabled = false;
	}
	else
	{
		cboBits->Items->Clear();
		if (cboChn->ItemIndex == 2 || grpPreset->ItemIndex < 2)
		{
			if (bits == 8) bits = 16;
		}
		else
		{
			cboBits->Items->AddObject("8-bit", (TObject*)8);
		}
		cboBits->Items->AddObject("16-bit", (TObject*)16);
		cboBits->Items->AddObject("24-bit", (TObject*)24);
		cboBits->Items->AddObject("32-bit", (TObject*)32);

		cboBits->ItemIndex = cboBits->Items->IndexOfObject((TObject*)bits);

		if (grpPreset->ItemIndex < 2) return;

		if (bits == 8 || cboChn->ItemIndex == 2)
		{
			chkLow->Checked = false;
			chkLow->Enabled = false;
		}
		else
			chkLow->Enabled = true;
		chkSurround->Enabled = true;
		chkReverse->Enabled = true;
		trkStereo->Enabled = true;
		lblTStereo->Enabled = true;
		lblStereo->Enabled = true;
		trkEcho->Enabled = true;
		lblTEcho->Enabled = true;
		lblEcho->Enabled = true;
	}

	ItemChange(sender);
}


//**************************************************************************************************
void __fastcall TfrmConfig::trkStereoChange(TObject *sender)
{
	lblStereo->Caption = trkStereo->Position;
	ItemChange(sender);
}


//**************************************************************************************************
void __fastcall TfrmConfig::trkEchoChange(TObject *sender)
{
	AnsiString	str;
	s32			i;


	i = trkEcho->Position;
	if (i < 0) str = "-";
	i = 100 - abs(i);
	str += i;
	str += "%";
	lblEcho->Caption = str;

	ItemChange(sender);
}


//**************************************************************************************************
void __fastcall TfrmConfig::txtEndExit(TObject *sender)
{
	u32	time;


	time = Str2Ticks(txtEnd->Text.c_str());

	if (time)
		txtEnd->Text = FloatToStrF((f64)time/XID6_TICKSSEC,ffFixed,5,2);
	else
		txtEnd->Text = "";
}


//**************************************************************************************************
void __fastcall TfrmConfig::txtLoopXExit(TObject *sender)
{
	if (!txtLoopX->Text.ToIntDef(0))
		txtLoopX->Text = "";
}


//**************************************************************************************************
// Disable the threshold slider if AAR is turned off

void __fastcall TfrmConfig::grpAARClick(TObject *sender)
{
	b8	state;


	if (grpPreset->ItemIndex < 2) return;
	
	state = grpAAR->ItemIndex != 0;
	lblTThresh->Enabled = state;
	lblThresh->Enabled = state;
	trkThresh->Enabled = state;

	ItemChange(sender);
}


//**************************************************************************************************
// Limits the level to a value between the min and max values specified by the user

void __fastcall TfrmConfig::trkAmpChange(TObject *sender)
{
	s32	i = trkAmp->Position;


	if (i < trkAmp->SelStart)
	{
		i = trkAmp->SelStart;
		trkAmp->Position = i;
	}
	else
	if (i > trkAmp->SelEnd)
	{
		i = trkAmp->SelEnd;
		trkAmp->Position = i;
	}

	lblAmp->Caption = FormatFloat("0.0'dB'", trkAmp->Position / 10.0);
	trkAmp->Tag = F2I(pow(2.0, trkAmp->Position / 60.0) * 65536.0);

	ItemChange(sender);
}


//**************************************************************************************************
void __fastcall TfrmConfig::trkThreshChange(TObject *sender)
{
	lblThresh->Caption = FormatFloat("0.00'dB'", (f64)trkThresh->Position / 20.0);
	trkThresh->Tag = F2I(pow(2.0, trkThresh->Position / 120.0) * 32768.0);

	ItemChange(sender);
}


//**************************************************************************************************
void __fastcall TfrmConfig::txtMinAmpExit(TObject *sender)
{
	f64	min,max;
	s32	i;


	try
	{
		min = txtMinAmp->Text.ToDouble();
		max = txtMaxAmp->Text.ToDouble();
	}
	catch(...)
	{
		min = 0.0f;
	}

	_Clamp(min, -6.0f, max);

	i = min * 10.0f;
	trkAmp->SelStart = i;
	txtMinAmp->Text = FloatToStrF(min, ffFixed, 4, 1);
	txtMinAmp->Tag = F2I(pow(2.0, txtMinAmp->Text.ToDouble() / 6.0) * 65536.0);

	if (trkAmp->Position < i) trkAmp->Position = i;
}


//**************************************************************************************************
void __fastcall TfrmConfig::txtMaxAmpExit(TObject *sender)
{
	f64	min,max;
	s32	i;


	try
	{
		min = txtMinAmp->Text.ToDouble();
		max = txtMaxAmp->Text.ToDouble();
	}
	catch(...)
	{
		max = 18.0f;
	}

	_Clamp(max, min, 18.0f);

	i = max * 10.0f;
	trkAmp->SelEnd = i;
	txtMaxAmp->Text = FloatToStrF(max, ffFixed, 4, 1);
	txtMaxAmp->Tag = F2I(pow(2.0, txtMaxAmp->Text.ToDouble() / 6.0) * 65536.0 + 0.5);

	if (trkAmp->Position > i) trkAmp->Position = i;
}


//**************************************************************************************************
void __fastcall TfrmConfig::txtSongMinExit(TObject *sender)
{
	s32	i;


	i = txtSongMin->Text.ToIntDef(0);

	if (i < 0) i = 0;
	else
	if (i > 99) i = 99;

	txtSongMin->Text = i;
}


//**************************************************************************************************
// Check Time
//
// Verifies the time entered is in 00.00 form

void __fastcall TfrmConfig::CheckTime(TObject *sender)
{
	TEdit	*txt = static_cast<TEdit*>(sender);

	txt->Text = FloatToStrF((f64)Str2Ticks(txt->Text.c_str()) / XID6_TICKSSEC, ffFixed, 5, 2);
}


//**************************************************************************************************
// Save Configuration
//
// Gets values of all the controls and updates the settings

void __fastcall TfrmConfig::SaveCfg()
{
	Settings	newcfg;


	switch(grpPreset->ItemIndex)
	{
	case(0): newcfg.preset = -1; break;
	case(1): newcfg.preset = -2; break;
	case(2): newcfg.preset = 0; break;
	}

	if (!newcfg.preset)
	{
		newcfg.udsp.chn			= cboChn->ItemIndex ? 2 : 1;
		newcfg.udsp.mix			= (cboChn->ItemIndex == 2) ? MIX_FLOAT : MIX_INT;
		newcfg.udsp.inter		= cboInter->ItemIndex;
		newcfg.udsp.stereo		= trkStereo->Position;
		newcfg.udsp.echo	   	= trkEcho->Position;
		newcfg.udsp.lowPass		= chkLow->Checked;
		newcfg.udsp.surround   	= chkSurround->Checked;
		newcfg.udsp.reverse		= chkReverse->Checked;
		newcfg.udsp.oldADPCM   	= chkADPCM->Checked;
		newcfg.udsp.noEcho		= chkNoEcho->Checked;

		newcfg.umix.aar			= (s32)grpAAR->ItemIndex;
		newcfg.umix.amp			= trkAmp->Tag;
		newcfg.umix.minAmp		= txtMinAmp->Tag;
		newcfg.umix.maxAmp		= txtMaxAmp->Tag;
		newcfg.umix.threshold	= trkThresh->Tag;
		newcfg.umix.tagAmp		= chkTagAmp->Checked;
		newcfg.umix.reset		= chkReset->Checked;
	}
	else
	{
		memcpy(&newcfg.udsp, &udsp, sizeof(udsp));
		memcpy(&newcfg.umix, &umix, sizeof(umix));
	}

	newcfg.udsp.rate	   	= cboRate->Text.ToInt();
	newcfg.udsp.bits	   	= (s32)cboBits->Items->Objects[cboBits->ItemIndex];

	newcfg.waVol			= chkWAVol->Checked;

	newcfg.time.song  		= (txtSongMin->Text.ToInt() * XID6_TICKSMIN) +
							  Str2Ticks(txtSongSec->Text.c_str());
	newcfg.time.fade  		= Str2Ticks(txtFade->Text.c_str());
	newcfg.time.silence		= Str2Ticks(txtSilence->Text.c_str());
	newcfg.time.autoEnd		= Str2Ticks(txtEnd->Text.c_str());
	newcfg.time.loopx		= txtLoopX->Text.ToIntDef(0);
	newcfg.time.defBin		= chkBinTag->Checked;
	newcfg.time.useTimer	= chkTimer->Checked;
	newcfg.time.fastSeek	= chkFastSeek->Checked;

	strcpy(newcfg.titleFmt, txtTitle->Text.c_str());

	if (cboLang->ItemIndex == -1)
	{
		newcfg.language		= -1;
	}
	else
	{
		newcfg.language		= (u32)cboLang->Items->Objects[cboLang->ItemIndex];
		searched = 0;							//Loading LRM erases language list
	}

	strcpy(newcfg.fileExt, txtExt->Text.c_str());

	newcfg.tricks			= chkTricks->Checked;
	newcfg.ctrl				= chkCtrl->Checked;

	SaveConfig(newcfg);
}


//**************************************************************************************************
// Open a web browser

void __fastcall TfrmConfig::lblSMLinkClick(TObject *sender)
{
	ShellExecute(Application->Handle, NULL, "http://www.snesmusic.org", NULL, NULL, 0);
}

void __fastcall TfrmConfig::lblA2LinkClick(TObject *sender)
{
	ShellExecute(Application->Handle, NULL, "http://www.alpha-ii.com", NULL, NULL, 0);
}


//**************************************************************************************************
void __fastcall TfrmConfig::btnExtClick(TObject *Sender)
{
	txtExt->Text = "SPC;SP*;RSN;ZST;ZS*";
}


//**************************************************************************************************
void __fastcall TfrmConfig::btnResetClick(TObject *sender)
{
	ResetConfig();
	pgcCfg->ActivePageIndex = 0;
	FormShow(NULL);
	btnApply->Enabled = true;
}
