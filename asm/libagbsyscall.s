	.include "asm/macros/function.inc"
	.syntax unified

	.text

	thumb_func_start CpuFastSet
CpuFastSet:
	svc #0xc
	bx lr

	thumb_func_start CpuSet
CpuSet:
	svc #0xb
	bx lr

	thumb_func_start Div
Div:
	svc #6
	bx lr

	thumb_func_start Mod
Mod:
	svc #6
	adds r0, r1, #0
	bx lr

	thumb_func_start SoftReset
SoftReset:
	ldr r3, =0x04000208 @ REG_IME
	movs r2, #0
	strb r2, [r3]
	ldr r1, =0x03007F00
	mov sp, r1
	svc #1
	svc #0
	movs r0, r0
	.pool
