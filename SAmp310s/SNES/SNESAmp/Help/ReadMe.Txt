SNESAmp v3.1 Translation
=============================================================================

I only ask that you translate the dialog text, but you may also translate the
help files.

The RTF files used for creating the help are not standard richtext format,
thanks to Microsoft.  If you want to translate the help files, you must use
a regular text editor.

I've included a file called rtf.syn.  This is a syntax highlighting file for
TextPad (www.textpad.com), which could make editing the files easier.

Help files (.HLP) are created by Help Workshop.  This program comes with
Visual Studio and C++Builder, and may come with other development packages.
You only need Help Workshop if you want to compile the RTF files yourself.


Files you edit:

Dialog.Txt         - Text displayed in the dialog boxes

Help\Config.rtf    - Context sensitive help for the Configuration dialog
Help\Control.rtf   - Context sensitive help for the Control dialog
Help\ID666.rtf     - Context sensitive help for the ID666 editor

Manual\Config.rtf  - Help for the Configuration dialog
Manual\Control.rtf - Help for the Control dialog
Manual\ID666.rtf   - Help for the ID666 editor
Manual\ReadMe.rtf  - The main help file
Manual\SNESAmp.cnt - Contents file


Files you don't edit:

Help\in_snes.hpj   - Help Workshop project file
Manual\SNESAmp.hpj - Help Workshop project file
HelpID.h           - Needed for compiling the help
ReadMe.Txt         - This file
rtf.syn            - Syntax hightlighting file (used by TextPad, if you want)
