/***************************************************************************************************
* Window Docking Library                                                                           *
*                                                           Copyright (C)2003 Alpha-II Productions *
***************************************************************************************************/

#include	"windows.h"
#include	"Types.h"
#include	"A2Dock.h"


//**************************************************************************************************
// Variables

//User32 functions -----------------------------
BOOL	__stdcall (*pGetWindowInfo)(HWND,PWINDOWINFO) = NULL;							//Win98
BOOL	__stdcall (*pEnumDisplayMonitors)(HDC,LPCRECT,MONITORENUMPROC,LPARAM) = NULL;	//Win98
void	__stdcall (*pGetLayeredWindowAttributes)(HWND,COLORREF*,BYTE*,DWORD*) = NULL;	//Win2k
BOOL	__stdcall (*pSetLayeredWindowAttributes)(HWND,COLORREF,BYTE,DWORD) = NULL;		//Win2k

static HINSTANCE	hUser = NULL;				//Handle to USER32.DLL
static RECT			adj;						//Amount to adjust real position from mouse coord


//**************************************************************************************************
// Function Prototypes

static BOOL __stdcall EnumThreadWndProc(HWND hwnd, LPARAM lParam);
static BOOL __stdcall EnumThreadWndProcA(HWND hwnd, LPARAM lParam);
static BOOL __stdcall MonitorEnumProc(HMONITOR hmonitor, HDC hdc, LPRECT pRect, LPARAM usr);
static b8 DockingPath(RECT *pRect, RECT aRect[], u32 num, u32 idx = 0);


////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions

//**************************************************************************************************
// Initialize Docking Library

void InitDock()
{
	if (hUser) return;

	//Connect to the user library --------------
	// These are functions that don't exist in all versions of Windows.  If we allow the
	// compiler to link to them at run-time, Winamp won't be able to load in_snes.dll under
	// Win95/98.

	hUser = LoadLibrary("USER32.DLL");
	if (hUser)
	{
		pGetWindowInfo =
			(BOOL (__stdcall *)(HWND,PWINDOWINFO))GetProcAddress(hUser,"GetWindowInfo");
		pEnumDisplayMonitors =
			(BOOL (__stdcall *)(HDC,LPCRECT,MONITORENUMPROC,LPARAM))GetProcAddress(hUser,"EnumDisplayMonitors");
		pGetLayeredWindowAttributes =
			(void (__stdcall *)(HWND,COLORREF*,BYTE*,DWORD*))GetProcAddress(hUser,"GetLayeredWindowAttributes");
		pSetLayeredWindowAttributes =
			(BOOL (__stdcall *)(HWND,COLORREF,BYTE,DWORD))GetProcAddress(hUser,"SetLayeredWindowAttributes");

		if (!pGetWindowInfo)
		{
			FreeLibrary(hUser);
			hUser = NULL;
			return;
		}
	}
}


//**************************************************************************************************
// Shutdown Docking Library

void ShutDock()
{
	if (hUser)
	{
		pGetWindowInfo = NULL;
		pEnumDisplayMonitors = NULL;
		pGetLayeredWindowAttributes = NULL;
		pSetLayeredWindowAttributes = NULL;

		FreeLibrary(hUser);						//Release USER32.DLL
		hUser = NULL;
	}
}


//**************************************************************************************************
// Is Form Docked to Window?

b8 __fastcall IsDocked(DockTest *pDock)
{
	RECT	rect,*pTest;
	u32		i;
	b8		sticky;


	//Check if docked against root window ------
	pDock->windows = 1;
	pTest = &pDock->rect[0];
	GetWindowRect(pDock->hRoot, pTest);			//Get rect of root window
	GetWindowRect(pDock->hWindow, &rect);		//Get rect of window to test
	sticky = 0;

	if (( (rect.left   == pTest->right || rect.right  == pTest->left) &&
		 ((rect.top    >= pTest->top   && rect.top    <  pTest->bottom) ||
		  (rect.bottom >  pTest->top   && rect.bottom <= pTest->bottom)) ) ||
		( (rect.top   == pTest->bottom || rect.bottom == pTest->top) &&
		 ((rect.left  >= pTest->left   && rect.left   <  pTest->right) ||
		  (rect.right >  pTest->left   && rect.right  <= pTest->right)) )) sticky = 1;

	if (!sticky) return 0;

	//Check if docked to any other window ------
	if (pGetWindowInfo)
	{
		EnumThreadWindows(pDock->thread, (int(__stdcall*)())&EnumThreadWndProc, (u32)pDock);
		sticky = 0;

		for (i=1; (u8)i<pDock->windows; i++)	//The cast to a u8 is because of a STUPID warning
		{										// bug in Borland C++.
			pTest = &pDock->rect[i];

			if (( (rect.left   == pTest->right || rect.right  == pTest->left) &&
				 ((rect.top    >= pTest->top   && rect.top    <  pTest->bottom) ||
				  (rect.bottom >  pTest->top   && rect.bottom <= pTest->bottom)) ) ||
				( (rect.top   == pTest->bottom || rect.bottom == pTest->top) &&
				 ((rect.left  >= pTest->left   && rect.left   <  pTest->right) ||
				  (rect.right >  pTest->left   && rect.right  <= pTest->right)) )) sticky = 1;

			if (sticky) return 0;
		}
	}

	return 1;
}


//**************************************************************************************************
// Is Form Docked to Root Window?

b8 __fastcall IsDockedPath(DockTest *pDock)
{
	RECT	rect;


	GetWindowRect(pDock->hRoot, &pDock->rect[0]);	//Get rect of root window
	pDock->windows = 1;
	if (pGetWindowInfo)							//Get rects of remaining windows
	{
		EnumThreadWindows(pDock->thread, (int(__stdcall*)())&EnumThreadWndProc, (u32)pDock);
	}

	GetWindowRect(pDock->hWindow, &rect);		//Get rect of window to test

	return DockingPath(&rect, pDock->rect, pDock->windows);
}


//**************************************************************************************************
// Prepare to Move Dialog

void __fastcall PrepareMove(DockTest *pDock, s32 x, s32 y)
{
	u8	win;


	GetWindowRect(pDock->hWindow, &adj);
	adj.top -= y; 						 		//Relative position of mouse cursor to dialog's
	adj.left -= x;								// screen position
	adj.bottom -= y;
	adj.right -= x;

	GetWindowRect(pDock->hRoot, &pDock->rect[0]);	//Get coordinates of root window
	pDock->windows = 1;

	if (pGetWindowInfo)							//Get coordinates for all windows created by process
		EnumThreadWindows(pDock->thread, (int (__stdcall*)())&EnumThreadWndProc, (u32)pDock);

	win = pDock->windows;
	if (pEnumDisplayMonitors)
		pEnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (u32)pDock);

	pDock->monitors = pDock->windows;
	pDock->windows = win;
}


//**************************************************************************************************
// Move Dialog

b8 __fastcall MoveDialog(RECT *pRect, DockTest *pDock, s32 x, s32 y)
{
	RECT 	rect;								//Rectangle used for docking test
	RECT	*pTest;								//-> rectangle to test against
	s32		snapTop,snapLeft;					//Coordinates after snapping to an edge
	s32		newTop,newLeft;						//New (final) coordinates of Control dialog
	s32		height,width;
	u32		sticky,snapped,i;


	sticky = 0;									//Assume Control dialog is not docked

	//Get dialog's logical position based on mouse cursor
	newTop	=	snapTop		=	rect.top	=	y + adj.top;
	newLeft	=	snapLeft	=	rect.left	=	x + adj.left;
								rect.bottom	=	y + adj.bottom;
								rect.right	=	x + adj.right;

	height = adj.bottom - adj.top;
	width = adj.right - adj.left;

	//==========================================
	//Test for docking against all windows
	//
	//Bits 0 and 1 of 'snapped' are used to flag if the control dialog has docked to the top/bottom
	//and left/right of the current window.  Bits 0 and 1 of 'sticky' are used if the dialog has
	//docked to any window.
	//
	//At the end of the function, 'sticky' will be 1 or 0, depending on if the control dialog is
	//docked to the root window.

	for (i=0; (u8)i<pDock->windows; i++)		//Iterate through all visible windows
	{
		pTest = &pDock->rect[i];				//-> current window's rectangle
		snapped = 0;

		//Test for docking on top or bottom ----
		//Before testing if the dialog is within snapping range on the top or bottom, make sure that
		//it isn't already docked to the top/bottom edge of another window and that some part of it
		//is inside the window on the left or right side.
		if (!(sticky&1) && rect.left<=pTest->right && rect.right>=pTest->left)
		{
			//If the bottom of the dialog is within ten pixels of the top edge of the window...
			if (abs(rect.bottom - pTest->top) < 10)
			{
				snapTop = pTest->top - height;
				snapped |= 1;
			}
			else
			if (abs(rect.top - pTest->bottom) < 10)	//top -> bottom
			{
				snapTop = pTest->bottom;
				snapped |= 1;
			}
		}

		//Test for docking on left or right ----
		if (!(sticky&2) && rect.top<=pTest->bottom && rect.bottom>=pTest->top)
		{
			if (abs(rect.right - pTest->left) < 10)	//right -> left
			{
				snapLeft = pTest->left - width;
				snapped |= 2;
			}
			else
			if (abs(rect.left - pTest->right) < 10)	//left -> right
			{
				snapLeft = pTest->right;
				snapped |= 2;
			}
		}

		//Test for alignment on top or bottom --
		//If the dialog just docked to the left/right edge of a window, and it isn't already docked
		//on the top/bottom edge of another window, then see if it's within range to be aligned with
		//the top/bottom edge of this window.  This code causes corners to snap together.
		if (!(sticky&1) && snapped==2)
		{
			if (abs(rect.top - pTest->top) < 10)  	 	//top -> top
			{
				snapTop = pTest->top;
				snapped |= 1;
			}
			else
			if (abs(rect.bottom - pTest->bottom) < 10)	//bottom -> bottom
			{
				snapTop = pTest->bottom - height;
				snapped |= 1;
			}
		}

		//Test for alignment on left or right --
		if (!(sticky&2) && snapped==1)
		{
			if (abs(rect.left - pTest->left) < 10)		//left -> left
			{
				snapLeft = pTest->left;
				snapped |= 2;
			}
			else
			if (abs(rect.right - pTest->right) < 10)	//right -> right
			{
				snapLeft = pTest->right - width;
				snapped |= 2;
			}
		}

		//Verify new position is good ----------
		//The dialog should only stick to the outside of windows, not the inside.
		if (snapped)
		{
			if (!((( snapLeft          >= pTest->left &&  snapLeft          <  pTest->right )  ||
				   ((snapLeft + width) >  pTest->left && (snapLeft + width) <= pTest->right )) &&
				  (( snapTop           >= pTest->top  &&  snapTop           <  pTest->bottom)  ||
				   ((snapTop + height) >  pTest->top  && (snapTop + height) <= pTest->bottom))))
			{
				if (snapped & 1) newTop = snapTop;
				if (snapped & 2) newLeft = snapLeft;
				if (!i) sticky |= 4;			//Set bit 2 if docked to main window (see below)
				sticky |= snapped;
			}
		}
	}

	//==========================================
	//Test for snapping against monitor edges
	//
	//A little different from the above loop, but the same basic concept.

	for (; (u8)i<pDock->monitors; i++)
	{
		pTest = &pDock->rect[i];
		snapped = 0;

		//Test for docking on top or bottom ----
		if (!(sticky&1) && rect.left<=pTest->right && rect.right>=pTest->left)
		{
			if (abs(rect.top - pTest->top) < 10)
			{
				snapTop = pTest->top;
				snapped |= 1;
			}
			else
			if (abs(rect.bottom - pTest->bottom) < 10)
			{
				snapTop = pTest->bottom - height;
				snapped |= 1;
			}
		}

		//Test for docking on left or right ----
		if (!(sticky&2) && rect.top<=pTest->bottom && rect.bottom>=pTest->top)
		{
			if (abs(rect.left - pTest->left) < 10)
			{
				snapLeft = pTest->left;
				snapped |= 2;
			}
			else
			if (abs(rect.right - pTest->right) < 10)
			{
				snapLeft = pTest->right - width;
				snapped |= 2;
			}
		}

		if (snapped)
		{
			//Same as above test, but reversed to verify dialog is inside rectangle
			if (((( snapLeft          >= pTest->left &&  snapLeft          <  pTest->right )  ||
				  ((snapLeft + width) >  pTest->left && (snapLeft + width) <= pTest->right )) &&
				 (( snapTop           >= pTest->top  &&  snapTop           <  pTest->bottom)  ||
				  ((snapTop + height) >  pTest->top  && (snapTop + height) <= pTest->bottom))))
			{
				if (snapped & 1) newTop = snapTop;
				if (snapped & 2) newLeft = snapLeft;
				sticky |= snapped;
			}
		}
	}

	//Set current position ---------------------
	pRect->top = newTop;
	pRect->left = newLeft;
	pRect->bottom = newTop + height;
	pRect->right = newLeft + width;

	return (sticky >> 2) != 0;					//Shift off temp bits and save main window flag
}


//**************************************************************************************************
// Get Alpha of All Windows

b8 __fastcall GetWindowAlpha(DockTest *pDock)
{
	if (!pGetLayeredWindowAttributes) return 0;

	pDock->windows = 0;
	pDock->monitors = 0;

	EnumThreadWindows(pDock->thread, (int (__stdcall*)())&EnumThreadWndProcA, (u32)pDock);

	return 1;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
// Private Functions

//**************************************************************************************************
// Enumerate Windows Created by a Thread
//
// Called by EnumThreadWindows once for each window created by a thread.  Used to build a list of
// all visible windows associated with an application.

BOOL __stdcall EnumThreadWndProc(HWND hwnd, LPARAM usr)
{
	WINDOWINFO	winInfo;
	DockTest	*p;


	p = (DockTest*)usr;

	if (hwnd == p->hWindow || hwnd == p->hRoot)	//We don't need to store our own or the root
		return 1;								// window's coordinates

	if (p->windows < DOCK_MAXWIN)				//Make sure there's room in the array
	{
		winInfo.cbSize = sizeof(WINDOWINFO);
		pGetWindowInfo(hwnd, &winInfo);

		if (winInfo.dwStyle & WS_VISIBLE)		//If the window's visible, copy its coordinates
			memcpy(&p->rect[p->windows++], &winInfo.rcWindow, sizeof(RECT));

		return 1;
	}

	return 0;									//Tell Windows we're done accepting handles
}


BOOL __stdcall EnumThreadWndProcA(HWND hwnd, LPARAM usr)
{
	WINDOWINFO	winInfo;
	DockTest	*p;
	DWORD		flags;
	u8			blend;


	p = (DockTest*)usr;

	if (hwnd == p->hWindow) return 1;			//We don't need to store our own alpha

	if (p->windows < DOCK_MAXWIN)				//Make sure there's room in the array
	{
		winInfo.cbSize = sizeof(WINDOWINFO);
		pGetWindowInfo(hwnd, &winInfo);

		if (winInfo.dwStyle & WS_VISIBLE)
		{
			p->alpha[p->windows] = 255;

			if (winInfo.dwExStyle & WS_EX_LAYERED)
			{
				pGetLayeredWindowAttributes(hwnd, NULL, &blend, &flags);

				if (flags & LWA_ALPHA) p->alpha[p->windows] = blend;
			}

			p->windows++;
		}

		return 1;
	}

	return 0;									//Tell Windows we're done accepting handles
}


//**************************************************************************************************
// Enumerate Monitors Comprising Display
//
// Called by EnumDisplayMonitors once for each monitor used by the desktop.

BOOL __stdcall MonitorEnumProc(HMONITOR hmonitor, HDC hdc, LPRECT pRect, LPARAM usr)
{
	DockTest	*p;


	p = (DockTest*)usr;

	if (p->windows < DOCK_MAXWIN)				//Make sure there's room in the array
	{
		memcpy(&p->rect[p->windows++], pRect, sizeof(RECT));
		return 1;
	}

	return 0;									//Tell Windows we're done accepting handles
}


//**************************************************************************************************
// Follow a Window Docking Path to the Main Window
//
// A recursive function that determines if a window is docked to the main window either directly or
// indirectly through other windows.
//
// In:
//    pRect -> Rectangle to test for docking
//    aRect -> Array of rectangles to test against
//    num    = Number of rectangles in array
//    idx    = Used for recursion, don't set
//
// Out:
//    true, if window is somehow connected to the main window

b8 DockingPath(RECT *pRect, RECT aRect[], u32 num, u32 idx)
{
	RECT	*pTest;
	b8		sticky,c;


	//Check if docked to the main window -------
	pTest = &aRect[0];
	sticky = 0;

	if ( (pRect->left   == pTest->right || pRect->right  == pTest->left) &&
		((pRect->top    >= pTest->top   && pRect->top    <= pTest->bottom) ||
		 (pRect->bottom >= pTest->top   && pRect->bottom <= pTest->bottom)) ) sticky = 1;
	else
	if ( (pRect->top   == pTest->bottom || pRect->bottom == pTest->top) &&
		((pRect->left  >= pTest->left   && pRect->left   <= pTest->right) ||
		 (pRect->right >= pTest->left   && pRect->right  <= pTest->right)) ) sticky = 1;

	if (sticky) return 1;

	//Check if docked to other windows ---------
	for (++idx;idx<num;idx++)
	{
		pTest = &aRect[idx];
		sticky = 0;

		if ( (pRect->left   == pTest->right || pRect->right  == pTest->left) &&
			((pRect->top    >= pTest->top   && pRect->top    <= pTest->bottom) ||
			 (pRect->bottom >= pTest->top   && pRect->bottom <= pTest->bottom)) ) sticky = 1;
		else
		if ( (pRect->top   == pTest->bottom || pRect->bottom == pTest->top) &&
			((pRect->left  >= pTest->left   && pRect->left   <= pTest->right) ||
			 (pRect->right >= pTest->left   && pRect->right  <= pTest->right)) ) sticky = 1;

		if (sticky)
		{
			c = DockingPath(pTest, aRect, num, idx);
			if (c) return 1;
		}
	}

	return 0;
}
