SNESAmp v3.1 Source Code
______________________________________________________________________________

This confusing mass of source code is the base material for SNESAmp, a Winamp
input plug-in that emulates the Sony SPC700 and DSP found the the Super
Nintendo Entertainment System(tm).

With version 2.5, SNESAmp has been rewritten under Borland C++Builder v5.0.
Calling the Windows API directly made a small plug-in, but the code was
getting so hard to read that I gave up on it.  Using Borland's VCL also gives
me options that would've been a nightmare to code otherwise (international
support being one of them).

With version 2.99 I rewrote much of the code, moved reusable functions into
separate files, cleaned up interfaces between classes, and tried to document
exactly what it is I'm trying to do.  The result is a great improvement over
2.5, but alas still a disappointment to me as I know it could be better.

The reason I bit the bullet and decided to release this source to the public
is because I found very little information on programming plug-ins for
Winamp 2.x.  With Nullsoft pushing Winamp3, I may be a bit late in my efforts
to help other plug-in developers, but here it is anyway.  I'm giving it out in
hopes that someone will find it useful, whether they're intersted in video
game music or not.

Note:  I set my tab stops to 4 characters.


Main DLL
________

The executable part of the program is composed of six parts:

\
 Types.h  - Type redefinitions for better cross platform support
 A2Date   - A class used by the ID666 class for managing dates
 A2Dock   - Functions for creating snapping/docking windows
 A2Math   - A few simple assembly math functions
 A2Str    - Some custom string routines

\SNES\
 ID666    - A class for reading, editing, and writing ID666 tags

\SNES\SNESAmp\
 Main     - Main program
 Winamp   - All the exported functions for Winamp
 About    - About SNESAmp dialog
 Config   - Configuration dialog
 Control  - Control dialog
 Tag      - ID666 editor dialog

\SNES\SNESAmp\Help\
 Richtext files for context sensitive help (in_snes.hlp)

\SNES\SNESAmp\Manual\
 Richtext files for SNESAmp manual (SNESAmp.hlp)

The language resource DLL's are not included.


Help
____

The rich text files and help project are found in the Help subdirectory.

Understand that the source to the .HLP file isn't necesssary to complile
SNESAmp (in fact, the .HLP file itself isn't necessary to SNESAmp's
execution).  It's included because, once again, I was able to find little
information, and thought someone might be able to use it.

Surprisingly, the help inside the Microsoft Help Workshop is very good.
(Apparently there's a ~few~ people at MS that have a clue.)  It includes
examples and step-by-step procedures, as well as a list of all RTF
formatting codes used in the help files.


Extended ID666 format
_____________________

A description of the extended ID666 format can be found at:
http://www.alpha-ii.com/Info/SPCFormat.txt

See ID666.cpp for an example of reading extended information.


Thanks
______

Datschge-
 Intensive beta testing, suggestions, building the SNESAmp site, and being
 the driving force behind the SNESAmp plug-in

Marp-
 SNESAmp wouldn't have gotten off the ground, if it hadn't have been for
 the YMAMP source.

Matt Conte-
 I learned some cool tricks from the Nosefart source

Safai Ma-
 I learned more cool tricks from the Album List source

...and all the people that beta tested and/or e-mailed me with suggestions
or bugs


Using the source in your own programs
_____________________________________

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


SNESAmp is copyright (C)2000-2004 Alpha-II Productions (www.alpha-ii.com)

"SNES" and "Super Nintendo Entertainment System" are registered trademarks of
Nintendo (www.nintendo.com)
