/***************************************************************************************************
* About Box                                                                                        *
*                                                      Copyright (C)2001-2003 Alpha-II Productions *
***************************************************************************************************/

#include	<vcl.h>
#include	"SNESAmp.h"
#pragma	hdrstop
#include	"About.h"


//**************************************************************************************************
#pragma	package(smart_init)
#pragma	resource "*.dfm"

TfrmAbout	*frmAbout;


//**************************************************************************************************
__fastcall TfrmAbout::TfrmAbout(TComponent* owner):TForm(owner)
{
}


//**************************************************************************************************
void __fastcall TfrmAbout::Reinit()
{
	HelpFile = Application->HelpFile;
}


//**************************************************************************************************
void __fastcall TfrmAbout::FormCreate(TObject *sender)
{
	Reinit();

	lblTitle->Caption = "SNESAmp input plug-in v3.1";
}


//**************************************************************************************************
void __fastcall TfrmAbout::btnReadMeClick(TObject *sender)
{
	WinHelp(frmAbout->Handle, hlpFile, HELP_CONTENTS, 0);
	Close();
}


//**************************************************************************************************
void __fastcall TfrmAbout::btnOKClick(TObject *sender)
{
	Close();
}

