/***************************************************************************************************
* ID666 Tag Editor                                                                                 *
*                                                      Copyright (C)2001-2003 Alpha-II Productions *
***************************************************************************************************/

#include	<vcl.h>
#include	"SNESAmp.h"
#pragma	hdrstop
#include	<math.h>
#include	"Tag.h"
#include	"Control.h"


//**************************************************************************************************
#pragma	package(smart_init)
#pragma	resource "*.dfm"

//Table of possible noise clock values
static const s8	nClk[32][12]={
			"0Hz",
			"15.625Hz",	"20.83Hz",	"25Hz",
			"31.25Hz",	"41.67Hz",	"50Hz",
			"62.5Hz",	"83.33Hz",	"100Hz",
			"125Hz",	"166.67Hz",	"200Hz",
			"250Hz",	"333.33Hz",	"400Hz",
			"500Hz",	"666.67Hz",	"800Hz",
			"1kHz",		"1.33kHz",	"1.6kHz",
			"2kHz",		"2.67kHz",	"3.2kHz",
			"4kHz",		"5.33kHz",	"6.4kHz",
			"8kHz",		"10.67kHz",
			"16kHz",
			"32kHz"};

TfrmTag *frmTag;


ID6Type __fastcall LoadFile(s8 *fn, ID666 &id6, s8 *pData = NULL, b8 *pRAR = NULL, b8 xpl = 0);


/***************************************************************************************************
* Form Functions                                                                                   *
***************************************************************************************************/

//**************************************************************************************************
__fastcall TfrmTag::TfrmTag(TComponent* owner):TForm(owner)
{
}


//**************************************************************************************************
void __fastcall TfrmTag::Reinit()
{
	s8	str[32],*s;
	u32	i;


	dockTest.hWindow = Handle;

	HelpFile = Application->HelpFile;			//Assign help file to form

	for (i=0;i<MAX_KEYS;i++)					//Search button captions for hotkey letters
	{
		strcpy(str,hotKeys[i].btn->Caption.c_str());
		s = strchr(str,'&');
		hotKeys[i].key = s ? *(s+1)|0x20 : 0;
	}

	btnCopy->HelpContext  	= ID6_Copy;
	btnPaste->HelpContext	= ID6_Paste;
	btnClear->HelpContext	= ID6_Clear;

	txtFile->HelpContext	= ID6_File;
	chkUpdate->HelpContext	= ID6_Update;
	chkTimer->HelpContext	= ID6_Timer;

	chkBin->HelpContext		= ID6_Binary;
	txtSong->HelpContext	= ID6_Song;
	txtGame->HelpContext	= ID6_Game;
	txtArtist->HelpContext	= ID6_Artist;
	txtDumper->HelpContext	= ID6_Dumper;
	txtDate->HelpContext	= ID6_Date;
	cboEmu->HelpContext		= ID6_Emu;
	mmoCmnt->HelpContext	= ID6_Comment;
	txtSongMin->HelpContext	= ID6_SongLen;
	txtSongSec->HelpContext	= ID6_SongLen;
	txtFadeOld->HelpContext	= ID6_FadeLen;
	btnSong->HelpContext	= ID6_SongBtn;
	btnFadeOld->HelpContext	= ID6_FOldBtn;

	chkExt->HelpContext		= ID6_Ext;
	txtOST->HelpContext		= ID6_OST;
	txtDisc->HelpContext	= ID6_Disc;
	txtTrack->HelpContext	= ID6_Track;
	txtPub->HelpContext		= ID6_Publish;
	txtCopy->HelpContext	= ID6_Year;
	txtAmp->HelpContext		= ID6_Amp;
	chk1->HelpContext		= ID6_Mute;
	chk2->HelpContext		= ID6_Mute;
	chk3->HelpContext		= ID6_Mute;
	chk4->HelpContext		= ID6_Mute;
	chk5->HelpContext		= ID6_Mute;
	chk6->HelpContext		= ID6_Mute;
	chk7->HelpContext		= ID6_Mute;
	chk8->HelpContext		= ID6_Mute;
	txtIntroMin->HelpContext	= ID6_IntrLen;
	txtIntroSec->HelpContext	= ID6_IntrLen;
	txtLoopMin->HelpContext	= ID6_LoopLen;
	txtLoopSec->HelpContext	= ID6_LoopLen;
	txtLoopX->HelpContext	= ID6_LoopX;
	txtEndMin->HelpContext	= ID6_EndLen;
	txtEndSec->HelpContext	= ID6_EndLen;
	txtFadeSec->HelpContext	= ID6_FadeLen;
	btnIntro->HelpContext	= ID6_IntrBtn;
	btnLoop->HelpContext	= ID6_LoopBtn;
	btnEnd->HelpContext		= ID6_EndBtn;
	btnFade->HelpContext	= ID6_FadeBtn;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Form Events

//**************************************************************************************************
void __fastcall TfrmTag::FormCreate(TObject *sender)
{
	//Get pointers to each mute checkbox -------
	chkMute[0] = chk1;
	chkMute[1] = chk2;
	chkMute[2] = chk3;
	chkMute[3] = chk4;
	chkMute[4] = chk5;
	chkMute[5] = chk6;
	chkMute[6] = chk7;
	chkMute[7] = chk8;

	//Get pointers to all buttons --------------
	hotKeys[0].btn = btnApply;
	hotKeys[1].btn = btnCopy;
	hotKeys[2].btn = btnPaste;
	hotKeys[3].btn = btnClear;
	hotKeys[4].btn = btnSong;
	hotKeys[5].btn = btnFadeOld;
	hotKeys[6].btn = btnIntro;
	hotKeys[7].btn = btnLoop;
	hotKeys[8].btn = btnEnd;
	hotKeys[9].btn = btnFade;

	//Get pointers to all time fields ----------
	timeField[0].min = txtIntroMin;
	timeField[0].sec = txtIntroSec;
	timeField[1].min = txtLoopMin;
	timeField[1].sec = txtLoopSec;
	timeField[2].min = txtEndMin;
	timeField[2].sec = txtEndSec;
	timeField[3].min = NULL;
	timeField[3].sec = txtFadeSec;
	timeField[4].min = txtSongMin;
	timeField[4].sec = txtSongSec;
	timeField[5].min = NULL;
	timeField[5].sec = txtFadeOld;

	//Set timeField index in time fields -------
	txtIntroMin->Tag = 0;
	txtIntroSec->Tag = 0;
	txtLoopMin->Tag = 1;
	txtLoopSec->Tag = 1;
	txtEndMin->Tag = 2;
	txtEndSec->Tag = 2;
	txtFadeSec->Tag = 3;
	txtSongMin->Tag = 4;
	txtSongSec->Tag = 4;
	txtFadeOld->Tag = 5;

	chkTimer->Enabled = cfg.time.useTimer;

	EnableTime(NULL);

	//Initialize keypress events (BCB bug?) ----
	OnKeyPress = FormKeyPress;

	//Initialize focus rectangle for mute ------
	mutefocus.left = lbl1->Left-3;
	mutefocus.right = lbl8->Left+lbl8->Width+3;
	mutefocus.top = chk1->Top-3;
	mutefocus.bottom = chk1->Top+chk1->Height+3;

	SetRelPos(this, cfg.tagTop, cfg.tagLeft);

	dockTest.hRoot = inMod.hMainWindow;
	dockTest.thread = GetWindowThreadProcessId(inMod.hMainWindow, NULL);

	btnCancel->Tag = btnCancel->Left;

	chkUpdate->Enabled = cfg.tricks;

	Reinit();									//Set help file and extract hotkeys
}


//**************************************************************************************************
void __fastcall TfrmTag::FormShow(TObject *sender)
{
	SetTimer(2,!chkTimer->Checked);				//Disable the timer, if the user has the box checked
}


//**************************************************************************************************
void __fastcall TfrmTag::FormHide(TObject *sender)
{
	SetTimer(2,true);							//Re-enable the timer, incase it was disabled
}


//**************************************************************************************************
// The focus box around the mute checkboxes is manually drawn, so when the form's paint function is
// called, we may need to draw the focus.

void __fastcall TfrmTag::Paint()
{
	TCustomForm::Paint();
	if (pgcTag->ActivePageIndex==2 && chkMuteFocused) chkMuteFocus(true);
}


//*** The following events are used as a hack to fix the VCL's inability to handle the tab key when
//*** a form is modeless and doesn't have a VCL parent.

//**************************************************************************************************
void __fastcall TfrmTag::FormKeyDown(TObject *sender, WORD &key, TShiftState shift)
{
	u32		i;

	//By default, block the keypress to keep Windows from generating a "ding".
	keyHandled = 0;

	//Check for a hotkey -----------------------
	if (shift.Contains(ssAlt))					//Is the alt key being pressed?
		for (i=0;i<MAX_KEYS;i++)				//Search for a letter matching the current key
			if (hotKeys[i].key==key && hotKeys[i].btn->Enabled)
			{
				key = 0;
				hotKeys[i].btn->Click();        //Simulate a button press
				return;
			}

	//Check other key presses ------------------
	switch(key)
	{
	case(VK_TAB):
		//If Ctrl-Tab is pressed in the comment box, add a tab character.
		//Otherwise, treat the tab as normal.
		if (!(shift.Contains(ssCtrl) && ActiveControl->ClassNameIs("TMemo")))
		{
			SelectNext(ActiveControl, !shift.Contains(ssShift),true);
			key = 0;
		}
		return;

	case(VK_F5):
		//Reload the current ID666 tag
		if (!readonly)
		{
			curType = LoadFile(id6.file, id6);
			SetForm(NULL);
		}
		key = 0;
		return;

	case(VK_RETURN):
		if (btnOK->Visible) btnOKClick(NULL);
		return;

	case(VK_ESCAPE):
		Close();
		return;

	default:
		//If the component is any kind of text editor, let the form handle the keypress.
		if (ActiveControl->ClassNameIs("TEdit") ||
			ActiveControl->ClassNameIs("TMemo") ||
			ActiveControl->ClassNameIs("TComboBox"))
		{
			keyHandled = -1;
			return;
		}
	}

	//Update mute checkboxes -------------------
	//If the mute has the focus, update the checkboxes based on the number pressed.
	if (chkMuteFocused && !readonly)
	{
		if (key == '0')							//Reset all checkboxes
		{
			for(i=0;i<8;i++)
				chkMute[i]->Checked = false;
			key = 0;
			return;
		}
		else
		if (key>='1' && key<='8')				//Toggle a checkbox
		{
			chkMute[key-0x31]->Checked ^= true;
			key = 0;
			return;
		}
	}

	keyHandled = -1;							//If we got this far, let the form handle the key
}


//**************************************************************************************************
void __fastcall TfrmTag::FormKeyPress(TObject *sender, char &key)
{
	key &= keyHandled;
}


//**************************************************************************************************
// Display the context help when a user selects "What's this?" from the popup menu

void __fastcall TfrmTag::itmWhatClick(TObject *sender)
{
	TWinControl *pWinControl;

	pWinControl = static_cast<TWinControl*>(popHelp->PopupComponent);
	Application->HelpCommand(HELP_CONTEXTPOPUP,pWinControl->HelpContext);
}


//**************************************************************************************************
void __fastcall TfrmTag::btnCopyClick(TObject *sender)
{
	strcpy(id6Copy.game, txtGame->Text.c_str());
	strcpy(id6Copy.artist, txtArtist->Text.c_str());
	strcpy(id6Copy.dumper, txtDumper->Text.c_str());
	id6Copy.emu = cboEmu->ItemIndex;
	strcpy(id6Copy.comment, mmoCmnt->Text.c_str());

	strcpy(id6Copy.ost, txtOST->Text.c_str());
	id6Copy.disc = txtDisc->Text.ToIntDef(0);
	strcpy(id6Copy.pub, txtPub->Text.c_str());
	id6Copy.copy = txtCopy->Text.ToIntDef(0);
}


//**************************************************************************************************
void __fastcall TfrmTag::btnPasteClick(TObject *sender)
{
	if (id6Copy.game[0])	txtGame->Text		= id6Copy.game;
	if (id6Copy.artist[0])	txtArtist->Text		= id6Copy.artist;
	if (id6Copy.dumper[0])	txtDumper->Text		= id6Copy.dumper;
	if (id6Copy.emu)		cboEmu->ItemIndex	= id6Copy.emu;
	if (id6Copy.comment[0]) mmoCmnt->Text		= id6Copy.comment;

	if (id6Copy.ost[0])		txtOST->Text		= id6Copy.ost;
	if (id6Copy.disc)		txtDisc->Text		= id6Copy.disc;
	if (id6Copy.pub[0])		txtPub->Text		= id6Copy.pub;
	if (id6Copy.copy)		txtCopy->Text		= id6Copy.copy;
}


//**************************************************************************************************
void __fastcall TfrmTag::btnClearClick(TObject *sender)
{
	u32	i;


	txtSong->Text = "";
	txtGame->Text = "";
	txtArtist->Text = "";
	txtDumper->Text = "";
	txtDate->Text = "";
	id6.datetxt[0] = 0;
	cboEmu->ItemIndex = 0;
	mmoCmnt->Text = "";
	txtSongMin->Text = "0";
	txtSongSec->Text = "0.00";
	txtFadeOld->Text = "0.00";

	txtOST->Text = "";
	txtDisc->Text = "";
	txtTrack->Text = "";
	txtPub->Text = "";
	txtCopy->Text = "";
	txtAmp->Text = "";
	for (i = 0;i<7;i++)
		chkMute[i]->Checked = false;
	chkExt->Checked = false;
}


//**************************************************************************************************
void __fastcall TfrmTag::btnOKClick(TObject *sender)
{
	if (btnApply->Enabled)
		btnApplyClick(sender);					//Save the tag
	Close();
}


//**************************************************************************************************
void __fastcall TfrmTag::btnCancelClick(TObject *sender)
{
	Close();
}


//**************************************************************************************************
void __fastcall TfrmTag::btnApplyClick(TObject *sender)
{
	s8		str[256],*s;
	u32		i;


	//Store dialog fields in id6 object --------
	strcpy(id6.song, txtSong->Text.c_str());
	strcpy(id6.game, txtGame->Text.c_str());
	strcpy(id6.artist, txtArtist->Text.c_str());
	strcpy(id6.dumper, txtDumper->Text.c_str());
	id6.date = curDate;
	id6.emu = cboEmu->ItemIndex;
	strcpy(id6.comment, mmoCmnt->Text.c_str());

	if (chkExt->Checked)
	{
		strcpy(id6.ost, txtOST->Text.c_str());
		id6.disc = txtDisc->Text.ToIntDef(0);
		strcpy(str,txtTrack->Text.c_str());
		id6.track = atoi(str) << 8;
		s = str + strlen(str) - 1;
		if (*s<'0' || *s>'9')					//Extract letter from track, if there is one
			id6.track |= *s;

		strcpy(id6.pub,txtPub->Text.c_str());
		id6.copy = txtCopy->Text.ToIntDef(0);

		id6.mute = 0;							//Set mute flags
		for (i=0;i<8;i++)
		{
			if (chkMute[i]->Checked)
				id6.mute |= 1 << i;
		}
		try
		{
			id6.amp = F2I(pow(2.0, txtAmp->Text.ToDouble()) * 65536.0);
		}
		catch(...)
		{
			id6.amp = 0;
		}

		id6.intro = txtIntroMin->Text.ToInt() * XID6_TICKSMIN + Str2Ticks(txtIntroSec->Text.c_str());
		id6.loop = txtLoopMin->Text.ToInt() * XID6_TICKSMIN + Str2Ticks(txtLoopSec->Text.c_str());
		id6.loopx = txtLoopX->Text.ToInt();
		id6.end = txtEndMin->Text.ToInt() * XID6_TICKSMIN + Str2Ticks(txtEndSec->Text.c_str());
		id6.fade = Str2Ticks(txtFadeSec->Text.c_str());
	}
	else
	{
		id6.song[32] = 0;
		id6.artist[32] = 0;
		id6.dumper[16] = 0;
		id6.comment[32] = 0;

		id6.ost[0] = 0;
		id6.pub[0] = 0;
		id6.copy = 0;
		id6.disc = 0;
		id6.track = 0;
		id6.mute = 0;
		id6.amp = 0;

		id6.intro = txtSongMin->Text.ToInt() * XID6_TICKSMIN + Str2Ticks(txtSongSec->Text.c_str());
		id6.loop = 0;
		id6.loopx = 1;
		id6.end = 0;
		id6.fade = Str2Ticks(txtFadeOld->Text.c_str());
	}

	if (id6.SaveTag(chkBin->Checked, chkExt->Checked))
	{
		SetID666(id6);
		btnApply->Enabled = false;
	}
	else
		Application->MessageBox(LoadStr(STR_ERR_SAVE).c_str(), "SNESAmp plug-in",
								MB_OK|MB_ICONEXCLAMATION);
}


//**************************************************************************************************
// Called when the current page changes.  Used to en/disable the copy, paste, and clear buttons
// depending on the current tab selected.

void __fastcall TfrmTag::pgcTagChange(TObject *sender)
{
	if (pgcTag->ActivePageIndex>0)
	{
		btnCopy->Enabled = true;
		btnPaste->Enabled = true;
		btnClear->Enabled = true;
	}
	else
	{
		btnCopy->Enabled = false;
		btnPaste->Enabled = false;
		btnClear->Enabled = false;
	}

	chkMuteFocused = false;						//The mute checkboxes are no longer selected
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Information Page

//**************************************************************************************************
// En/disable the song timer based on the checkbox

void __fastcall TfrmTag::chkTimerClick(TObject *sender)
{
	SetTimer(2,!chkTimer->Checked);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// ID666 Page

//**************************************************************************************************
b8 __fastcall TfrmTag::StrDate(s8 *str, A2Date &date)
{
	TDateTime	*tDate;
	s32			y,m,d;


	if (!date) return 0;

	date.GetDate(y,m,d);
	tDate = new TDateTime(y,m,d);
	strcpy(str, tDate->DateString().c_str());
	delete tDate;

	return 1;
}


//**************************************************************************************************
// If the tag contains a valid date, it is formatted to a style used by the editor.  Otherwise the
// date field is cleared.

void __fastcall TfrmTag::txtDateEnter(TObject *sender)
{
	s32	d,m,y;

	dateChanged = false;
	oldDate = txtDate->Text;
	if ((s32)curDate)
	{
		curDate.GetDate(y,m,d);
		txtDate->Text.printf("%i/%i/%i", m, d, y % 100);
	}
}


//**************************************************************************************************
// Filters out invalid characters for entering dates

void __fastcall TfrmTag::txtDateKeyPress(TObject *sender, char &key)
{
	if ((key>='0' && key<='9') || key=='.' || key=='/' || key==VK_BACK || key==VK_DELETE)
		dateChanged = true;						//Flag date field as having changed
	else
		key = 0;								//Otherwise invalidate keypress
}


//**************************************************************************************************
// If the user entered anything, verify the date is valid and format the new date according to the
// region settings.

void __fastcall TfrmTag::txtDateExit(TObject *sender)
{
	A2Date	*date;
	s32	  	d,m,y;
	s8	  	str[32],*s;


	d = txtDate->Text.Length();

	if (dateChanged || d != oldDate.Length())	//Was any text entered?
	{
		if (d)
		{
			strcpy(str,txtDate->Text.c_str());
			txtDate->Text = "";					//Erase date field incase date is invalid

			s = strchr(str,'/');				//Search for US style date separator
			if (s)
			{
				m = atoi(str);
				d = atoi(++s);
				s = strchr(s,'/');
				if (!s) return;
				y = atoi(s + 1);
			}
			else
			{
				s = strchr(str,'.');			//Search for European separator
				if (!s) return;
				d = atoi(str);
				m = atoi(++s);
				s = strchr(s,'.');
				if (!s) return;
				y = atoi(s + 1);
			}

			date = new A2Date;
			if (id6.FixDate(*date,y,m,d))		//Verify date
			{
				curDate = *date;
				StrDate(str,*date);
				txtDate->Text = str;
			}
			delete date;
		}
		else
		{
			txtDate->Text = "";
			curDate.Invalidate();
			id6.datetxt[0] = 0;
		}
	}
	else
		txtDate->Text = oldDate;				//If nothing changed, restore old text
}


//**************************************************************************************************
// Calculate the time in the song field when the user clicks on the Song button.  This field is
// different in that the maximum value can change depending on the format of the tag.

void __fastcall TfrmTag::btnSongClick(TObject *sender)
{
	s32	time,max;


	time = GetTime();
	max = chkBin->Checked ? (99 * XID6_TICKSMIN + 59 * XID6_TICKSSEC)
						  : (15 * XID6_TICKSMIN + 59 * XID6_TICKSSEC);

	if (time>max) time = max;

	txtSongMin->Text = time/XID6_TICKSMIN;
	txtSongSec->Text = AnsiString((time % XID6_TICKSMIN) / XID6_TICKSSEC) + ".00";
	timeField[4].time = time;
}


//**************************************************************************************************
// Special handling is required for the song seconds field, because fractions of a second aren't
// allowed.

void __fastcall TfrmTag::txtSongSecExit(TObject *sender)
{
	u32	t = Str2Ticks(txtSongSec->Text.c_str()) / XID6_TICKSSEC;


	txtSongSec->Text = FloatToStrF((f64)t,ffFixed,5,2);
	timeField[4].time = t * XID6_TICKSSEC;
	timeField[4].time += txtSongMin->Text.ToInt() * XID6_TICKSMIN;
}


//**************************************************************************************************
void __fastcall TfrmTag::btnFadeOldClick(TObject *sender)
{
	s32	time;


	time = GetTime() - timeField[4].time;
	if (time < 0) time = 0;
	else
	if (time > XID6_TICKSMIN - (XID6_TICKSSEC / 100)) time = XID6_TICKSMIN - (XID6_TICKSSEC / 100);

	txtFadeOld->Text = FloatToStrF((f64)time / XID6_TICKSSEC,ffFixed,5,2);
	timeField[5].time = time;
}


//**************************************************************************************************
// If the user is using the extended time fields, then the time stored in the old ID666 tag can
// change if the total extended time is > 15:59.

void __fastcall TfrmTag::chkBinClick(TObject *sender)
{
	if (chkExt->Checked) TotalTime();			//Update time displayed in disabled ID666 fields
	ItemChange(NULL);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Extended Page

//**************************************************************************************************
// Verifies the disc is a number

void __fastcall TfrmTag::txtDiscExit(TObject *sender)
{
	u32	t;

	t = txtDisc->Text.ToIntDef(0);
	if (t<1 || t>9) txtDisc->Text = "";
}


//**************************************************************************************************
// Verifies the track contains a valid value

void __fastcall TfrmTag::txtTrackExit(TObject *sender)
{
	s8 	str[4];
	u32	l,t;
	s8 	a=0;


	strcpy(str, txtTrack->Text.c_str());
	l = strlen(str);

	if (l--)									//Does string have a length?
	{
		if (str[l]<'0' || str[l]>'9')			//Is last character non-numeric?
		{
			a = str[l];							//Get last character
			a |= 0x20;							//Change to lowercase
			if (a<'a' || a>'z') a = 0;			//If not alpha, invalidate
			str[l] = 0;							//Remove last character from string
		}
	}
	t = atoi(str);								//Convert string to int

	if (t<=0 || t>99)							//If track is invalid, erase field
		txtTrack->Text = "";
	else
	{
		txtTrack->Text.printf(str, "%i%c", t, a);
	}
}


//**************************************************************************************************
// Verifies the copyright year is valid

void __fastcall TfrmTag::txtCopyExit(TObject *sender)
{
	s32	year;


	year = txtCopy->Text.ToIntDef(-2000);
	if (year < 100)
	{
		year += 1900;
		if (year < 1989) year += 100;
	}
	if (year < 1989) year = 0;

	if (year)
		txtCopy->Text = year;
	else
		txtCopy->Text = "";
}


//**************************************************************************************************
void __fastcall TfrmTag::txtAmpExit(TObject *sender)
{
	f32	f;


	try
	{
		f = txtAmp->Text.ToDouble();
	}
	catch(...)
	{
		txtAmp->Text = "";
		return;
	}

	_Clamp(f, -6.0, 18.0);

	txtAmp->Text = FloatToStrF(f, ffFixed, 4, 1);
}


//**************************************************************************************************
void __fastcall TfrmTag::chkMuteEnter(TObject *sender)
{
	if (chkMuteFocused) return;					//If a focus box doesn't exist, draw one

	chkMuteFocus(true);
	chkMuteFocused = true;
}


//**************************************************************************************************
void __fastcall TfrmTag::chkMuteExit(TObject *sender)
{
	chkMuteFocus(false);
	chkMuteFocused = false;
}


//**************************************************************************************************
// Mute Focus Box
// Desc:
//    Draws a focus box around the mute checkboxes
//
// In:
//    true, if the box should be draw
//    false, if the box should be erased

void __fastcall TfrmTag::chkMuteFocus(b8 draw)
{
	HDC		dc;
	HBRUSH	wincolor;


	dc = GetDC(tabExt->Handle);					//Get a device context to draw on
	wincolor = GetSysColorBrush(COLOR_BTNFACE);	//Get the system default color

	FrameRect(dc,&mutefocus,wincolor);			//Draw a grey box
	if (draw) DrawFocusRect(dc,&mutefocus);		//Then put a focus box on top of it
	ReleaseDC(tabExt->Handle,dc);				//Release the DC for another program
}


//**************************************************************************************************
void __fastcall TfrmTag::txtLoopXExit(TObject *sender)
{
	s32	x;


	if (!txtLoopX->Modified) return;
	txtLoopX->Modified = false;

	x = txtLoopX->Text.ToIntDef(-1);
	if (x < 1)   								//If loop is not 1 to 9, default to 1
	{
		x = 1;
		txtLoopX->Text = x;
	}
	txtLoopX->Tag = x;							//Store binary value

	TotalTime();								//Recalculate total time
}


//**************************************************************************************************
void __fastcall TfrmTag::btnIntroClick(TObject *sender)
{
	s32	time;


	time = GetTime();
	if (time > XID6_MAXTICKS) time = XID6_MAXTICKS;
	timeField[0].time = time;

	txtIntroMin->Text = time / XID6_TICKSMIN;
	time %= XID6_TICKSMIN;
	txtIntroSec->Text = FloatToStrF((f64)time / XID6_TICKSSEC, ffFixed, 5, 2);

	TotalTime();
}


//**************************************************************************************************
void __fastcall TfrmTag::btnLoopClick(TObject *sender)
{
	s32	time;


	time = GetTime() - timeField[0].time;
	if (time < 0) time = 0;
	else
	if (time > XID6_MAXTICKS) time = XID6_MAXTICKS;
	timeField[1].time = time;

	txtLoopMin->Text = time / XID6_TICKSMIN;
	time %= XID6_TICKSMIN;
	txtLoopSec->Text = FloatToStrF((f64)time / XID6_TICKSSEC, ffFixed, 5, 2);

	TotalTime();
}


//**************************************************************************************************
void __fastcall TfrmTag::btnEndClick(TObject *sender)
{
	s32	time;


	time = GetTime() - timeField[0].time - timeField[1].time;
	if (time < 0) time = 0;
	else
	if (time > XID6_MAXTICKS) time = XID6_MAXTICKS;
	timeField[2].time = time;

	txtEndMin->Text = time / XID6_TICKSMIN;
	time %= XID6_TICKSMIN;
	txtEndSec->Text = FloatToStrF((f64)time / XID6_TICKSSEC, ffFixed, 5, 2);

	TotalTime();
}


//**************************************************************************************************
void __fastcall TfrmTag::btnFadeClick(TObject *sender)
{
	s32	time;


	time = GetTime() - timeField[0].time - timeField[1].time - timeField[2].time;
	if (time < 0) time = 0;
	else
	if (time > XID6_TICKSMIN - 100) time = XID6_TICKSMIN - 100;
	timeField[3].time = time;

	txtFadeSec->Text = FloatToStrF((f64)time / XID6_TICKSSEC, ffFixed, 5, 2);

	TotalTime();
}


//**************************************************************************************************
// A function that en/disables various components based on the state of the extended checkbox

void __fastcall TfrmTag::chkExtClick(TObject *sender)
{
	if (chkExt->Checked)
	{
		txtSong->MaxLength		= 255;
		txtGame->MaxLength		= 255;
		txtArtist->MaxLength	= 255;
		txtDumper->MaxLength	= 255;
		mmoCmnt->MaxLength		= 255;

		btnSong->Enabled		= false;
		btnFadeOld->Enabled		= false;
		txtSongMin->Enabled		= false;
		txtSongSec->Enabled		= false;
		txtFadeOld->Enabled		= false;

		lblOST->Enabled			= true;
		lblDisc->Enabled		= true;
		lblTrack->Enabled		= true;
		lblPub->Enabled			= true;
		lblCopy->Enabled		= true;
		lblAmp->Enabled			= true;
		lblMute->Enabled		= true;
		lbl1->Enabled			= true;
		lbl8->Enabled			= true;
		lblTotal->Enabled		= true;

		txtOST->Enabled			= true;
		txtDisc->Enabled		= true;
		txtTrack->Enabled		= true;
		txtPub->Enabled			= true;
		txtCopy->Enabled		= true;
		txtAmp->Enabled			= true;
		chk1->Enabled			= true;
		chk2->Enabled			= true;
		chk3->Enabled			= true;
		chk4->Enabled			= true;
		chk5->Enabled			= true;
		chk6->Enabled			= true;
		chk7->Enabled			= true;
		chk8->Enabled			= true;
		txtIntroMin->Enabled	= true;
		txtIntroSec->Enabled	= true;
		txtLoopMin->Enabled		= true;
		txtLoopSec->Enabled		= true;
		txtLoopX->Enabled		= true;
		txtEndMin->Enabled		= true;
		txtEndSec->Enabled		= true;
		txtFadeSec->Enabled		= true;

		btnIntro->Enabled		= curSong && !readonly;
		btnLoop->Enabled		= curSong && !readonly;
		btnEnd->Enabled			= curSong && !readonly;
		btnFade->Enabled		= curSong && !readonly;
	}
	else
	{
		txtSong->MaxLength		= 32;
		txtGame->MaxLength		= 32;
		txtArtist->MaxLength	= 32;
		txtDumper->MaxLength	= 16;
		mmoCmnt->MaxLength		= 32;

		txtSongMin->Enabled		= true;
		txtSongSec->Enabled		= true;
		txtFadeOld->Enabled		= true;
		btnSong->Enabled		= curSong && !readonly;
		btnFadeOld->Enabled		= curSong && !readonly;

		lblOST->Enabled			= false;
		lblDisc->Enabled		= false;
		lblTrack->Enabled		= false;
		lblPub->Enabled			= false;
		lblCopy->Enabled		= false;
		lblAmp->Enabled			= false;
		lblMute->Enabled		= false;
		lbl1->Enabled			= false;
		lbl8->Enabled			= false;
		lblTotal->Enabled		= false;

		txtOST->Enabled			= false;
		txtDisc->Enabled		= false;
		txtTrack->Enabled		= false;
		txtPub->Enabled			= false;
		txtCopy->Enabled		= false;
		txtAmp->Enabled			= false;
		chk1->Enabled			= false;
		chk2->Enabled			= false;
		chk3->Enabled			= false;
		chk4->Enabled			= false;
		chk5->Enabled			= false;
		chk6->Enabled			= false;
		chk7->Enabled			= false;
		chk8->Enabled			= false;
		txtIntroMin->Enabled	= false;
		txtIntroSec->Enabled	= false;
		txtLoopMin->Enabled		= false;
		txtLoopSec->Enabled		= false;
		txtLoopX->Enabled		= false;
		txtEndMin->Enabled		= false;
		txtEndSec->Enabled		= false;
		txtFadeSec->Enabled		= false;
		btnIntro->Enabled		= false;
		btnLoop->Enabled		= false;
		btnEnd->Enabled			= false;
		btnFade->Enabled		= false;
	}

	if (sender) ItemChange(sender);
}


void __fastcall TfrmTag::SetReadOnly()
{
	txtSong->ReadOnly		= readonly;
	txtGame->ReadOnly		= readonly;
	txtArtist->ReadOnly		= readonly;
	txtDumper->ReadOnly		= readonly;
	txtDate->ReadOnly		= readonly;
	cboEmu->Enabled			= !readonly;
	mmoCmnt->ReadOnly		= readonly;
	txtSongMin->ReadOnly	= readonly;
	txtSongSec->ReadOnly	= readonly;
	txtFadeOld->ReadOnly	= readonly;
	chkBin->Enabled			= !readonly;

	txtOST->ReadOnly		= readonly;
	txtDisc->ReadOnly		= readonly;
	txtTrack->ReadOnly		= readonly;
	txtPub->ReadOnly		= readonly;
	txtCopy->ReadOnly		= readonly;
	txtAmp->ReadOnly		= readonly;
	chk1->Enabled			= !readonly;
	chk2->Enabled			= !readonly;
	chk3->Enabled			= !readonly;
	chk4->Enabled			= !readonly;
	chk5->Enabled			= !readonly;
	chk6->Enabled			= !readonly;
	chk7->Enabled			= !readonly;
	chk8->Enabled			= !readonly;
	txtIntroMin->ReadOnly	= readonly;
	txtIntroSec->ReadOnly	= readonly;
	txtLoopMin->ReadOnly	= readonly;
	txtLoopSec->ReadOnly	= readonly;
	txtLoopX->ReadOnly		= readonly;
	txtEndMin->ReadOnly		= readonly;
	txtEndSec->ReadOnly		= readonly;
	txtFadeSec->ReadOnly	= readonly;
	chkExt->Enabled			= !readonly;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Other Internal Functions

//**************************************************************************************************
// Enables the Apply and OK buttons when an item in the tag is changed

void __fastcall TfrmTag::ItemChange(TObject *sender)
{
	btnApply->Enabled = true;
	btnOK->Enabled = true;
}


//**************************************************************************************************
// Verifies minutes entered is between 0 and 99

void __fastcall TfrmTag::CheckMinutes(TObject *sender)
{
	TEdit	*txt;
	s32		i,t;


	txt = static_cast<TEdit*>(sender);
	if (!txt->Modified) return;					//If the text hasn't been modified, don't bother

	txt->Modified = false;						//Reset the modified flag
	i = txt->Tag;								//Tag contains the index into timeField
	t = timeField[i].min->Text.ToIntDef(-1);

	if (i != txtEndMin->Tag && t < 0)			//Make sure value isn't negative
	{
		t = 0;
		timeField[i].min->Text = t;
	}

	if (chkExt->Checked)						//If ext is checked, then this in an ext time field
	{
		t *= XID6_TICKSMIN;
		if (t >= 0)
			t += Str2Ticks(timeField[i].sec->Text.c_str());
		else
			t -= Str2Ticks(timeField[i].sec->Text.c_str());

		timeField[i].time = t;
		TotalTime();							//Total up time
	}
	else
	{
		if (!chkBin->Checked && t>15)			//Clip time at 15 minutes, if the tag's not binary
		{
			t = 15;
			timeField[i].min->Text = t;
		}
		timeField[i].time = t * XID6_TICKSMIN + Str2Ticks(timeField[i].sec->Text.c_str());
	}
}


//**************************************************************************************************
// Verifies seconds entered is between 0 and 59.99

void __fastcall TfrmTag::CheckSeconds(TObject *sender)
{
	TEdit	*txt;
	s32		i,time;


	txt = static_cast<TEdit*>(sender);
	if (!txt->Modified) return;

	txt->Modified = false;
	i = txt->Tag;
	time = Str2Ticks(timeField[i].sec->Text.c_str());

	timeField[i].sec->Text = FloatToStrF((f64)time / XID6_TICKSSEC, ffFixed, 5, 2);
	timeField[i].time = time;
	if (timeField[i].min)						//Fade field doesn't have a minute counterpart
		timeField[i].time += timeField[i].min->Text.ToInt() * XID6_TICKSMIN;

	if (chkExt->Checked) TotalTime();
}


//**************************************************************************************************
void __fastcall TfrmTag::TotalTime()
{
	u32	t,s,max;
	s8	str[12];

	s = timeField[0].time +						//s = song length
		(timeField[1].time * txtLoopX->Tag) +
		timeField[2].time;
	t = s + timeField[3].time;					//t = total length

	//Update old song time ---------------------
	max = chkBin->Checked ? (99 * XID6_TICKSMIN + 59 * XID6_TICKSSEC)
						  : (15 * XID6_TICKSMIN + 59 * XID6_TICKSSEC);
	if (s > max)								//Try to find a time that will in the old song field
	{
		s = timeField[0].time + timeField[2].time;	//s = intro + end
		if (s+timeField[1].time < max)			//Find the number of loops that will fit
			s += timeField[1].time * ((max - s) / timeField[1].time);
		else
			s = max;							//If extended length won't fit, default to max
	}

	txtSongMin->Text = s / XID6_TICKSMIN;
	txtSongSec->Text = FloatToStrF((f64)((s % XID6_TICKSMIN) / XID6_TICKSSEC), ffFixed, 5, 2);
	txtFadeOld->Text = FloatToStrF((f64)timeField[3].time / XID6_TICKSSEC, ffFixed, 5, 2);
	timeField[4].time = s - (s % XID6_TICKSSEC);
	timeField[5].time = timeField[3].time;

	//Update total time on Extended tab --------
	wsprintf(str, "%i:%02i.%02i", t / XID6_TICKSMIN,
								 (t % XID6_TICKSMIN) / XID6_TICKSSEC,
								 (t % XID6_TICKSSEC) / (XID6_TICKSMS * 10));
	lblTotal->Caption = str;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Loading and Saving Tags

//**************************************************************************************************
// Populate left pane on Information page

void __fastcall TfrmTag::SetInfo(u8 *fRegs, DSPReg *pDSP)
{
	u32	i,j;


	//Set echo status --------------------------
	if (pDSP->flg & 0x20)						//Is echo enabled?
	{
		lblEcho->Caption = LoadStr(STR_DISABLED);
		lblDelay->Enabled = false;
		lblFB->Enabled = false;
		lblFIR->Enabled = false;
	}
	else
	{
		lblEcho->Caption = LoadStr(STR_ENABLED);
		lblDelay->Enabled = true;
		lblFB->Enabled = true;
		lblFIR->Enabled = true;
	}

	//Check for FIR filter ---------------------
	i = pDSP->fir[0].c - 0x7F;					//If the first tap is 127 and the rest of the taps
	for (j=1;j<8;j++)							// are 0, then the filter has no effect
		i |= pDSP->fir[j].c;

	//Set echo values --------------------------
	lblDelay->Caption = AnsiString(pDSP->edl << 4) + "ms";
	lblFB->Caption = AnsiString((pDSP->efb * 100) >> 7) + "%";
	lblFIR->Caption = LoadStr(i ? STR_ON : STR_OFF);
	i = pDSP->flg & 0x1F;
	lblNoise->Caption = i ? nClk[i] : LoadStr(STR_OFF).c_str();

	//Set timer status -------------------------
	lblT0->Enabled = fRegs[0x1] & 1;
	lblT1->Enabled = fRegs[0x1] & 2;
	lblT2->Enabled = fRegs[0x1] & 4;

	//Set timer values -------------------------
	i = fRegs[0xA];
	if (!i) i = 256;
	lblT0->Caption = AnsiString(8000 / i) + "Hz";
	i = fRegs[0xB];
	if (!i) i = 256;
	lblT1->Caption = AnsiString(8000 / i) + "Hz";
	i = fRegs[0xC];
	if (!i) i = 256;
	lblT2->Caption = AnsiString(64000 / i) + "Hz";
}


//**************************************************************************************************
// Fill in form with data from ID666 tag and file (pFile can be NULL)

void __fastcall TfrmTag::SetForm(s8 *pFile)
{
	s8		str[36];
	DWORD	l;
	s8		*s;


	txtFile->Text = id6.file;

	//Populate form -------------------------
	if (curType==1 || curType==2)
	{
		if (pFile)
		{
			memcpy(str, pFile, 33);
			str[33] = 0;
			lblHdr->Caption = str;

			SetInfo(&pFile[0x1F0], (DSPReg*)&pFile[0x10100]);
		}

		txtSong->Text = id6.song;
		txtGame->Text = id6.game;
		txtArtist->Text = id6.artist;
		txtDumper->Text = id6.dumper;
		if ((u32)id6.date)
		{
			StrDate(str,id6.date);
			txtDate->Text = str;
		}
		else
			txtDate->Text = id6.datetxt;
		curDate = id6.date;
		cboEmu->ItemIndex = id6.emu;
		mmoCmnt->Text = id6.comment;
		if (id6.HasTime())
		{
			txtFadeOld->Text = FloatToStrF(id6.fade / XID6_TICKSSEC, ffFixed, 5, 2);

			txtIntroMin->Text	= id6.intro / XID6_TICKSMIN;
			txtIntroSec->Text	= FloatToStrF((f64)(id6.intro % XID6_TICKSMIN) / XID6_TICKSSEC, ffFixed, 5, 2);
			txtLoopMin->Text	= id6.loop / XID6_TICKSMIN;
			txtLoopSec->Text	= FloatToStrF((f64)(id6.loop % XID6_TICKSMIN) / XID6_TICKSSEC, ffFixed, 5, 2);
			if ((s32)id6.end < 0)
				txtEndMin->Text	= "-" + AnsiString(abs((s32)id6.end) / XID6_TICKSMIN);
			else
				txtEndMin->Text	= id6.end / XID6_TICKSMIN;
			txtEndSec->Text		= FloatToStrF((f64)(abs(id6.end) % XID6_TICKSMIN) / XID6_TICKSSEC, ffFixed, 5, 2);
			txtFadeSec->Text	= FloatToStrF((f64)id6.fade / XID6_TICKSSEC, ffFixed, 5, 2);
			txtLoopX->Text		= id6.loopx;

			timeField[0].time = id6.intro;
			timeField[1].time = id6.loop;
			timeField[2].time = id6.end;
			timeField[3].time = id6.fade;
			txtLoopX->Tag = id6.loopx;
			TotalTime();
		}
		else
		{
			txtSongMin->Text	= "0";
			txtSongSec->Text	= "0.00";
			txtFadeOld->Text	= "0.00";

			txtIntroMin->Text	= "0";
			txtIntroSec->Text	= "0.00";
			txtLoopMin->Text	= "0";
			txtLoopSec->Text	= "0.00";
			txtEndMin->Text		= "0";
			txtEndSec->Text		= "0.00";
			txtFadeSec->Text	= "0.00";
			txtLoopX->Text		= "1";

			for (l=0;l<6;l++)
				timeField[l].time = 0;
		}

		txtOST->Text = id6.ost;

		if (id6.disc)
			txtDisc->Text = id6.disc;
		else
			txtDisc->Text = "";

		if (id6.track)
		{
			txtTrack->Text = id6.track >> 8;
			if (id6.track & 0xFF)
				txtTrack->Text += AnsiString((s8)id6.track);
		}
		else
			txtTrack->Text = "";

		txtPub->Text = id6.pub;

		if (id6.copy)
			txtCopy->Text = id6.copy;
		else
			txtCopy->Text = "";

		if (id6.amp)
			txtAmp->Text = FloatToStrF(YLog2(6.0, (s32)id6.amp / 65536.0), ffFixed, 4, 1);
		else
			txtAmp->Text = "";

		for (l=0;l<8;l++)
			chkMute[l]->Checked = (id6.mute >> l) & 1;

		chkExt->Checked = id6.IsExt();
		chkBin->Checked = id6.IsBin();

		tabID6->TabVisible = true;
		tabExt->TabVisible = true;
	}
	else
	{
		if (curType==3)
		{
			if (pFile)
			{
				memcpy(str, pFile, 32);
				str[32] = 0;
				s=strchr(str,0x1A);
				if (s) *s = 0;
				lblHdr->Caption = str;


				SetInfo(&pFile[0x1F0], (DSPReg*)&pFile[0x10100]);
			}
		}
		else
		{
			lblHdr->Caption = "Unknown file";

			lblEcho->Caption = "";
			lblDelay->Enabled = true;
			lblFB->Enabled = true;
			lblFIR->Enabled = true;

			lblDelay->Caption = "";
			lblFB->Caption = "";
			lblFIR->Caption = "";
			lblNoise->Caption = "";

			lblT0->Enabled = true;
			lblT1->Enabled = true;
			lblT2->Enabled = true;

			lblT0->Caption = "";
			lblT1->Caption = "";
			lblT2->Caption = "";
		}

		pgcTag->ActivePageIndex = 0;
		tabID6->TabVisible = false;
		tabExt->TabVisible = false;
	}

	btnApply->Enabled = false;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Window Messages

//**************************************************************************************************
void __fastcall TfrmTag::WMEnterSizeMove(TMessage &msg)
{
	if (cfg.tricks) PrepareMove(&dockTest, Mouse->CursorPos.x, Mouse->CursorPos.y);
}


//**************************************************************************************************
void __fastcall TfrmTag::WMMoving(TMessage &msg)
{
	if (cfg.tricks) MoveDialog((RECT*)msg.LParam, &dockTest, Mouse->CursorPos.x, Mouse->CursorPos.y);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// External Functions

//**************************************************************************************************
void __fastcall TfrmTag::EditTag(s32 fType, ID666 *pID6, s8 *pFile, b8 force, b8 ro)
{
	b8	enable;


	//If the user didn't specifically request this load, only load the tag if no changes have
	// been made and the Auto update checkbox is checked.
	if (force || (!btnApply->Enabled && chkUpdate->Checked))
	{
		readonly = ro;
		enable = !ro;
		
		id6 = *pID6;
		curType = fType;
		SetForm(pFile);

		btnCopy->Visible	= enable;
		btnPaste->Visible	= enable;
		btnClear->Visible	= enable;
		btnOK->Visible		= enable;
		if (cfg.language == 0 || cfg.language == 1033)
			btnCancel->Caption	= enable ? "Cancel" : "Close";
		btnCancel->Left		= enable ? btnCancel->Tag : btnApply->Left;
		btnApply->Visible	= enable;

		//Disable controls based on file loaded
		chkExtClick(NULL);
		SetReadOnly();
	}
}


//**************************************************************************************************
void __fastcall TfrmTag::EnableTime(const s8* fn)
{
	b8	enable;


	curSong = 0;
	if (fn) curSong = id6.SameFile(fn);

	if (curSong)
	{

		lblPlaying->Visible = false;

		enable = !chkExt->Checked && !readonly;
		btnSong->Enabled = enable;
		btnFadeOld->Enabled = enable;

		enable = chkExt->Checked && !readonly;
		btnIntro->Enabled = enable;
		btnLoop->Enabled = enable;
		btnEnd->Enabled = enable;
		btnFade->Enabled = enable;
	}
	else
	{
		lblPlaying->Visible = true;			//"not playing"
		btnSong->Enabled = false;			//Disable all timer buttons
		btnFadeOld->Enabled = false;
		btnIntro->Enabled = false;
		btnLoop->Enabled = false;
		btnEnd->Enabled = false;
		btnFade->Enabled = false;
	}
}
