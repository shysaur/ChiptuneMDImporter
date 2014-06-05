sible\cell\row
\intbl\cell +\cell Control dialog tries to work with transparency plug-ins\cell\row
\intbl\cell +\cell Tag editor and control dialog remember their absolute screen positions\cell\row
\intbl\cell +\cell Added ability to save the mixing level in the tag\cell\row
\intbl\cell +\cell Added ability to disable the echo in the configuration\cell\row
\intbl\cell +\cell Changed output threshold to +/-3.00dB\cell\row
\intbl\cell -\cell Added version information to LRM's and improved LRM detection and loading (LRM's from previous versions of SNESAmp won't get loaded and crash)\cell\row
\intbl\cell -\cell Fixed another return value error in the message handler (titlebar text scrolls)\cell\row
\intbl\cell -\cell Visualization works with 32-bit samples\cell\row
\intbl\cell -\cell Sometimes the mixing level in the config was displayed incorrectly\cell\row
\intbl\cell -\cell Forgot to save the date in the tag\cell\row
\intbl\cell -\cell Tag editor doesn't crash when the date is corrupted\cell\row
\intbl\cell -\cell Forgot to apply the mute flags from the tag\cell\row
\intbl\cell -\cell Fixed the time calculation so the buttons in the tag editor get the correct time when a song's speed has been adjusted via the control dialog\cell\row
\intbl\cell -\cell Fixing a mixing level problem when auto attenuation was turned off\cell\row
\intbl\cell  \cell \par SNESAPU.DLL v0.98\cell\row
\intbl\cell +\cell Added micro ramping to the channel volumes in the 32-bit sample routine (reduces zipper effects and pops in some songs)\cell\row
\intbl\cell -\cell Added a 17-bit clamp to the sample decompression (fixes Hiouden, but the wind in CT isn't quite right)\cell\row
\intbl\cell -\cell Added linear interpolation to the FIR filter (fixes some songs that turned to noise)\cell\row
\intbl\cell -\cell ENVX always gets updated, regardless of ADSR/GAIN flag (fixes Clue)\cell\row
\intbl\cell -\cell Added code to the speed hack to try and isolate small polling loops (fixes the tempo problems some games were having)\cell\row
\intbl\cell -\cell Changed the loader code, again, to copy extra RAM from the correct place (fixes incorrect sounds in Tales of Phantasia)\cell\row
\intbl\cell -\cell Fixed a lame bug when clearing the in-ports (fixes Dragon Quest songs stopping half way through)\cell\row
}

{\li360\par 2.5  2.10.01 Remastered\par}
{\cellx720\cellx870\cellx7200
\intbl\cell +\cell Rewrote the entire ID666 editor, and pieces of the other code, after a Win2k install went bad\cell\row
\intbl\cell +\cell Added support for the additional time fields in the XID6 tag\cell\row
\intbl\cell +\cell Playback switches to fadeout if timer is enabled after song would've ended\cell\row
\intbl\cell -\cell Fixed the way samples were sent to Winamp, so DSP plug-ins and EQ work again\cell\row
\intbl\cell -\cell Decible value on threshold control was incorrect\cell\row
\intbl\cell -\cell Fixed FIR filter detection in the ID666 editor\cell\row
\intbl\cell -\cell Noise clock speed is displayed correctly\cell\row
\intbl\cell -\cell Fixed the multiple instances bug when opening SPC's from Explorer\cell\row
\intbl\cell -\cell Control dialog will stay visible if Winamp is left of the primary monitor\cell\row
\intbl\cell -\cell SNESAPU.DLL can be installed in the Plugins folder, if the System folder isn't available\cell\row
\intbl\cell -\cell SNESAPU was accidentally getting linked twice\cell\row
\intbl\cell  \cell \par SNESAPU.DLL v0.95,0.96\cell\row
\intbl\cell +\cell Added 4-point Gaussian interpolation\cell\row
\intbl\cell +\cell Made fast seeking faster, and normal seeking slower and more accurate (now you can tell a difference between the two)\cell\row
\intbl\cell +\cell Improved the method of reversing stereo channels\cell\row
\intbl\cell +\cell Made more internal optimizations and changes\cell\row
\intbl\cell -\cell Pushed some data through the SNES and rewrote the ADPCM routine (sample decompression is virtually perfect, except for Hiouden)\cell\row
\intbl\cell -\cell Forgot to disable pitch modulation after a PMON flag was cleared (Asteri