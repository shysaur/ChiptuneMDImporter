/***************************************************************************************************
* File:       Window Docking Library                                                               *
* Platform:   Windows C++                                                                          *
* Programmer: Anti Resonance                                                                       *
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
* 1.1  6.4.03                                                                                      *
*      + Added GetWindowAlpha()                                                                    *
*                                                           Copyright (C)2003 Alpha-II Productions *
***************************************************************************************************/


//**************************************************************************************************
// Structures

#define	DOCK_MAXWIN	16							//Max number of windows to test for docking against

typedef struct
{
	HWND	hWindow;							//Handle of window to test
	HWND	hRoot;								//Handle of root window
	u32		thread;								//Handle of thread to enumerate windows from
	u8		windows; 							//Total windows enumerated
	u8		monitors;							//Total monitors enumerated plus windows
	u8		_r[2];
	RECT	rect[DOCK_MAXWIN];					//Bounding rectangles for each window and monitor
	u8		alpha[DOCK_MAXWIN];					//Transparency value of each window, if obtained
} DockTest;


//**************************************************************************************************
// Public Variables

extern BOOL	__stdcall (*pGetWindowInfo)(HWND,PWINDOWINFO);							//Win98
extern BOOL	__stdcall (*pEnumDisplayMonitors)(HDC,LPCRECT,MONITORENUMPROC,LPARAM);	//Win98
extern void	__stdcall (*pGetLayeredWindowAttributes)(HWND,COLORREF*,BYTE*,DWORD*);	//Win2k
extern BOOL	__stdcall (*pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD);		//Win2k


//**************************************************************************************************
// Initialize Docking Library
//
// Initializes the functions pointers to additional functions in the USER32.DLL

void InitDock();


//**************************************************************************************************
// Shutdown Docking Library

void ShutDock();


//**************************************************************************************************
// Is Form Docked to Window?
//
// Determines if a window is docked to, and only to, the root window
//
// Notes:  Windows 95 can't test if the window is docked to other windows
//         This function does not count corner docking
//
// In:
//    pDock -> hWindow = Handle to window to test for docking
//             hRoot   = Handle to window to test for docking against
//             thread  = Thread handle of process
// Out:
//    true, if form is only docked to root window

b8 __fastcall IsDocked(DockTest *pDock);


//**************************************************************************************************
// Is Form Docked to Root Window?
//
// Determines if a window is docked to the root window, either directly or through a path of other
// windows.
//
// Note:  Windows 95 can only test for docking against the root window
//
// In:
//    pDock -> hWindow = Handle to window to test for docking
//             hRoot   = Handle to window to test for docking against
//             thread  = Thread handle of process
//
// Out:
//    true, if form is docked

b8 __fastcall IsDockedPath(DockTest *pDock);


//**************************************************************************************************
// Prepare to Move Dialog
//
// Initalizes the 'rect' array in a DockTest structure to be used by MoveDialog
//
// Note:  Windows 95 can only test for snapping against the root window
//        The first value in 'rect' is the bounding box of the root window
//
// In:
//    pDock -> hWindow = Handle to window being moved
//             hRoot   = Handle to application's main window
//             thread  = Thread handle of process
//    x,y    = Mouse cursor position

void __fastcall PrepareMove(DockTest *pDock, s32 x, s32 y);


//**************************************************************************************************
// Move Dialog Position
//
// Returns the position of a dialog based on the position of the mouse cursor.  If the dialog is
// close enough to another window, the position will snap next to it.
//
// PrepareMove must be called first.  See above.
//
// Note:  MoveDialog uses some static variables, so it can only be used for one window at a time.
//        However, since Windows only lets you move one window at a time with the mouse, this
//        shouldn't be a problem.
// In:
//    pRect -> Rectangle to store new coordinates in
//    pDock -> Windows to test for snapping against
//    x,y    = Mouse cursor position
//
// Out:
//    true, if the dialog snapped to the root window

b8 __fastcall MoveDialog(RECT *pRect, DockTest *pDock, s32 x, s32 y);


//**************************************************************************************************
// Get Alpha of All Windows
//
// Gathers the transparency value (layered attribute) of all windows in a thread
//
// Notes: Only works in Windows 2000 or later.  Other versions return false.
//        Unlike PrepareMove(), the first value in 'alpha' is not guaranteed to be the root window
//        'hWindow' is only needed if you want to exclude a window from alpha retrieval
//
// In:
//    pDock -> hWindow = Handle to current window (can be NULL)
//             thread  = Thread handle of process
//
// Out:
//    true, if function was successful

b8 __fastcall GetWindowAlpha(DockTest *pDock);
