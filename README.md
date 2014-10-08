The Wall
=======
Johan Berntsson, 2014

A game for the Commodore 64 written in C using the cc65 cross compiler

Building and running
-----

You need to install the cc65 cross compiler and the vice C64 emulator.

Add these lines to .bashrc (adjust as needed)

  export CC65_HOME=~/commodore/cc65
  export PATH=${PATH}:${CC65_HOME}/bin

Type make to compile and create the thewall.d64 image. If there are no
errors make will also start vice with the new d64 image preloaded automatically.
