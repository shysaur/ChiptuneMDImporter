/***************************************************************************************************
* Control Dialog                                                                                   *
*                                                      Copyright (C)2001-2003 Alpha-II Productions *
***************************************************************************************************/

#include	<vcl.h>
#include	"SNESAmp.h"
#pragma hdrstop
#include	<math.h>
#include	"Control.h"


//**************************************************************************************************
#pragma package(smart_init)
#pragma resource "*.dfm"

TfrmCtrl *frmCtrl;


//**************************************************************************************************
__fastcall TfrmCtrl::TfrmCtrl(TComponent* owner) : TForm(owner)
{
}


//**************************************************************************************************
void __fastcall TfrmCtrl::Reinit()
{
	dockTest.hWindow	= Handle;

	//Init popup menu items --------------------
	itmADPCM->Checked	= cfg.dsp.oldADPCM;		//Check "Old ADPCM" according to config setting
	itmTimer->Checked	= cfg.time.useTimer;	//Enable "Use timer" according to config setting

	//Initialize default control values --------
	trkAmp->SelEnd		= 180 - F2I(YLog2(60.0, cfg.mix.minAmp / 65536.0));	//Set AAR range on amp
	trkAmp->SelStart	= 180 - F2I(YLog2(60.0, cfg.mix.maxAmp / 65536.0));	// slider
	trkPitch->Position	= cfg.pitch / 683; 		//Convert short to -48<=x<=48 (32768/48=683)
	trkSpeed->Position	= cfg.speed / 683;
	trkStereo->Position	= 100 - cfg.dsp.stereo;
	trkStereo->Enabled	= (cfg.dsp.chn == 2);
	lblStereo->Enabled	= (cfg.dsp.chn == 2);
	trkEcho->Position	= -cfg.dsp.echo;
	trkEcho->Enabled	= (cfg.dsp.chn == 2);
	lblEcho->Enabled	= (cfg.dsp.chn == 2);

	SetTimer(1,cfg.time.useTimer);				//Set external timer flag to itmTimer's state

	HelpFile = Application->HelpFile;

	chk1->HelpContext		= Ctrl_Mute;
	chk2->HelpContext		= Ctrl_Mute;
	chk3->HelpContext		= Ctrl_Mute;
	chk4->HelpContext		= Ctrl_Mute;
	chk5->HelpContext		= Ctrl_Mute;
	chk6->HelpContext		= Ctrl_Mute;
	chk7->HelpContext		= Ctrl_Mute;
	chk8->HelpContext		= Ctrl_Mute;
	trkAmp->HelpContext		= Ctrl_Amp;
	trkPitch->HelpContext	= Ctrl_Pitch;
	trkSpeed->HelpContext	= Ctrl_Speed;
	trkStereo->HelpContext	= Ctrl_Stereo;
	trkEcho->HelpContext	= Ctrl_EFBCT;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Form Events

//**************************************************************************************************
// Create Window Style Parameters
//
// Overrides the TForm function so I can create a dialog box without a title bar

void __fastcall TfrmCtrl::CreateParams(TCreateParams &params)
{
	TForm::CreateParams(params);
	params.Style &= ~WS_CAPTION;
}


//**************************************************************************************************
void __fastcall TfrmCtrl::FormCreate(TObject *sender)
{
	//Init pointer array to checkboxes ---------
	chkMute[0] = chk1;
	chkMute[1] = chk2;
	chkMute[2] = chk3;
	chkMute[3] = chk4;
	chkMute[4] = chk5;
	chkMute[5] = chk6;
	chkMute[6] = chk7;
	chkMute[7] = chk8;
	setMute = 1;								//Changes to mute checkboxes are by user
	curMute = 0;								//No voices are muted
	usrMute = 0;

	SetRelPos(this, cfg.ctrlTop, cfg.ctrlLeft);

	speed = 0x10000;
	minimized = 0;
	showable = 0;
	hFocus = GetMainWnd();

	dockTest.hRoot = inMod.hMainWindow;
	dockTest.thread = GetWindowThreadProcessId(inMod.hMainWindow, NULL);

	itmClose->Visible = !cfg.tricks;

	Reinit();
}


//**************************************************************************************************
// Since the control dialog is modeless, we have to handle the keypresses manually

void __fastcall TfrmCtrl::FormKeyDown(TObject *sender, WORD &key, TShiftState shift)
{
	switch(key)
	{
	case(VK_LEFT):
		FindNextControl(ActiveControl,false,true,false)->SetFocus();
		key = 0;
		break;

	case(VK_RIGHT):
		FindNextControl(ActiveControl,true,true,false)->SetFocus();
		key = 0;
		break;

	case(VK_TAB):
		SetForegroundWindow(hFocus);
		key = 0;
		break;

	case(VK_F1):
		itmHelpClick(sender);
		key = 0;
		break;

	case('0'):
		itmUnmuteClick(sender);
		key = 0;
		break;

	default:
		if (key>='1' && key<='8')
		{
			if (shift.Contains(ssAlt))
			{
				popHelp->PopupComponent = chkMute[--key & 0xF];
				itmSoloClick(sender);
			}
			else
				chkMute[--key & 0xF]->Checked = !chkMute[key & 0xF]->Checked;
			key = 0;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Popup Menu Events

//**************************************************************************************************
void __fastcall TfrmCtrl::itmCfgClick(TObject *sender)
{
	inMod.Config(inMod.hMainWindow);
}


//**************************************************************************************************
void __fastcall TfrmCtrl::itmAboutClick(TObject *sender)
{
	inMod.About(inMod.hMainWindow);
}


//**************************************************************************************************
void __fastcall TfrmCtrl::itmADPCMClick(TObject *sender)
{
	u32	opts,state;

	
	state = !itmADPCM->Checked;

	opts = (cfg.dsp.lowPass ? DSP_ANALOG:0) |
		   (state ? DSP_OLDSMP:0) |
		   (cfg.dsp.surround ? DSP_SURND:0) |
		   (cfg.dsp.reverse ? DSP_REVERSE:0) |
		   (cfg.dsp.noEcho ? DSP_NOECHO:0);

	itmADPCM->Checked = state;

	sapu.SetAPUOpt(-1,-1,-1,-1,-1,opts);
}


//**************************************************************************************************
void __fastcall TfrmCtrl::itmTimerClick(TObject *sender)
{
	itmTimer->Checked = !itmTimer->Checked;
	SetTimer(1,itmTimer->Checked);
}


//**************************************************************************************************
void __fastcall TfrmCtrl::itmDisableClick(TObject *sender)
{
	if (Visible) GetRelPos(this, cfg.ctrlTop, cfg.ctrlLeft);

	Application->MessageBox(LoadStr(STR_WARN_CTRL).c_str(), "SNESAmp plug-in",
							MB_OK|MB_ICONINFORMATION);

	cfg.ctrl = 0;
	WritePrivateProfileString("SNESAmp", "Control", NULL, cfg.iniFile);

	delete this;
	frmCtrl = NULL;
}


//**************************************************************************************************
void __fastcall TfrmCtrl::itmCloseClick(TObject *sender)
{
	Hide();
}


//**************************************************************************************************
void __fastcall TfrmCtrl::itmHelpClick(TObject *sender)
{
	WinHelp(Handle,hlpFile,HELP_FINDER,0);
}


//**************************************************************************************************
void __fastcall TfrmCtrl::itmSoloClick(TObject *sender)
{
	TCheckBox	*pMute = (TCheckBox*)popHelp->PopupComponent;
	u32			i;

	for (i=0;i<8;i++)
		chkMute[i]->Checked = true;
	pMute->Checked = false;
}


//**************************************************************************************************
void __fastcall TfrmCtrl::itmUnmuteClick(TObject *sender)
{
	u32	i;

	for (i=0;i<8;i++)
		chkMute[i]->Checked = false;
}


//**************************************************************************************************
void __fastcall TfrmCtrl::itmWhatClick(TObject *sender)
{
	TWinControl *pWinControl = static_cast<TWinControl*>(popHelp->PopupComponent);

	Application->HelpCommand(HELP_CONTEXTPOPUP,pWinControl->HelpContext);
}


//**************************************************************************************************
// Display the "Solo" and "Unmute" menu items if the user right clicked on a mute checkbox

void __fastcall TfrmCtrl::popHelpPopup(TObject *sender)
{
	itmSolo->Visible = popHelp->PopupComponent->ClassNameIs("TCheckBox");
	itmUnmute->Visible = popHelp->PopupComponent->ClassNameIs("TCheckBox");
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Banner (Horizontal Titlebar) Events

//**************************************************************************************************
// If the mouse button is pressed, the user may be trying to move the dialog

void __fastcall TfrmCtrl::imgBannerMouseDown(TObject *sender, TMouseButton button,
												TShiftState shift, int x, int y)
{
	moving = 1;									//Control dialog is moving

	if (cfg.tricks)
	{
		PrepareMove(&dockTest, Mouse->CursorPos.x, Mouse->CursorPos.y);
	}
	else
	{
		adjust.x = Left - Mouse->CursorPos.x;
		adjust.y = Top - Mouse->CursorPos.y;
	}
}


//**************************************************************************************************
// If the mouse button is released, then the user has stopped moving the dialog

void __fastcall TfrmCtrl::imgBannerMouseUp(TObject *sender, TMouseButton button,
											  TShiftState shift, int x, int y)
{
	moving = 0;									//Control dialog is no longer moving
}


//**************************************************************************************************
// If 'moving' is true, then we need to move the dialog when the mouse cursor moves

void __fastcall TfrmCtrl::imgBannerMouseMove(TObject *sender, TShiftState shift, int x, int y)
{
	RECT	rect;


	if (!moving) return;						//If Control dialog isn't moving, don't test

	if (cfg.tricks)
	{
		MoveDialog(&rect, &dockTest, Mouse->CursorPos.x, Mouse->CursorPos.y);
		Left = rect.left;
		Top = rect.top;
	}
	else
	{
		Left = Mouse->CursorPos.x + adjust.x;
		Top = Mouse->CursorPos.y + adjust.y;
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Change Events

//**************************************************************************************************
void __fastcall TfrmCtrl::chkMuteChange(TObject *sender)
{
	TCheckBox	*box = static_cast<TCheckBox*>(sender);


	if (box->State == cbChecked)
	{
		MuteVoice(box->Tag,1);					//Update mixing flags in DSP emulator
		usrMute |= setMute << box->Tag;			//Update internal flags
		curMute |= 1 << box->Tag;
	}
	else
	{
		MuteVoice(box->Tag,0);
		usrMute &= ~(setMute << box->Tag);
		curMute &= ~(1 << box->Tag);
	}
}


//**************************************************************************************************
void __fastcall TfrmCtrl::trkAmpChange(TObject *sender)
{
	s32	dB;


	dB = 180 - trkAmp->Position;
	lblAmp->Caption = FormatFloat("0.0'dB'", dB/10.0);

	if (setAmp) ::SetAmp(F2I(65536.0 * pow(2.0, dB/60.0)));
}


//**************************************************************************************************
// The pitch is 2 raised to the power of the track bar's position (-48 to 48) divided by 48, which
// gives us a logarithmic value from 0.5 to 2.0, or -/+ 1 octave.

void __fastcall TfrmCtrl::trkPitchChange(TObject *sender)
{
	//Dividing by 4 gives us the number of half steps difference (12 half-steps per octave)
	lblPitch->Caption = FloatToStrF((f64)(-trkPitch->Position) / 4.0, ffFixed, 5, 2);

	sapu.SetDSPPitch(F2I(Pow2(-trkPitch->Position / 48.0) * 32000.0));
}


//**************************************************************************************************
// The speed is calculated in much the same way as the pitch, but the logarithmic value is converted
// directly into a 16.16 fixed point number.

void __fastcall TfrmCtrl::trkSpeedChange(TObject *sender)
{
	speed = Pow2(-trkSpeed->Position / 48.0) * 65536.0;

	lblSpeed->Caption = FormatFloat("0.00x", Pow2(-trkSpeed->Position / 48.0));

	sapu.SetAPUSmpClk(speed);
}


//**************************************************************************************************
// Internally, the emulator follows a square root curve for determining stereo separation, but on
// this side we just need to find a linear value.  Stereo separation is a -1.16 fixed point value.

void __fastcall TfrmCtrl::trkStereoChange(TObject *sender)
{
	s32	i;


	i = 100 - trkStereo->Position;
	lblStereo->Caption = i;
	sapu.SetDSPStereo((i << 16) / 100);
}


//**************************************************************************************************
void __fastcall TfrmCtrl::trkEchoChange(TObject *sender)
{
	AnsiString	str;
	s32			i;


	//Incase you're wondering at this point, vertical trackbars are inherited from the vertical
	//scrollbars used on the sides of windows.  When scrolling down a window, it makes sense to
	//start at 0 and count up as you move down.  This is usually opposite of what people want when
	//using trackbars, so I have to negate the position before I can do anything useful with it.

	i = -trkEcho->Position;						//Vertical trackbars have their values inverted
	if (i < 0) str = "-";						//If the value's negative, add a '-' to the display
	str += 100 - abs(i);						//This value is always positive (hence the above)
	str += "%";
	lblEcho->Caption = str;
	sapu.SetDSPEFBCT((i << 15) / 100);
}


//**************************************************************************************************
// Set Alpha Value
//
// Sets the transparancy level of the dialog to that of the main window

void __fastcall TfrmCtrl::SetAlpha()
{
	u8		alpha;
	DWORD	flags;
	u32		style;
	s32		i;


	if (!cfg.tricks || !GetWindowAlpha(&dockTest)) return;

	flags = 0;
	style = GetWindowLong(Handle, GWL_EXSTYLE);
	if (style & WS_EX_LAYERED) pGetLayeredWindowAttributes(GetMainWnd(), NULL, &alpha, &flags);
	if (!(flags & LWA_ALPHA)) alpha = 255;

	for (i=0; i<dockTest.windows; i++)
		if (alpha > dockTest.alpha[i] && dockTest.alpha[i] > 0) alpha = dockTest.alpha[i];

	if (alpha != 255)
	{
		if (!(style & WS_EX_LAYERED)) SetWindowLong(Handle, GWL_EXSTYLE, style | WS_EX_LAYERED);

		pSetLayeredWindowAttributes(Handle, 0, alpha, LWA_ALPHA);
	}
	else
	{
		if (style & WS_EX_LAYERED) SetWindowLong(Handle, GWL_EXSTYLE, style & ~WS_EX_LAYERED);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Exported Functions

//**************************************************************************************************
// Show Form without Focus

void __fastcall TfrmCtrl::ShowN()
{
	HWND			hwnd;
	WINDOWPLACEMENT	wndpl;
	b8				vis;


	showable = 1;
	if (minimized) return;

	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(GetMainWnd(), &wndpl);

	if (wndpl.showCmd != SW_SHOWMINIMIZED)		//Showing the dialog while Winamp is minimized
	{											// causes problems
		hwnd = GetForegroundWindow();
		if (!Visible) SetAlpha();				//Set the alpha level
		Show();
		if (hwnd) SetForegroundWindow(hwnd);
	}
}


//**************************************************************************************************
// Set Muted Voices
//
// Changes made by the user to the mute check boxes get stored in the variable mute.  When a song is
// loaded that has the mute flags set, those flags are used.  But when a song is loaded that doesn't
// have any flags set, then the user defined flags in mute get used.
//
// Setting 'setMute' to 0 keeps 'curMute' and 'usrMute' from getting modified when the checkboxes
// are set.

void __fastcall TfrmCtrl::SetMute(u32 m)
{
	s32	i;


	if (!m) m = usrMute;						//Are we to restore the user states?

	if (m == curMute) return;					//If mute flags haven't changed, don't do anything

	setMute = 0;								//Mute state changes will have no effect on user states
	for (i=0;i<8;i++)							//Set each checkbox
	{
		chkMute[i]->Checked = m & 1;
		m >>= 1;
	}
	setMute = 1;								//Proceeding changes will affect the user mute states
}


//**************************************************************************************************
// Set Amplification Level
//
// The slider seems to have a bug when using the SelStart and SelEnd parameters with a negative Min
// and a positive Max.  So the range is set to 0-240 and an adjustment has to be made.

void __fastcall TfrmCtrl::SetAmp(u32 amp)
{
	setAmp = 0;												//Disable calling SetAmp in Winamp.cpp
	trkAmp->Position = 180 - F2I(YLog2(60.0, (s32)amp / 65536.0));	//Set slider position
	setAmp = 1;
}
