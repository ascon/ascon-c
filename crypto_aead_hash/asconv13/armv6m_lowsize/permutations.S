	.arch armv6-m
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 2
	.eabi_attribute 34, 0
	.eabi_attribute 18, 4
	.file	"permutations.c"
	.text
	.align	1
	.p2align 2,,3
	.global	P
	.syntax unified
	.code	16
	.thumb_func
	.fpu softvfp
	.type	P, %function
P:
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	movs	r3, r1
	movs	r2, #12
	adds	r1, r1, #3
	subs	r3, r2, r3
	lsls	r1, r1, #4
	push	{r4, r5, r6, r7, lr}
	orrs	r1, r3
	mov	lr, fp
	mov	r7, r10
	mov	r6, r9
	mov	r5, r8
	ldr	r3, [r0]
	push	{r5, r6, r7, lr}
	mov	r9, r3
	ldr	r3, [r0, #4]
	sub	sp, sp, #12
	mov	r8, r3
	ldr	r3, [r0, #20]
	ldr	r2, [r0, #28]
	mov	ip, r3
	ldr	r3, [r0, #32]
	ldr	r6, [r0, #12]
	mov	r10, r3
	ldr	r7, [r0, #8]
	ldr	r5, [r0, #16]
	ldr	r4, [r0, #24]
	str	r0, [sp, #4]
	ldr	r0, [r0, #36]
	mov	r3, r9
	mov	fp, r2
	mov	r9, r6
	mov	r2, r10
	mov	r10, r0
	.syntax divided
	rbegin_49:;
	eor r5, r5, r1
	push  {r1}
	eor r3, r3, r2
	eor r2, r2, r4
	eor r5, r5, r7
	mvn r6, r3
	orr r6, r6, r2
	movs r1, r5
	bic r1, r1, r7
	eor r3, r3, r1
	mvn r1, r2
	orr r1, r1, r4
	eor r5, r5, r1
	movs r1, r7
	bic r1, r1, r3
	eor r2, r2, r1
	movs r1, r4
	and r1, r1, r5
	eor r1, r7, r1
	eor r6, r4, r6
	eor r6, r6, r5
	eor r1, r1, r3
	eor r3, r3, r2
	movs r7, r8
	movs r4, r9
	movs r8, r5
	movs r9, r3
	movs r3, ip
	movs r5, fp
	movs r0, r10
	movs ip, r6
	movs fp, r2
	eor r7, r7, r0
	eor r0, r0, r5
	eor r3, r3, r4
	mvn r6, r7
	orr r6, r6, r0
	movs r2, r3
	bic r2, r2, r4
	eor r7, r7, r2
	mvn r2, r0
	orr r2, r2, r5
	eor r3, r3, r2
	movs r2, r4
	bic r2, r2, r7
	eor r0, r0, r2
	movs r2, r5
	and r2, r2, r3
	eor r4, r4, r2
	eor r5, r5, r6
	eor r5, r5, r3
	eor r4, r4, r7
	eor r7, r7, r0
	movs r10, r5
	movs r5, r8
	movs r8, r7
	lsr r2, r3, #6
	lsl r7, r5, #26
	lsr r6, r5, #6
	eor r2, r2, r7
	lsl r7, r3, #26
	eor r6, r6, r7
	lsr r7, r3, #1
	eor r2, r2, r7
	lsl r7, r5, #31
	eor r2, r2, r7
	lsr r7, r5, #1
	eor r6, r6, r7
	lsl r7, r3, #31
	eor r6, r6, r7
	eor r3, r3, r2
	eor r5, r5, r6
	lsl r2, r4, #3
	lsr r7, r1, #29
	lsl r6, r1, #3
	eor r2, r2, r7
	lsr r7, r4, #29
	eor r6, r6, r7
	lsl r7, r4, #25
	eor r2, r2, r7
	lsr r7, r1, #7
	eor r2, r2, r7
	lsl r7, r1, #25
	eor r6, r6, r7
	lsr r7, r4, #7
	eor r6, r6, r7
	eor r4, r4, r2
	eor r1, r1, r6
	movs r2, fp
	movs fp, r1
	lsl r1, r0, #23
	lsr r7, r2, #9
	lsl r6, r2, #23
	eor r1, r1, r7
	lsr r7, r0, #9
	eor r6, r6, r7
	lsr r7, r0, #7
	eor r1, r1, r7
	lsl r7, r2, #25
	eor r1, r1, r7
	lsr r7, r2, #7
	eor r6, r6, r7
	lsl r7, r0, #25
	eor r6, r6, r7
	eor r0, r0, r1
	eor r2, r2, r6
	movs r7, fp
	movs r1, r10
	movs r10, r0
	movs fp, r4
	movs r4, ip
	movs ip, r3
	lsr r0, r1, #17
	lsl r3, r4, #15
	lsr r6, r4, #17
	eor r0, r0, r3
	lsl r3, r1, #15
	eor r6, r6, r3
	lsr r3, r1, #10
	eor r0, r0, r3
	lsl r3, r4, #22
	eor r0, r0, r3
	lsr r3, r4, #10
	eor r6, r6, r3
	lsl r3, r1, #22
	eor r6, r6, r3
	eor r1, r1, r0
	eor r4, r4, r6
	movs r6, r8
	movs r3, r9
	movs r8, r2
	movs r9, fp
	movs fp, r1
	lsr r2, r6, #28
	lsl r1, r3, #4
	lsr r0, r3, #28
	eor r2, r2, r1
	lsl r1, r6, #4
	eor r0, r0, r1
	lsr r1, r6, #19
	eor r2, r2, r1
	lsl r1, r3, #13
	eor r2, r2, r1
	lsr r1, r3, #19
	eor r0, r0, r1
	lsl r1, r6, #13
	eor r0, r0, r1
	pop  {r1}
	eor r6, r6, r2
	eor r3, r3, r0
	movs r2, r8
	movs r8, r6
	sub r1, r1, #15
	cmp r1, #60
	beq rend_49
	b rbegin_49
	rend_49:;
	
	.thumb
	.syntax unified
	ldr	r1, [sp, #4]
	mov	r0, r10
	str	r3, [r1]
	mov	r3, r8
	mov	r10, r2
	str	r3, [r1, #4]
	mov	r3, ip
	mov	r2, fp
	mov	r6, r9
	str	r3, [r1, #20]
	mov	r3, r10
	str	r7, [r1, #8]
	str	r6, [r1, #12]
	str	r5, [r1, #16]
	str	r4, [r1, #24]
	str	r2, [r1, #28]
	str	r3, [r1, #32]
	str	r0, [r1, #36]
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7}
	mov	fp, r7
	mov	r10, r6
	mov	r9, r5
	mov	r8, r4
	pop	{r4, r5, r6, r7, pc}
	.size	P, .-P
	.ident	"GCC: (15:10.3-2021.07-4) 10.3.1 20210621 (release)"
