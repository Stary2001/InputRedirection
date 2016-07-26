.global read_input
.global read_input_sz

read_input:
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

@buttons init
ldr r0, =0x10df20
ldr r1, =0xFFF
bl .init

@touch init
ldr r0, =0x10df24
ldr r1, =0x2000000
bl .init

@cpad init
ldr r0, =0x10df28
ldr r1, =0x800800
bl .init

@buttons copy
ldr r0, =0x10df20
ldr r1, =0xFFF
ldr r2, =0x1ec46000
ldr r3, =0x10df00
bl .copy

@touch copy
ldr r0, =0x10df24
ldr r1, =0x2000000
ldr r2, =0x10df10
ldr r3, =0x10df08
bl .copy

@cpad copy
ldr r0, =0x10df28
ldr r1, =0x800800
ldr r2, =0x10df14
ldr r3, =0x10df0c
bl .copy

.ret:
LDMFD   SP!, {R4-R6,PC}

.init:
ldr r2, [r0]
cmp r2, #0
streq r1, [r0]
mov pc, r14

.copy:
ldr r4, [r0]
cmp r4, r1
ldreq r4, [r2]
str r4, [r3]
mov pc, r14

.LTORG # assembles literal pool

read_input_sz:
.4byte .-read_input