/***************************************************************************************************
* Form: ID666 Tag Editor                                                                           *
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

#ifndef TagH
#define TagH

#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Menus.hpp>
#include <ExtCtrls.hpp>

#define	MAX_KEYS	10

typedef struct HotKey							//Structure used to keep track of keyboard shortcuts
{
	WORD	key;								//Character that summons button
	TButton	*btn;								//Pointer to button's OnClick event
} HotKey;


typedef struct TimeField						//Structure used to manage different time values
{
	TEdit	*min,*sec;							//Pointers to text edit fields
	u32		time;								//Time, in ticks, contained in text fields
} TimeField;


//**************************************************************************************************
class TfrmTag:public TForm
{
__published:	// IDE-managed Components
	TPopupMenu	*popHelp;
	TMenuItem	*itmWhat;
	
	TButton		*btnCopy;
	TButton		*btnPaste;
	TButton		*btnClear;
	TButton		*btnOK;
	TButton		*btnCancel;
	TButton		*btnApply;
	TPageControl	*pgcTag;
	TTabSheet	*tabInfo;
	TTabSheet	*tabID6;
	TTabSheet	*tabExt;

	TEdit		*txtFile;
	TBevel		*bvlInfo;
	TLabel		*lblHdr;
	TLabel		*lblTEcho;
	TLabel		*lblTDelay;
	TLabel		*lblTFB;
	TLabel		*lblTFIR;
	TLabel		*lblTNoise;
	TLabel		*lblTT0;
	TLabel		*lblTT1;
	TLabel		*lblTT2;
	TLabel		*lblEcho;
	TLabel		*lblDelay;
	TLabel		*lblFB;
	TLabel		*lblFIR;
	TLabel		*lblNoise;
	TLabel		*lblT0;
	TLabel		*lblT1;
	TLabel		*lblT2;
	TLabel		*lblPlaying;
	TLabel		*lblOutput;
	TLabel		*lblTTime;
	TLabel		*lblTNum;
	TLabel		*lblTSmp;
	TLabel		*lblTByte;
	TLabel		*lblTime;
	TLabel		*lblNum;
	TLabel		*lblSmp;
	TLabel		*lblByte;
	TCheckBox	*chkUpdate;
	TCheckBox	*chkTimer;

	TButton		*btnSong;
	TButton		*btnFadeOld;
	TLabel		*lblSong;
	TLabel		*lblGame;
	TLabel		*lblArtist;
	TLabel		*lblDumper;
	TLabel		*lblOn;
	TLabel		*lblWith;
	TLabel		*lblComments;
	TLabel		*lblSongSep;
	TEdit		*txtSong;
	TEdit		*txtGame;
	TEdit		*txtArtist;
	TEdit		*txtDumper;
	TEdit		*txtDate;
	TComboBox	*cboEmu;
	TMemo		*mmoCmnt;
	TEdit		*txtSongMin;
	TEdit		*txtSongSec;
	TEdit		*txtFadeOld;
	TCheckBox	*chkBin;

	TButton		*btnIntro;
	TButton		*btnLoop;
	TButton		*btnEnd;
	TButton		*btnFade;
	TLabel		*lblOST;
	TLabel		*lblDisc;
	TLabel		*lblTrack;
	TLabel		*lblPub;
	TLabel		*lblCopy;
	TLabel		*lblAmp;
	TLabel		*lblMute;
	TLabel		*lbl1;
	TLabel		*lbl8;
	TLabel		*lblIntroSep;
	TLabel		*lblLoopSep;
	TLabel		*lblLoopX;
	TLabel		*lblEndSep;
	TLabel		*lblEqu;
	TEdit		*txtOST;
	TEdit		*txtDisc;
	TEdit		*txtTrack;
	TEdit		*txtPub;
	TEdit		*txtCopy;
	TEdit		*txtAmp;
	TCheckBox	*chk1;
	TCheckBox	*chk2;
	TCheckBox	*chk3;
	TCheckBox	*chk4;
	TCheckBox	*chk5;
	TCheckBox	*chk6;
	TCheckBox	*chk7;
	TCheckBox	*chk8;
	TEdit		*txtIntroMin;
	TEdit		*txtIntroSec;
	TEdit		*txtLoopMin;
	TEdit		*txtLoopSec;
	TEdit		*txtLoopX;
	TEdit		*txtEndMin;
	TEdit		*txtEndSec;
	TEdit		*txtFadeSec;
	TLabel		*lblTotal;
	TCheckBox	*chkExt;

	void __fastcall FormCreate(TObject *sender);
	void __fastcall FormKeyDown(TObject *sender, WORD &key, TShiftState shift);
	void __fastcall btnApplyClick(TObject *sender);
	void __fastcall btnOKClick(TObject *sender);
	void __fastcall btnCancelClick(TObject *sender);
	void __fastcall btnCopyClick(TObject *sender);
	void __fastcall btnPasteClick(TObject *sender);
	void __fastcall btnClearClick(TObject *sender);
	void __fastcall pgcTagChange(TObject *sender);
	void __fastcall txtDateEnter(TObject *sender);
	void __fastcall txtDateKeyPress(TObject *sender, char &key);
	void __fastcall txtDateExit(TObject *sender);
	void __fastcall CheckMinutes(TObject *sender);
	void __fastcall CheckSeconds(TObject *sender);
	void __fastcall txtDiscExit(TObject *sender);
	void __fastcall txtTrackExit(TObject *sender);
	void __fastcall txtCopyExit(TObject *sender);
	void __fastcall ItemChange(TObject *sender);
	void __fastcall chkTimerClick(TObject *sender);
	void __fastcall FormKeyPress(TObject *sender, char &key);
	void __fastcall itmWhatClick(TObject *sender);
	void __fastcall chkMuteEnter(TObject *sender);
	void __fastcall chkMuteExit(TObject *sender);
	void __fastcall chkExtClick(TObject *sender);
	void __fastcall txtLoopXExit(TObject *sender);
	void __fastcall txtSongSecExit(TObject *sender);
	void __fastcall chkBinClick(TObject *sender);
	void __fastcall txtAmpExit(TObject *sender);
	void __fastcall FormHide(TObject *sender);
	void __fastcall FormShow(TObject *sender);
	void __fastcall btnSongClick(TObject *sender);
	void __fastcall btnFadeOldClick(TObject *sender);
	void __fastcall btnIntroClick(TObject *sender);
	void __fastcall btnLoopClick(TObject *sender);
	void __fastcall btnEndClick(TObject *sender);
	void __fastcall btnFadeClick(TObject *sender);

private:
	HotKey		hotKeys[MAX_KEYS];				//Button shortcut keys
	TCheckBox*	chkMute[8];						//Pointers to each mute checkbox
	TimeField	timeField[6];					//Structure for each time field

	s32		  	keyHandled;						//Keypress was already handled

	ID666	  	id6,id6Copy;					//ID666 tags for current file and clipboard
	u32			curType;						//Type of current file
	b8		  	curSong;						//True, if current file is playing
	b8			readonly;						//Current file is read-only

	A2Date		curDate;						//Current date entered in date field
	AnsiString	oldDate;						//Actual text in date field
	b8		  	dateChanged;					//User changed the text in the date field

	b8		  	chkMuteFocused;					//Mute checkboxes have the focus
	RECT		mutefocus;

	DockTest	dockTest;

	DYNAMIC void __fastcall Paint(void);
	static b8	__fastcall StrDate(s8*,A2Date&);
	void		__fastcall chkMuteFocus(b8);
	void		__fastcall SetReadOnly();
	void		__fastcall TotalTime();
	void		__fastcall SetInfo(u8*, DSPReg*);
	void		__fastcall SetForm(s8*);

protected:
	void		__fastcall WMEnterSizeMove(TMessage &msg);
	void		__fastcall WMMoving(TMessage &msg);

#pragma	warn -8027								//Turn off warning produced by BEGIN_MESSAGE_MAP
BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_ENTERSIZEMOVE, TMessage, WMEnterSizeMove)
	MESSAGE_HANDLER(WM_MOVING, TMessage, WMMoving)
END_MESSAGE_MAP(TForm)
#pragma	warn .8027

public:
	s32			relTop,relLeft;					//Position relative to moving window


	__fastcall TfrmTag(TComponent* owner);

	//**********************************************************************************************
	// Reinitialize Form
	//
	// Initializes data affected by changing the language resource module

	void __fastcall Reinit();


	//**********************************************************************************************
	// Show Form
	//
	// Same as Show(), but the dialog is repositioned to stay docked to Winamp

	void __fastcall ShowN();


	//**********************************************************************************************
	// Edit Tag
	//
	// Loads the ID666 editor fields with the information obtained from a file
	//
	// In:
	//    fType    = Type of file (returned by ID666::LoadTag)
	//    id6     -> ID666 object of the tag to edit
	//    pFile   -> Buffer containing file
	//    force    = Force the editor to load the tag, even if the current tag has been modified
	//    readonly = Disable editing of the tag

	void __fastcall EditTag(s32 fType, ID666 *pID6, s8 *pFile, b8 force, b8 readonly);


	//**********************************************************************************************
	// Enable Time Buttons
	//
	// In:
	//    fn -> Path and filename of currently playing file (can be NULL)

	void __fastcall EnableTime(const s8 *fn);
};

extern PACKAGE TfrmTag *frmTag;
#endif
