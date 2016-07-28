# limitations
i don't hook the IR service, so no zl/zr or c-stick

## credits and stuff
Memory patching code taken from BootNTR

Steveice10's buildtools used to build input_proc

Shinyquagsire for inspiring me to start this with his writeup, then helping me along the way

## how do i use this
Install the input_proc cia on your 3ds, then run the injector. this will do the thing.

use one of these input programs to do thing:
[SDL for linux](https://github.com/Stary2001/InputClient-SDL)
[XInput for windows](https://github.com/Kazo/InputRedirectionClient)

## note
buildtools requires a 1 line patch for the input redirector cia to work best:
replace MemoryType: Application with MemoryType: Base in template.rsf line 70
