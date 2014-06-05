/***************************************************************************************************
* Winamp Functions                                                                                 *
*                                                      Copyright (C)2000-2003 Alpha-II Productions *
***************************************************************************************************/

#include	<vcl.h>
#include	<windows.h>
#include	"SNESAmp.h"
#pragma	hdrstop
#include	<math.h>
#include	"About.h"
#include	"Config.h"
#include	"Control.h"
#include	"Tag.h"

#pragma	inline

//**************************************************************************************************
// Defines and Enumerations

typedef enum
{
	TS_DEAD,
	TS_NOTEMU,
	TS_EMU
} ThStat;


//**************************************************************************************************
// Private Function Prototypes

void	HookWinamp();

static void	About(HWND parent);
static void	Config(HWND parent);
static void	Init();
static void	Quit();
static s32	IsOurFile(s8 *fn);
static void	GetTitle(s8 *fName, s8 *fInfo, s32 &fLen);
static s32	EditTag(s8 *fn, HWND parent);
static s32	GetLength();
static s32	Play(s8 *fn);
static s32	IsPaused();
static void	Pause();
static void	Unpause();
static void	Stop();
static void	SetVol(s32 vol);
static void	SetBal(s32 pan);
static s32	GetPos();
static void	SetPos(u32 time);
static void	SetEQ(s32 on, s8 band[10], s32 amp);
static u32	EmuAPUThread(void *kill);
static void	SPCHalt(volatile u8 *pc, volatile u16 ya, volatile u8 x, volatile SPCFlags psw,
					volatile u8 *sp, volatile u32 cnt);


//**************************************************************************************************
// Variables

//Export structure -----------------------------
InModule inMod =
{
	0x100,										//Input plug-in interface version
	"Alpha-II SPC Player v3.1 (x86)",			//Plug-in description
	0,											//Handle to Winamp window (filled by Winamp)
	0,											//Handle to this DLL (filled by Winamp)

	//Filespec used by in_snes
	"0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF" \
	"SNES music files (*.SPC, *.RSN, *.ZST)\0\0",
	1,											//Current song is seekable
	1,											//Uses an output plugin

	//Other windows
	Config,
	About,

	//Setup and cleanup
	Init,
	Quit,

	//Playlist information
	GetTitle,
	EditTag,
	IsOurFile,

	//Playback
	Play,
	Pause,
	Unpause,
	IsPaused,
	Stop,

	//Time and position
	GetLength,
	GetPos,
	SetPos,

	//Volume and balance
	SetVol,
	SetBal,

	//Visualization (filled by Winamp)
	0,0,0,0,0,0,0,0,0,

	//dsp processing (filled by Winamp)
	0,0,

	//Output
	SetEQ,										//Equalizer
	0,											//Output settings
	0											//-> OutMod structure
};


static s8	dspBase[4608];						//DSP output buffer
static s8	curFile[0x10200];					//Current file being played
static s8	lastFile[0x10200];					//Last file opened

//Playback thread ------------------------------
static HANDLE	th;								//Thread handle
static ThStat	tStatus;						//Thread status
static u32	tID;							  	//Regardless of what MSDN reads, Win98 needs this
static u32	tSeek;								//Position to seek to in song (-1 if no seek)
static u8	tKill;								//Flag to kill thread
static u8	tPaused;							//Song is paused
static u8	tStop;								//Flag to stop playing song
static u8	tSleep;						  		//Stop thread from emulating

static s32	frameSize;							//Size of each sample frame in bytes
static u32	amp;								//Current amplification level

static u32	timerOff;							//Disable playback timer
static u32	songLen;

static u32	outSongs;							//Number of songs that have completely been played
static u32	outTime;							//How much time has been played
static u32	outSmp;								//Number of samples that have been generated
static u32	outByte;							//Number of bytes that have been generated


//File information ----------------------------
static ID6Type	curType,lastType;
static ID666	curID6,lastID6;
static b8		curRAR,lastRAR,loaded;

static HANDLE	fh;				   				//General file handle
static DWORD	l;				   				//Temp used for file operations

extern UnrarFunc	rar;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Exported Functions

//**************************************************************************************************
// Get Input Module Information (Winamp)
//
// Returns the location of the InModule structure that contains pointers to all the various
// functions of the plug-in.  This is the only exported function. (see in_snes.def)
//
// In:
//    nothing
//
// Out:
//    -> InModule structure

__declspec(dllexport) InModule* __stdcall winampGetInModule2()
{
	return &inMod;
}


//**************************************************************************************************
// Change Output Settings

void __fastcall ChangeOutput(b8 reset)
{
	u32	opts;
	s32	maxLatency;


	opts = (cfg.dsp.lowPass ? DSP_ANALOG:0) |
		   (cfg.dsp.oldADPCM ? DSP_OLDSMP:0) |
		   (cfg.dsp.surround ? DSP_SURND:0) |
		   (cfg.dsp.reverse ? DSP_REVERSE:0) |
		   (cfg.dsp.noEcho ? DSP_NOECHO:0);

	tSleep |= 1;								//Set flag for playback thread to wait
	while(tStatus == TS_EMU){};					//Wait for thread to stall

	sapu.SetAPUOpt(cfg.dsp.mix, cfg.dsp.chn, cfg.dsp.bits, cfg.dsp.rate, cfg.dsp.inter, opts);
	frameSize = cfg.dsp.chn * (cfg.dsp.bits >> 3);

	if (reset && tStatus != TS_DEAD)			//Special handling, if an SPC is currently playing
	{
		inMod.outMod->Close();					//Close output stream
		inMod.SAVSADeInit();
												//Reopen stream with new settings
		maxLatency = inMod.outMod->Open(cfg.dsp.rate, cfg.dsp.chn, cfg.dsp.bits, 0, 0);
		if (maxLatency >= 0)
		{
			inMod.SetInfo(cfg.dsp.bits, cfg.dsp.rate/1000, cfg.dsp.chn, 1);
			inMod.SAVSAInit(maxLatency, cfg.dsp.rate);
			inMod.VSASetInfo(cfg.dsp.rate, cfg.dsp.chn);
			inMod.outMod->SetVolume(-666);
		}
		else
			inMod.Stop();						//Kill current song, if there was a problem
	}

	tSleep &= ~1;
}


//**************************************************************************************************
// Mute Voice

v0 __fastcall MuteVoice(u8 voice, b8 state)
{
	if (state)
		sapu.voice[voice].mFlg |= 1;
	else
		sapu.voice[voice].mFlg &= ~1;
}


//**************************************************************************************************
void __fastcall SetMute(u32 m)
{
	s32	i;


	if (frmCtrl)
	{
		frmCtrl->SetMute(m);
		return;
	}

	for (i=0;i<8;i++)
	{
		MuteVoice(i, m & 1);
		m >>= 1;
	}
}


//**************************************************************************************************
// Set Mixing Level

v0 __fastcall SetAmp(u32 i)
{
	if (i != -1) amp = i;

	if (frmCtrl) frmCtrl->SetAmp(amp);

	if ((cfg.waVol & 1) && !(cfg.waVol & 2))
	{
		cfg.waVol |= 2;

		i = F2I(YLog2(64.0, (s32)amp / 65536.0)) + 64;

		SendMessage(inMod.hMainWindow, WM_USER, i, IPC_SETVOLUME);

		cfg.waVol &= ~2;
	}

	sapu.SetDSPAmp(amp);
}


//**************************************************************************************************
// Set ID666 Tag

v0 __fastcall SetID666(ID666 &id6)
{
	if (curID6.SameFile(id6.file))
	{
		curID6 = id6;

		if (!timerOff)
			songLen = sapu.SetAPULength(curID6.GetSong(),curID6.GetFade());

		if (cfg.time.loopx) curID6.loopx = cfg.time.loopx;

		SetMute(curID6.mute);
	}
}


//**************************************************************************************************
// Set Playback Timer State

v0 __fastcall SetTimer(u32 bit, b8 state)
{
	//Set or clear bit -------------------------
	if (state)
		timerOff &= ~(1<<bit);
	else
		timerOff |= 1<<bit;

	//Turn timer on or off ---------------------
	if (!timerOff)
	{
		inMod.seekable = true;
		if (*sapu.t64Cnt < curID6.GetSong())	//If song isn't over, restore time
			songLen = sapu.SetAPULength(curID6.GetSong(), curID6.GetFade());
		else									//Otherwise begin fade out
			songLen = sapu.SetAPULength(*sapu.t64Cnt, curID6.GetFade());
	}
	else
	{
		inMod.seekable = false;
		sapu.SetAPULength(-1,0);
	}
}


//**************************************************************************************************
// Get Output Time

u32	__fastcall GetTime()
{
	u32	time;


	tSleep |= 2;

	time = inMod.outMod->GetWrittenTime();		//Get unplayed buffer length
	time -= inMod.outMod->GetOutputTime();

	if (frmCtrl)
		time = (time * frmCtrl->speed) >> 10;
	else
		time <<= 6;								//Convert to timer ticks

	time = *sapu.t64Cnt - time;					//Subtract from time emulated for total time played

	tSleep &= ~2;

	return time;
}


//**************************************************************************************************
// Load File
//
// Detects a file type and reads it into memory if it's recognized.
//
// If the file is a .RAR, it will be searched for any files with an .SPC extension.  Any files found
// will be added to the playlist.  The file returned will be the first file in the RAR archive.
//
// In:
//    fn   -> Path and filename
//    id6  -> ID666 object to fill
//    pData-> 0x10200 byte buffer to fill with SPC data
//    pRAR -> Boolean value set to true, if file was extracted from a RAR
//    xpl   = Check if file is a RAR file, if it is open and extract the playlist (*.SPC)
//
// Out:
//    Type of file opened (ROM files will return ID6_UNK)

typedef struct
{
	s8	*pData;									//-> data buffer
	s32	alloc;									//Size of buffer
	s32	size;									//Size of data in buffer (can be less than 'alloc')
} RARFile;

static s32 __stdcall RARCallback(u32 msg, long data, long p1, long p2)
{
	RARFile	*p;


	if (msg == UCM_PROCESSDATA && ((RARFile*)data)->size >= 0)
	{
		p = (RARFile*)data;

		if (p->alloc < p->size + p2)			//Is more space needed in the buffer?
		{
			p->pData = (s8*)realloc(p->pData, p->size + p2);
			p->alloc = p->size + p2;
		}

		memcpy(&p->pData[p->size], (s8*)p1, p2);
		p->size += p2;
	}

	return 0;
}

ID6Type __fastcall LoadFile(s8 *fn, ID666 &id6, s8 *pData = NULL, b8 *pRAR = NULL, b8 xpl = 0)
{
	static b8			parsing = 0;

	s8					str[256],*s;
	ID6Type				type;
	b8					setCur;
	DWORD				l;
	SPCReg				*pReg;

	COPYDATASTRUCT		cds;

	RAROpenArchiveData	ropen;
	RARHeaderData		rhdr;
	HANDLE				rh;
	RARFile				rf;


	if (pRAR) *pRAR = 0;

	type = id6.LoadTag(fn);

	switch(type)
	{
	case(ID6_UNK):								//File was unknown, it could be a RAR file
		if (!xpl || !rar.GetDllVersion || parsing) return ID6_UNK;

		parsing = 1;							//RAR file is being parsed

		//Open RSN file ------------------------
		memset(&ropen, 0, sizeof(RAROpenArchiveData));
		ropen.ArcName	= fn;
		ropen.OpenMode	= RAR_OM_EXTRACT;//LIST;

		rh = rar.OpenArchive(&ropen);
		if (!rh) goto Error;

		rf.pData = NULL;
		rf.alloc = 0;
		rf.size = -1;
		rar.SetCallback(rh, RARCallback, (s32)&rf);

		//Get pointer to internal filename -----
		strcpy(str, fn);
		s = ::StrEnd(str);
		*s++ = '\\';

		setCur = 0;								//Initial file hasn't been found yet

		cds.dwData = IPC_PLAYFILE;
		cds.lpData = str;

		while(!rar.ReadHeader(rh, &rhdr))
		{
			if ((rhdr.Flags & 0xE0) != 0xE0)
			{
				if (!strcmpi(ScanStrR(rhdr.FileName, '.'), "spc"))
				{
					strcpy(s, rhdr.FileName);

					//Extract file -------------
					rf.size = 0;
					rar.ProcessFile(rh, RAR_TEST, NULL, NULL);

					lastType = lastID6.LoadTag(rf.pData, rf.size);
					strcpy(lastID6.file, str);
					lastRAR = 1;

					if (lastType == ID6_SPC || lastType == ID6_EXT)
						memcpy(lastFile, rf.pData, 0x10200);
					else
						lastType = ID6_UNK;

					rf.size = -1;

					//Add file to playlist -----
					if (!setCur)				//If this is the first SPC, change current PL file
					{
						SendMessage(inMod.hMainWindow, WM_WA_IPC, (WPARAM)str, IPC_CHANGECURRENTFILE);
						setCur = 1;
					}
					else						//Otherwise add the file to the play list
					{
						cds.cbData = strlen(str) + 1;
						SendMessage(inMod.hMainWindow, WM_COPYDATA, (WPARAM)0, (LPARAM)&cds);
					}

					continue;
				}
			}

			if (rar.ProcessFile(rh, RAR_SKIP, NULL, NULL)) break;
		}

		if (rf.pData) free(rf.pData);
		rar.CloseArchive(rh);

		parsing = 0;
		if (!setCur) return ID6_UNK;

		l = SendMessage(inMod.hMainWindow, WM_WA_IPC, 0, IPC_GETLISTPOS);
		strcpy(str, (s8*)SendMessage(inMod.hMainWindow, WM_WA_IPC, l, IPC_GETPLAYLISTFILE));

	case(ID6_ERR):								//Error opening file, could be inside a RAR archive
		if (!rar.GetDllVersion || parsing) return ID6_ERR;

		parsing = 1;

		if (type == ID6_ERR) strcpy(str, fn);

		s = str;
		do
		{
Slash:
			s = strchr(s+1, '\\');
			if (!s) goto Error;
			if (s < &str[4]) goto Slash;
		} while ((*(u32*)&s[-4] | 0x20202000) != '.rsn');
		*s++ = 0;

		memset(&ropen, 0, sizeof(RAROpenArchiveData));
		ropen.ArcName	= str;
		ropen.OpenMode	= RAR_OM_EXTRACT;

		rh = rar.OpenArchive(&ropen);
		if (!rh) goto Error;

		rf.pData = NULL;
		rf.alloc = 0;
		rf.size = -1;
		rar.SetCallback(rh, RARCallback, (s32)&rf);

		while(!rar.ReadHeader(rh, &rhdr))
		{
			if ((rhdr.Flags & 0xE0) != 0xE0 && !strcmpi(s, rhdr.FileName))
			{
				rf.size = 0;
				rar.ProcessFile(rh, RAR_TEST, NULL, NULL);

				if (pRAR) *pRAR = 1;

				type = id6.LoadTag(rf.pData, rf.size);
				s[-1] = '\\';
				strcpy(id6.file, str);

				if (type != ID6_SPC && type != ID6_EXT) type = ID6_UNK;
				else
				if (pData) memcpy(pData, rf.pData, 0x10200);

				free(rf.pData);
				rar.CloseArchive(rh);
				parsing = 0;
				return type;
			}

			rar.ProcessFile(rh, RAR_SKIP, NULL, NULL);
		}

		rar.CloseArchive(rh);
Error:
		parsing = 0;
		return ID6_ERR;

	case(ID6_SPC):
	case(ID6_EXT):
		if (pData)
		{
			fh = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);
			ReadFile(fh, pData, 0x10200, &l, NULL);
			CloseHandle(fh);
		}
		return type;

	case(ID6_ZST):
		if (pData)
		{
			fh = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, NULL);

			memset(pData, 0, 256);

			ReadFile(fh, &pData[0], 32, &l, NULL);			//File header
			SetFilePointer(fh, 199699, 0, FILE_BEGIN);
			ReadFile(fh, &pData[0x100], 65536, &l, NULL);	//64K RAM
			SetFilePointer(fh, 16, 0, FILE_CURRENT);
			ReadFile(fh, &pData[0], 7*4, &l, NULL);			//Registers
			SetFilePointer(fh, 20, 0, FILE_CURRENT);
			ReadFile(fh, &pData[0x101C0], 64, &l, NULL);	//Extra RAM
			SetFilePointer(fh, 1260, 0, FILE_CURRENT);
			ReadFile(fh, &pData[0x10100], 128, &l, NULL);	//DSP Registers

			pReg = &((SPCHdr*)pData)->reg;
			pReg->pc[0]	= pData[0];				//PC(l)
			pReg->pc[1]	= pData[1];				//PC(h)
			pReg->a		= pData[1*4];			//A
			pReg->x		= pData[2*4];			//X
			pReg->y		= pData[3*4];			//Y
			pReg->psw	= pData[4*4] & 0x7D;	//PSW (reset N & Z)
			pReg->sp	= pData[6*4];			//SP

			if (pData[5*4] != 0)				//Fixup N & Z flags from previous result
			{
				pReg->psw |= pData[5*4] & 0x80;
				pReg->psw &= ~2;
			}
			else
			{
				pReg->psw &= ~0x80;
				pReg->psw |= 2;
			}


			CloseHandle(fh);
		}
		return ID6_ZST;
	}

	return ID6_UNK;
}


//**************************************************************************************************
// Check Playlist File
//
// Grabs the filename of the currently playing file and checks if it's playable by SNESAmp
//
// Note:
//    This function only gets called/only needs to be called if the Winamp process is hooked
//
// In:
//    fn -> Path and filename of file to be played
//
// Out:
//    loaded = true, if LoadFile() didn't return ID6_ERR

void __fastcall SongChange(s8 *fn)
{
	ID6Type		type;
	b8			tag,ctrl;


	//Do we need to check the file type at this moment?
	tag = frmTag && frmTag->Visible && frmTag->chkUpdate; //frmTag must be visible and want updates
	ctrl = frmCtrl && frmCtrl->showable;		//frmCtrl must be visible

	if (!(tag || ctrl)) return;

	//Get file type ----------------------------
	if (fn)
	{
		if (curID6.SameFile(fn))				//If this is the same file
		{
			loaded = (curType >= ID6_SPC);
			if (loaded) return;					//If we already know what it is, return
		}

		curType = LoadFile(fn, curID6, curFile, &curRAR, 1);
	}
	else
		curType = ID6_ERR;						//If no filename was passed, return error

	loaded = (curType >= ID6_SPC);

	//Update forms based on playability --------
	if (curType >= ID6_SPC)						//If this file is playable, update the tag editor
	{
		if (tag) frmTag->EditTag(curType, &curID6, curFile, 0, curRAR);
	}
	else										//If it's not, hide the Control dialog
	{
		if (ctrl)
		{
			frmCtrl->Hide();
			frmCtrl->showable = 0;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Winamp Functions

//**************************************************************************************************
// About Plug-in
//
// Displays the About box

static v0 About(HWND parent)
{
	s8	str[32];

	frmAbout = new TfrmAbout(NULL);

	wsprintf(str, "Using SNESAPU.DLL v%01X.%02X%C", emuVer>>16, (emuVer>>8) & 0xFF, emuVer & 0xFF);
	frmAbout->lblEmu->Caption = str;
	frmAbout->ParentWindow = parent;
	frmAbout->ShowModal();

	delete frmAbout;
	frmAbout = NULL;
}


//**************************************************************************************************
// Configure Plug-in
//
// Displays the configuration box, and updates changes to the plug-in

static v0 Config(HWND parent)
{
	frmConfig = new TfrmConfig(NULL);

	frmConfig->ShowModal();

	delete frmConfig;
	frmConfig = NULL;
}


//**************************************************************************************************
// Initialize Plug-in
//
// Allocates memory, initializes Winamp and the emulator, and sets up values.
// Called when Winamp is opened.

static v0 Init()
{
	static b8	initialized = 0;
	u32			opts;


	if (initialized) return;					//*** Workaround for a bug in Winamp that calls Init
												//    more than once ***
	initialized = 1;
	pLanguages = Languages();					//Get handle to language object
	inMod.hMainWndNew = NULL;

	memset(&cfg, 0, sizeof(cfg));

	//Get location of plugin.ini ---------------
	strcpy(cfg.iniFile, dllPath);
	strcpy(ScanStrR(cfg.iniFile,'\\'), "plugin.ini");	//Append 'plugin.ini'

	//Get configuration ------------------------
	opts = GetPrivateProfileInt("SNESAmp", "Version", 0, cfg.iniFile);
	if (opts != CUR_VER)
	{
		WritePrivateProfileString("SNESAmp", NULL, NULL, cfg.iniFile);
		WritePrivateProfileString("SNESAmp", "Version", "310", cfg.iniFile);
	}
	LoadConfig(cfg);

	*(1 + CopyStr(1 + CopyStr(inMod.fileExt, cfg.fileExt), "SNES music files (*.SPC, *.RSN, *.ZST)")) = 0;

	if (cfg.tricks) InitDock();					//Initialize docking library

	//Load language pack -----------------------
	LoadLRM(cfg.language);

	//Set emulation options --------------------
	sapu.SetSPCDbg(SPCHalt,0);
	opts = (cfg.dsp.lowPass ? DSP_ANALOG:0) |
		   (cfg.dsp.oldADPCM ? DSP_OLDSMP:0) |
		   (cfg.dsp.surround ? DSP_SURND:0) |
		   (cfg.dsp.reverse ? DSP_REVERSE:0) |
		   (cfg.dsp.noEcho ? DSP_NOECHO:0);
	sapu.SetAPUOpt(cfg.dsp.mix, cfg.dsp.chn, cfg.dsp.bits, cfg.dsp.rate, cfg.dsp.inter, opts);
	frameSize = cfg.dsp.chn * (cfg.dsp.bits >> 3);
	sapu.SetDSPStereo((cfg.dsp.stereo << 16) / 100);
	sapu.SetDSPEFBCT((cfg.dsp.echo << 15) / 100);

	timerOff = 0;
	SetTimer(0,cfg.time.useTimer);

	SetAmp(cfg.mix.amp);

	//Initialize ID666 options -----------------
	curID6.defSong = cfg.time.song;
	curID6.defFade = cfg.time.fade;
	curID6.preferBin = cfg.time.defBin;
	curType = ID6_ERR;
	loaded = 0;

	th = INVALID_HANDLE_VALUE;
	tKill = 0;
	tStop = 0;
	tSleep = 0;
	tPaused = 0;
	tStatus = TS_DEAD;
	tSeek = ~0;
}


//**************************************************************************************************
// Shutdown Plug-in
//
// Deallocates memory, and saves the current amplification level.
// Called when Winamp is closed.

static v0 Quit()
{
	s8	 option[16];


	if (frmCtrl)
	{
		//Save user defined mixing level -------
		if (!(cfg.mix.aar & AAR_TYPE))
		{
			wsprintf(option,"%i",amp);
			WritePrivateProfileString("SNESAmp", "Amp", option, cfg.iniFile);
		}

		//Save Control dialog position ---------
		GetRelPos(frmCtrl, cfg.ctrlTop, cfg.ctrlLeft);
		wsprintf(option,"%i",cfg.ctrlTop);
		WritePrivateProfileString("SNESAmp", "CtrlTop", option, cfg.iniFile);
		wsprintf(option,"%i",cfg.ctrlLeft);
		WritePrivateProfileString("SNESAmp", "CtrlLeft", option, cfg.iniFile);

		delete frmCtrl;

		frmCtrl = NULL;
	}

	if (frmTag)
	{
		//Save dialog position -----------------
		GetRelPos(frmTag, cfg.tagTop, cfg.tagLeft);
		wsprintf(option,"%i",cfg.tagTop);
		WritePrivateProfileString("SNESAmp", "TagTop", option, cfg.iniFile);
		wsprintf(option,"%i",cfg.tagLeft);
		WritePrivateProfileString("SNESAmp", "TagLeft", option, cfg.iniFile);

		delete frmTag;

		frmTag = NULL;
	}

	if (cfg.tricks) ShutDock();
}


//**************************************************************************************************
// Is This Our File
//
// Winamp will ask us if this is our file before trying to play or get title information

static s32 IsOurFile(s8 *fn)
{
	if (loaded && curID6.SameFile(fn)) return 1;	//If we've already loaded the file, it belongs
	if (lastID6.SameFile(fn)) return 1;				// to SNESAmp

	lastType = LoadFile(fn, lastID6, lastFile, &lastRAR, 0);	//Don't scan .RSN for other SPC's

	if (lastType >= ID6_SPC) return 1;				//If the unknown file type has an .rsn extension
	if (lastType == ID6_UNK) return strcmpi(ScanStrR(fn, '.'), "rsn") == 0;

	return 0;										//If there was an error, return 0
}


//**************************************************************************************************
// Get File Information
//
// Returns information about a song to populate the playlist
//
// In:
//    fName -> Filename to extract info from (NULL if WA wants info for currently playing file)
//    title -> Playlist title
//    len   -> Song length

static v0 GetTitle(s8 *fName, s8 *title, s32 &len)
{
	if (!fName || !*fName)						//Make sure WA sent us a valid filename
	{
		if (title) curID6.ToStr(title, cfg.titleFmt);
		if (&len) len = GetLength();			//Send back length
	}
	else										//Open the file and extract info
	{
		//This function will usually get called right after IsOurFile.  If this file isn't the same
		//file that was just tested, we need to load the information.

		if (!lastID6.SameFile(fName)) lastType = LoadFile(fName, lastID6, lastFile, &lastRAR, 0);

		switch(lastType)
		{
		case(ID6_SPC):							//.SPC
		case(ID6_EXT):
			if (title) lastID6.ToStr(title, cfg.titleFmt);
			if (&len) len = cfg.time.useTimer ? (lastID6.GetTotal() >> 6) : 0;
			break;

		case(ID6_ZST):							//.ZST
			if (title) strcpy(title, ScanStrR(fName, '\\'));
			if (&len) len = cfg.time.useTimer ? (lastID6.GetTotal() >> 6) : 0;
			break;

		default:								//Unknown or invalid file
			if (title) strcpy(title, fName);
			if (&len) len = -1;
		}
	}
}


//**************************************************************************************************
// Song Information Dialog Box
//
// Displays information about a song and edits the ID666 tag
//
// Out:
//    ???

static s32 EditTag(s8 *fn, HWND parent)
{
	if (!(lastID6.SameFile(fn) && lastRAR))
		lastType = LoadFile(fn, lastID6, lastFile, &lastRAR, 0);

	if (lastType)								//Display editor, if file was successfully opened
	{
		if (!frmTag)
		{
			HookWinamp();
			frmTag = new TfrmTag(NULL);
		}

		frmTag->EditTag(lastType, &lastID6, lastFile, 1, lastRAR);
//		frmTag->ParentWindow = parent;
//		SetRelPos(frmTag, cfg.tagTop, cfg.tagLeft);
		frmTag->Show();
		frmTag->EnableTime(curID6.file);
	}
	else
		Application->MessageBox(LoadStr(STR_ERR_OPEN).c_str(), "SNESAmp plug-in",
								MB_OK | MB_ICONEXCLAMATION);

	return 0;
}


//**************************************************************************************************
// Get Song Length
//
// Returns the length of the current song
//
// Out:
//    Song length in ms

static s32 GetLength()
{
	return timerOff ? 0 : (curID6.GetTotal() >> 6);
}


//**************************************************************************************************
// Start Emulation
//
// Loads an SPC700 dump, initializes a bunch of stuff, and begins the emulation thread.
//
// In:
//    fn -> Name of file to load
//
// Out:
//    0, if everything's cool


static s32 Play(s8 *fn)
{
	s32	maxlatency;
	s32	i;


	if (!loaded)								//If file isn't already loaded by SongChange()
	{
		if (curID6.SameFile(fn)) loaded = (curType >= ID6_SPC);

		if (!loaded)
		{
			curType = LoadFile(fn, curID6, curFile, &curRAR, 1);
			loaded = (curType >= ID6_SPC);
		}
	}

	if (curType < ID6_SPC) return -1;
	if (curType == ID6_EXT) curID6.loopx = cfg.time.loopx;	//Override tag's loop count

	//Open output plug-in ----------------------
	maxlatency = inMod.outMod->Open(cfg.dsp.rate, cfg.dsp.chn, cfg.dsp.bits, 0, 0);
	if (maxlatency < 0) return 1;				//Error opening device

	//Create Control dialog --------------------
	if (cfg.ctrl && !frmCtrl)
	{
		HookWinamp();
		frmCtrl = new TfrmCtrl(NULL);
		frmCtrl->SetAmp(amp);
	}

	//Reset APR --------------------------------
	cfg.mix.aar &= AAR_TYPE;					//Reset upper temp bits
	if (cfg.mix.tagAmp && curID6.amp)			//If the tag has a level, and the user wants to use
		SetAmp(curID6.amp);						// it, disable AAR and the the level
	else										//If APR is turned on, and the user wants the level
	{
		if (cfg.mix.reset) SetAmp(cfg.mix.amp);
		if (cfg.mix.aar & AAR_TYPE) cfg.mix.aar |= AAR_ON;	//Enable AAR flag if user wants it on
	}

	//Update ID666 tag editor ------------------
	if (frmTag && frmTag->Visible) frmTag->EnableTime(fn);

	//Fixup APU for emulation ------------------
	sapu.LoadSPCFile(curFile);
	sapu.SetSPCDbg((SPCDebug*)-1, 0);			//Reset debugging flags that may have been set by

	if (!timerOff)								//Set song length
		songLen = sapu.SetAPULength(curID6.GetSong(), curID6.GetFade());

	//Display Control dialog -------------------
	if (frmCtrl && !frmCtrl->Visible) frmCtrl->ShowN();

	SetMute(curID6.mute);

	//Initialize Winamp ------------------------//Display: kbps, kHz, Mono/Stereo
	inMod.SetInfo(cfg.dsp.bits, cfg.dsp.rate/1000, cfg.dsp.chn, 1);
	inMod.SAVSAInit(maxlatency, cfg.dsp.rate);	//Initialize visualization stuff
	inMod.VSASetInfo(cfg.dsp.rate, cfg.dsp.chn);
	inMod.outMod->SetVolume(-666);				//Set the output plug-in's default volume

	//Start thread -----------------------------
	//The emulation thread should be non-existant at this point, but too often it's still going.

	if (tStatus!=TS_DEAD && !tKill) Stop();		//If the thread is alive and not dying, stomp it
	else										//Othewise, wait a bit for it
	{
		i = 0;
		while(tStatus != TS_DEAD)
		{
			i++;
			Sleep(10);
			if (i > 200)						//If the thread won't go away quietly after 2 sec...
			{
//				Application->MessageBox("The damn thread isn't dead!","SNESAmp plug-in",
//  										MB_OK|MB_ICONEXCLAMATION);
				TerminateThread(th,0);
				tStatus = TS_DEAD;
			}
		}
	}

	th = (HANDLE)CreateThread(NULL, 40, (LPTHREAD_START_ROUTINE)EmuAPUThread, NULL, 0, (LPDWORD)&tID);
	if (!th) return -1;

	return 0;
}


//**************************************************************************************************
// Is Emulation paused?
//
// Returns true, if the song is currently paused

static s32 IsPaused()
{
	return tPaused;
}


//**************************************************************************************************
// Pause Emulation
//
// Pauses the song, and tells the output plug-in to wait

static v0 Pause()
{
	tPaused = 1;
	inMod.outMod->Pause(1);
}


//**************************************************************************************************
// Unpause Emulation
//
// Unpauses the song, and informs the output plug-in to resume

static v0 Unpause()
{
	tPaused = 0;
	inMod.outMod->Pause(0);
}


//**************************************************************************************************
// Stop Emulation
//
// Sets a flag to kill the thread, then tells the output plug-in to stop

static v0 Stop()
{
	if (tStatus != TS_DEAD)						//Make sure there's a thread to kill
	{
		tKill = 1;								//Flag thread to go away quietly
		if (WaitForSingleObject(th,2000) == WAIT_TIMEOUT)	//Give it some time to work things out
		{
//			Application->MessageBox("The emulation thread won't cooperate.  Press OK to terminate.",
//									"SNESAmp Error",MB_OK|MB_ICONEXCLAMATION);
			TerminateThread(th,0);				//Stop thread NOW!
		}
		tKill = 0;								//Reset flags
		tStop = 0;
		CloseHandle(th);						//Deallocate handle resources
		tStatus = TS_DEAD;						//Set thread status to dead
		th = INVALID_HANDLE_VALUE;				//Invalidate handle
		inMod.outMod->Close();					//Close output stream
		inMod.SAVSADeInit();					//Disable visualization
	}

	loaded = 0;

	if (frmTag) frmTag->EnableTime(NULL);		//Disable timer button features
}


//**************************************************************************************************
// Set Volume
//
// Called when the user moves the volume bar
//
// In:
//    Vol = Volume (0-255)

static v0 SetVol(s32 vol)
{
	s32	i;


	if ((cfg.waVol & 1) && !(cfg.waVol & 2))
	{
		cfg.waVol |= 2;

		i = F2I(pow(2.0, (vol - 64) / 64.0) * 65536.0);
		SetAmp(i);

		cfg.waVol &= ~2;
	}

	inMod.outMod->SetVolume(vol);
}


//**************************************************************************************************
// Set Balance
//
// Called when the user moves the balance bar

static v0 SetBal(s32 pan)
{
	inMod.outMod->SetPan(pan);
}


//**************************************************************************************************
// Get Position
//
// Returns the current position in the song
//
// Out:
//    Position in ms

static s32 GetPos()
{
	return GetTime() >> 6;
}


//**************************************************************************************************
// Set Position
//
// Called when the user moves the seek bar
//
// In:
//    time = Position to seek to in ms

static v0 SetPos(u32 time)
{
	if (tSeek == ~0)							//If no seek is taking place, set seek position
		tSeek = time<<6;
}


//**************************************************************************************************
// Set Equalizer
//
// A useless function that never gets called

static v0 SetEQ(s32 on, s8 band[10], s32 amp)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Local Functions

void __fastcall Reduce32to16(void*)
{
	asm
	{
		mov			esi,eax
		mov			edi,eax
		mov			ecx,576/8
	Next32:
		movq		mm7,[56+esi]	//Load 8 32-bit stereo sample frames
		movq		mm6,[48+esi]
		movq		mm5,[40+esi]
		movq		mm4,[32+esi]
		movq		mm3,[24+esi]
		movq		mm2,[16+esi]
		movq		mm1,[08+esi]
		movq		mm0,[00+esi]
		add			esi,64

		psrad		mm7,16			//Reduce samples to 16-bit
		psrad		mm6,16
		psrad		mm5,16
		psrad		mm4,16
		psrad		mm3,16
		psrad		mm2,16
		psrad		mm1,16
		psrad		mm0,16

		packssdw	mm6,mm7			//Pack two frames per register
		packssdw	mm4,mm5
		packssdw	mm2,mm3
		packssdw	mm0,mm1

		movq		[24+edi],mm6	//Store samples
		movq		[16+edi],mm4
		movq		[08+edi],mm2
		movq		[00+edi],mm0
		add			edi,32

		dec			ecx
		jnz			short Next32

		emms
	}
}


void __fastcall Reduce24to16(void*)
{
	asm
	{
		mov 	esi,eax
		mov		edi,eax
		push	ebp
		mov		ebp,576/4
	Next24:
		mov		bx,[10+esi]
		mov		dx,[7+esi]
		mov		cx,[4+esi]
		mov		ax,[1+esi]

		mov		[6+edi],bx
		mov		[4+edi],dx
		mov		[2+edi],cx
		mov		[0+edi],ax

		mov		bx,[22+esi]
		mov		dx,[19+esi]
		mov		cx,[16+esi]
		mov		ax,[13+esi]
		add		esi,24

		mov		[14+edi],bx
		mov		[12+edi],dx
		mov		[10+edi],cx
		mov		[8+edi],ax
		add		edi,16

		dec		ebp
		jnz		short Next24

		pop		ebp
	}
}


//**************************************************************************************************
// Emulation Thread
//
// Emulates the sound module and passes output to the Winamp dsp buffer.
//
// In:
//    -> ???
//
// Out:
//    NULL

static u32 EmuAPUThread(v0 *p)
{
	s32		numSmp;								//Number of samples needed for Winamp's DSP buffer
	u32		i,j;

	u32	t64Diff = 0;							//T64 before last emulation call
	u32	t64Inc = 0;								//T64 when level was last increased
	u32	t64Update = 0;							//T64 when stats were last updated
	u32	t64Active = 0;							//T64 when song was last know to be active
	s8	str[12];


	tStatus = TS_NOTEMU;						//Thread status: Not emulating
	while (!tKill)
	{
		if (tSleep)
		{
			Sleep(10);
			continue;
		}

		if (tStop)
		{
			if (!inMod.outMod->IsPlaying())		//Is output buffer empty?
			{
				PostMessage(inMod.hMainWindow, WM_WA_MPEG_EOF, 0, 0);
				outSongs++;
				return 0;
			}
			Sleep(10);
			continue;
		}

		if (tSeek != ~0)
		{
			inMod.outMod->Flush(*sapu.t64Cnt >> 6);	//Stop Winamp dsp output

			if (*sapu.t64Cnt >= tSeek)			//Is seek backwards?
			{									//  Yes, Emulation will have to be reset
				sapu.LoadSPCFile(curFile);		//Copy .SPC from backup

				if (!timerOff) sapu.SetAPULength(curID6.GetSong(), curID6.GetFade());
			}

			//Emulate up to seek position ------
			tStatus = TS_EMU;
			sapu.SeekAPU(tSeek - *sapu.t64Cnt, cfg.time.fastSeek);
			tStatus = TS_NOTEMU;

			t64Diff = *sapu.t64Cnt;
			t64Inc = *sapu.t64Cnt;
			t64Update = *sapu.t64Cnt;
			t64Active = *sapu.t64Cnt;

			tSeek = ~0;							//Enable seeking
			continue;
		}

		if (inMod.outMod->CanWrite() >= (576 * frameSize) << (inMod.DSPisActive() ? 1 : 0))
		{
			//Emulate --------------------------
			*sapu.vMMaxL = 0;					//Reset greatest output
			*sapu.vMMaxR = 0;

			tStatus = TS_EMU;
			sapu.EmuAPU(dspBase, 576, 1);    	//Emulate APU
			tStatus = TS_NOTEMU;

			numSmp = 576;	 					//Default number of samples
			if (inMod.DSPisActive())			//Get number of samples needed by Winamp
				numSmp = inMod.DSPdoSamples((s16*)dspBase, 576, cfg.dsp.bits, cfg.dsp.chn, cfg.dsp.rate);
			inMod.outMod->Write(dspBase, numSmp * frameSize);	//Send output to Winamp DSP buffer

			//Update visualization -------------
			if (cfg.dsp.bits >= 24)				 //Reduce samples to 16-bit
			{
				if (cfg.dsp.bits == 32)
					Reduce32to16(dspBase);
				else
					Reduce24to16(dspBase);

				//Send output to visualization
				inMod.SAAddPCMData(dspBase, cfg.dsp.chn, 16, *sapu.t64Cnt >> 6);
				inMod.VSAAddPCMData(dspBase, cfg.dsp.chn, 16, *sapu.t64Cnt >> 6);
			}
			else
			{
				inMod.SAAddPCMData(dspBase, cfg.dsp.chn, cfg.dsp.bits, *sapu.t64Cnt >> 6);
				inMod.VSAAddPCMData(dspBase, cfg.dsp.chn, cfg.dsp.bits, *sapu.t64Cnt >> 6);
			}

			//Mixing level attenation ----------
			if (cfg.mix.aar & AAR_ON)
			{
				j = amp;

				//Has output clipped threshold?
				i = *sapu.vMMaxL;
				if (i < *sapu.vMMaxR) i = *sapu.vMMaxR;	//i = highest output

				if (i > cfg.mix.threshold)
				{
					j = MulDiv(cfg.mix.threshold, amp, i);

					if (j < cfg.mix.minAmp)		//If level has reached minimum, disable attenuation
					{
						j = cfg.mix.minAmp;
						cfg.mix.aar &= ~AAR_ON;
					}

					cfg.mix.aar |= AAR_MAX;		//Level has reached max, disable increase
					SetAmp(j);					//Set new level
				}
				else
				//If APR type is 'increase', and output hasn't clipped or level reached max, then
				// increase level
				if ((cfg.mix.aar & (AAR_TYPE|AAR_MAX))==2)
				{
					if (*sapu.t64Cnt - t64Inc > 8000)	//Has more than 1/8 second gone by?
					{
						t64Inc = *sapu.t64Cnt;
						j *= 1.01161944030;		//60th root of 2, 1/10th of a decibel
						if (j >= cfg.mix.maxAmp)
						{
							j = cfg.mix.maxAmp;
							cfg.mix.aar |= AAR_MAX;
						}
						SetAmp(j);
					}
				}
			}

			//Check for end of song ------------
			if (!timerOff && *sapu.t64Cnt > (songLen + cfg.time.silence))
				tStop = 1;

			//Look for inactivity --------------
			//Autoend only works if the user has the feature enabled and the song doesn't have a
			//time with the timer enabled.
			if (cfg.time.autoEnd && !(cfg.time.useTimer && curID6.HasTime()))
			{
				for(j=8,i=0;i<7;i++)	 		//Check emulation flags to make sure nothing is on
					j &= sapu.voice[i].mFlg;	// (see MixO structure in DSP.h)

				if (j)							//If all voices are inactive, check elapsed time
					tStop |= (*sapu.t64Cnt - t64Active > cfg.time.autoEnd);
				else
					t64Active = *sapu.t64Cnt;
			}

			//Update totals --------------------
			outTime += *sapu.t64Cnt - t64Diff;	//Total time
			outSmp += numSmp;					//Total samples
			outByte += numSmp * frameSize;		//Total bytes
			t64Diff = *sapu.t64Cnt;

			if (*sapu.t64Cnt - t64Update >= 64000)	//Update dialog, if one second has gone by
			{
				t64Update = *sapu.t64Cnt;
				if (frmTag && frmTag->Visible)
				{
					wsprintf(str, "%01i:%02i:%02i", outTime / (XID6_TICKSMIN * 60),
												   (outTime / XID6_TICKSMIN) % 60,
												   (outTime / XID6_TICKSSEC) % 60);
					frmTag->lblTime->Caption = str;
//					*** Apparently printf doesn't work with TLabel ***
//					frmTag->lblTime->Caption.printf("%01i:%02i:%02i",
//													 outTime / (XID6_TICKSMIN * 60),
//													(outTime / XID6_TICKSMIN) % 60,
//													(outTime / XID6_TICKSSEC) % 60);
					frmTag->lblNum->Caption = outSongs;
					frmTag->lblSmp->Caption = outSmp;
					frmTag->lblByte->Caption = outByte;
				}
			}

			continue;
		}

		Sleep(10);								//Sleep for 10 ms while other processes take place
	}

	return 0;
}


//**************************************************************************************************
// SPC700 Halt Error
//
// Stops the current song, if a STOP instruction is encountered
//
// In/Out:
//    See DebugSPC in SPC700.h

void SPCHalt(volatile u8 *pc, volatile u16 ya, volatile u8 x, volatile SPCFlags psw,
			 volatile u8 *sp, volatile u32 cnt)
{
/*	*** For some reason, this code crashes Winamp ***
	s8	str[256];


	//Display debug info that's useless to most users :)
	wsprintf`(str,"SPC700 halted at %04Xh\n",(u16)PC);
	strcat(str,ScanStrR(curID6.file,'\\'));
	strcat(str," may be corrupt.");
	Application->MessageBox(str,"SNESAmp Error",MB_OK|MB_ICONSTOP);
*/
	tStop = 1;									//Stop song

//	sapu.SetSPCDbg((SPCDebug*)-1, SPC_HALT | SPC_RETURN);
}

