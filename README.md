The Wall
=======

This is a game for the Commodore 64 written in C using the cc65 cross compiler

![Screenshot](https://github.com/johanberntsson/thewall/blob/master/thewall.png)

This repository includes the a binary called thewall.d64 which can be used in an C64 emulator such as [Vice](http://vice-emu.sourceforge.net/) or any other emulator that can handle the d64 virtual floppy disc format.

The 2020 version gives bonus points if you smash many bricks at the same time, allows cursor keys, and includes better instructions.

Building
-----

You need to install the cc65 cross compiler and the vice C64 emulator.

Add these lines to .bashrc (adjust as needed)

	export CC65_HOME=~/commodore/cc65
	export PATH=${PATH}:${CC65_HOME}/bin

Type make to compile and create the thewall.d64 image. If there are no
errors make will also start vice with the new d64 image preloaded.
