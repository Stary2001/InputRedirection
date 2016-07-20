.global hid_inject
.global hid_inject_end

hid_inject:
STMFD   SP!, {R4-R6,LR}
MOV     R5, R1
MRC     p15, 0, R4,c13,c0, 3
MOV     R1, #0x10000
STR     R1, [R4,#0x80]!
LDR     R0, [R0]
SVC     0x32
ANDS    R1, R0, #0x80000000
BMI     .ret
push {r3}
ldr r3, =0x10df08
LDRD    R0, [R4,#8]
STRD    R0, [R3, #8]
LDRD	r0, [R3]
STRD	r0,	[R5]
LDR     R0, [R4,#4]
pop {r3}
.ret:
LDMFD   SP!, {R4-R6,PC}

hid_inject_end: