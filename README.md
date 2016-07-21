# limitations
the controller program doesn't have touchscreen yet

i don't hook the IR service, so no zl/zr or c-stick

## credits and stuff
Memory patching code taken from BootNTR

Steveice10's buildtools used to build input_proc

Shinyquagsire for inspiring me to start this with his writeup, then helping me along the way


## note
buildtools requires a 1 line patch for the input redirector cia to work best:
replace MemoryType: Application with MemoryType: Base in template.rsf line 70
