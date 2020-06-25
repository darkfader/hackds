	//.macro typedef a b c d e
	//.endm

#define SDK_ASM
#include "C:\NitroSDK\include\nitro\hw\ARM7\mmap_global.h"
#include "C:\NitroSDK\include\nitro\hw\ARM7\ioreg.h"

#define ARM9_ENTRY_ORIGINAL				0x02004800
#define	ARM7_ENTRY_ORIGINAL				0x02300000
#define ARM9_MAIN						0x02004B00
#define ARM9_MAIN_LOOP					0x02004B68
//#define ARM9_MAIN_BL						0x02004B6C

	.code	32

//////////////////////////////////////////////////////////////////////////////
// text
//////////////////////////////////////////////////////////////////////////////

	.section	.text


	.global _start
_start:

@	ldr		r8, =_ram_load
@	ldr		r9, =_ram_addr
@	ldr		r11, =_ram_size
@1:
@	ldrh	r12, [r8], #2
@	strh	r12, [r9], #2
@	subs	r11, r11, #2
@	bhi		1b
@
@	ldr		r12, =_ram_addr
@	mov		pc, r12
@
@	.pool
@
@//////////////////////////////////////////////////////////////////////////////
@// text2
@//////////////////////////////////////////////////////////////////////////////
@
@	.section	.text2

	bl		DisableInterrupts


	ldr		r10, =HW_CTRDG_ROM + 0x1F0
	ldr		r0, =0xAA55
	strh	r0, [r10, #0x02]


@	bl		PatchJump
	b		Init

@@	ldr		r1, =0x0207BF5C
@@	mov		r0, #' '
@@	strb	r0, [r1]
@@	ldr		r12, =ARM7_ENTRY_ORIGINAL
@@	mov		pc, r12

1:
	b		1b

//////////////////////////////////////////////////////////////////////////////
// PatchJump
//////////////////////////////////////////////////////////////////////////////

PatchJump:
	ldr		r2, =MainLoopJump
	ldr		r3, =ARM9_MAIN_LOOP
	ldr		r0, [r2], #4
	str		r0, [r3], #4
	ldr		r0, [r2], #4
	str		r0, [r3], #4
	mov		pc, lr

	.pool

MainLoopJump:
	ldr		pc, =MainLoopReplacement
	.pool

//////////////////////////////////////////////////////////////////////////////
// MainLoopReplacement
//////////////////////////////////////////////////////////////////////////////

MainLoopReplacement:
	bl		DisableInterrupts
	bl		DisableProtectionUnit
	bl		ARM9_GBA
@	//ldr		r1, =HW_CTRDG_ROM
@//1:
@	//add		r0, r0, #1
@	//strh	r0, [r1]
@	//b		1b		//MainLoopReplacement

Init:
	ldr		r11, =Commands

	ldr		r10, =HW_CTRDG_ROM + 0x1F0	// communication area
	mov		r0, #0
	strh	r0, [r10, #0x00]

Idle:
	ldrh	r3, [r10, #0x08]
	ldrh	r4, [r10, #0x0A]
	orr		r12, r3, r4, LSL #16		// r12 = address

	ldrh	r0, [r10, #0x00]
	add		r0, r11, r0, LSL #2
	ldr		pc, [r0]

Commands:
	.word	Idle		// 0
	.word	Read8		// 1
	.word	Read16		// 2
	.word	Read32		// 3
	.word	Write8		// 4
	.word	Write16		// 5
	.word	Write32		// 6
	.word	Call		// 7
	.word	SWI			// 8

Read8:
	ldrb	r0, [r12]
	strh	r0, [r10, #0x0C]
	b		Init

Read16:
	ldrh	r0, [r12]
	strh	r0, [r10, #0x0C]
	b		Init

Read32:
	ldr		r0, [r12]
	strh	r0, [r10, #0x0C]
	mov		r0, r0, LSR #16
	strh	r0, [r10, #0x0E]
	b		Init

Write8:
	ldrh	r0, [r10, #0x0C]
	strb	r0, [r12]
	b		Init

Write16:
	ldrh	r0, [r10, #0x0C]
	strh	r0, [r12]
	b		Init

Write32:
	ldrh	r0, [r10, #0x0C]
	ldrh	r1, [r10, #0x0E]
	orr		r0, r0, r1, LSL #16
	str		r0, [r12]
	b		Init

Call:
	ldr		sp, =0x02001000
	ldmfd	sp, {r0-r11}
	blx		r12
	stmfd	sp, {r0-r11}
	b		Init

/*
SoftReset:				swi		0	00
SoftResetExram:			swi		0	00
SoftResetRom:			swi		0	00
RegisterRamReset:		swi		1	01
IntrWait:				swi		4	04
VBlankIntrWait:			swi		5	05
Div:					swi		6	06
DivArm:					swi		7	07
DivRem:					swi		6	06
DivRemArm:				swi		7	07
Sqrt:					swi		8	08
ArcTan:					swi		9	09
ArcTan2:				swi		10	0A
CpuSet:					swi		11	0B
CpuFastSet:				swi		12	0C
BgAffineSet:			swi		14	0E
ObjAffineSet:			swi		15	0F
BitUnPack:				swi		16	10
LZ77UnCompWram:			swi		17	11
LZ77UnCompVram:			swi		18	12
HuffUnComp:				swi		19	13
RLUnCompWram:			swi		20	14
RLUnCompVram:			swi		21	15
Diff8bitUnFilterWram:	swi		22	16
Diff8bitUnFilterVram:	swi		23	17
Diff16bitUnFilter:		swi		24	18
SoundBiasChange:		swi		25	19
SoundBiasReset:			swi		25	19
SoundBiasSet:			swi		25	19
SoundDriverInit:		swi		26	1A
SoundDriverMode:		swi		27	1B
SoundDriverMain:		swi		28	1C
SoundDriverVSync:		swi		29	1D
SoundChannelClear:		swi		30	1E
MidiKey2Freq:			swi		31	1F
MusicPlayerOpen:		swi		32	20
MusicPlayerStart:		swi		33	21
MusicPlayerStop:		swi		34	22
MusicPlayerContinue:	swi		35	23
MusicPlayerFadeOut:		swi		36	24
MultiBoot:				swi		37	25
SoundDriverVSyncOff:	swi		40	28
SoundDriverVSyncOn:		swi		41	29





		.END



ARM7 DS

.text:00000000 SVC_SoftReset
.text:00000000                 SWI     0
.text:00000004 SVC_WaitByLoop
.text:00000004                 SWI     3
.text:00000008 SVC_WaitIntr
.text:0000000A                 SWI     4
.text:0000000E SVC_WaitVBlankIntr
.text:00000010                 SWI     5
.text:00000014 SVC_Halt
.text:00000014                 SWI     6
.text:00000018 SVC_Sleep
.text:00000034                 SWI     7
.text:0000004A SVC_ChangeSoundBias
.text:0000004A                 SWI     8
.text:0000004E SVC_SetSoundBias
.text:00000052                 SWI     8
.text:00000056 SVC_ResetSoundBias
.text:0000005A                 SWI     8
.text:0000005E SVC_Div
.text:0000005E                 SWI     9
.text:00000062 SVC_DivRem
.text:00000062                 SWI     9
.text:00000068 SVC_CpuSet
.text:00000068                 SWI     0xB
.text:0000006C SVC_CpuSetFast
.text:0000006C                 SWI     0xC
.text:00000070 SVC_Sqrt
.text:00000070                 SWI     0xD
.text:00000074 SVC_GetCRC16
.text:00000074                 SWI     0xE
.text:00000078 SVC_UnPackBits
.text:00000078                 SWI     0x10
.text:0000007C SVC_UnCompLZ77Byte
.text:0000007C                 SWI     0x11
.text:00000080 SVC_UnCompLZ77Short
.text:00000080                 SWI     0x12
.text:00000084 SVC_UnCompHuffman
.text:00000084                 SWI     0x13
.text:00000088 SVC_UnCompRLByte
.text:00000088                 SWI     0x14
.text:0000008C SVC_UnCompRLShort
.text:0000008C                 SWI     0x15
.text:00000090 SVC_GetSinTable
.text:00000090                 SWI     0x1A
.text:00000094 SVC_GetPitchTable
.text:00000094                 SWI     0x1B
.text:00000098 SVC_GetVolumeTable
.text:00000098                 SWI     0x1C

01		hangs
02		hangs
0A		hangs
0F		something		returns r3=0x027FFFE0
16		hangs
17		hangs
18		hangs
19		hangs
1D		something
1E		hangs
1F		nothing happens
20		hangs
21		hangs
22		hangs
*/

SWI:
	ldr		sp, =0x02001000
	ldmia	sp, {r0-r11}
	swi		0x1F << 16
	//MRS     R0, CPSR
	//mrc p15, 0, r0, c1, c0
	ldr		sp, =0x02001000
	stmia	sp, {r0-r11}
	b		Init

//////////////////////////////////////////////////////////////////////////////
// ARM9_GBA
//////////////////////////////////////////////////////////////////////////////

ARM9_GBA:
@	ldr		r1, =REG_EXMEMCNT_ADDR
@	ldrh	r0, [r1]
@	and		r0, r0, #~REG_MI_EXMEMCNT_CP_MASK
@	strh	r0, [r1]
	mov		pc, lr

//////////////////////////////////////////////////////////////////////////////
// DisableInterrupts
//////////////////////////////////////////////////////////////////////////////

DisableInterrupts:
	// disable IRQ/FIQ interrupts
	mrs		r0, CPSR
	orr		r0, r0, #0xC0
	msr		CPSR_fsxc, r0
	mov		pc, lr

//////////////////////////////////////////////////////////////////////////////
// DisableProtectionUnit
//////////////////////////////////////////////////////////////////////////////

DisableProtectionUnit:
	mrc		p15, 0, r0, c1, c0, 0		// read control register
	and		r0, r0, #~1		// HW_C1_PROTECT_UNIT_ENABLE
	mcr		p15, 0, r0, c1, c0, 0		// write control register
	mov		pc, lr


	//.section	.text

