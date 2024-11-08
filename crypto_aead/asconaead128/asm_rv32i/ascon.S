#include "api.h"

## REGISTER ALLOCATION
#define t0h t3
#define t0l t4
#define t1h t5
#define t1l t6
#define x0h s0
#define x0l s1
#define x1h s2
#define x1l s3
#define x2h s4
#define x2l s5
#define x3h s6
#define x3l s7
#define x4h s8
#define x4l s9
#define k0h s10
#define k0l s11
#define k1h a5
#define k1l a6

## OVERLAPPING REGISTER ALLOCATION
#define optr a0
#define iptr a3
#define ilen a4
#define mode a7

## STACK FRAME LAYOUT
##     +-----------+-----------+-----------+------------+-----------+
##     | ASCON128a | ASCON128  | ASCON80PQ | ASCONHASHa | ASCONHASH |
##     | RATE 16   | RATE  8   | RATE  8   | RATE  8    | RATE  8   |
##     | PA   12   | PA   12   | PA   12   | PA   12    | PA   12   |
##     | PB    8   | PB    6   | PB    6   | PB    8    | PB   12   |
##     | KEY  16   | KEY  16   | KEY  20   |            |           |
##     +-----------+-----------+-----------+------------+-----------+
##   0 | bytes     | bytes     | bytes     | bytes      | bytes     |
##   4 | |         | \----     | \----     | \----      | \----     |
##   8 | |         |           |           |            |           |
##  12 | \----     |           |           |            |           |
##  16 |           |           | key k2h   |            |           |
##  20 | optr      | optr      | optr      | optr       | optr      |
##  24 | mode      | mode      | mode      |            |           |
##  28 | saved s11 | saved s11 | saved s11 | saved s11  | saved s11 |
##  32 | saved s10 | saved s10 | saved s10 | saved s10  | saved s10 |
##  36 | saved s9  | saved s9  | saved s9  | saved s9   | saved s9  |
##  40 | saved s8  | saved s8  | saved s8  | saved s8   | saved s8  |
##  44 | saved s7  | saved s7  | saved s7  | saved s7   | saved s7  |
##  48 | saved s6  | saved s6  | saved s6  | saved s6   | saved s6  |
##  52 | saved s5  | saved s5  | saved s5  | saved s5   | saved s5  |
##  56 | saved s4  | saved s4  | saved s4  | saved s4   | saved s4  |
##  60 | saved s3  | saved s3  | saved s3  | saved s3   | saved s3  |
##  64 | saved s2  | saved s2  | saved s2  | saved s2   | saved s2  |
##  68 | saved s1  | saved s1  | saved s1  | saved s1   | saved s1  |
##  72 | saved s0  | saved s0  | saved s0  | saved s0   | saved s0  |
##  76 | saved ra  | saved ra  | saved ra  | saved ra   | saved ra  |
##  80 +-----------+-----------+-----------+------------+-----------+

## ASCON128a
#define VARIANT 1
#define RATE 16
#define PA_ROUNDS 12
#define PA_START_ROUND 0xf0
#define PB_ROUNDS 8
#define PB_START_ROUND 0xb4
#define IVl ((CRYPTO_ABYTES * 8) << 24 | PB_ROUNDS << 20 | PA_ROUNDS << 16 | VARIANT)
#define IVh (RATE << 8)

#define S_key 16
#define S_optr 20
#define S_mode 24

.macro sbox x0, x1, x2, x3, x4, t0, t1, t2
    xor \t1, \x0, \x4
    xor \t2, \x3, \x4
    xori \x4, \x4, -1
    xor \t0, \x1, \x2
    or \x4, \x4, \x3
    xor \x4, \x4, \t0
    xor \x3, \x3, \x1
    or \x3, \x3, \t0
    xor \x3, \x3, \t1
    xor \x2, \x2, \t1
    or \x2, \x2, \x1
    xor \x2, \x2, \t2
    or \x0, \x0, \t2
    xor \t0, \t0, \x0
    xori \t1, \t1, -1
    and \x1, \x1, \t1
    xor \x1, \x1, \t2
.endm

.macro linear dl, dh, sl, sh, sl0, sh0, r0, sl1, sh1, r1, t0
    slli \dh, \sl0, (32 - \r0)
    srli \t0, \sh0, \r0
    xor \dh, \dh, \t0
    slli \t0, \sl1, (32 - \r1)
    xor \dh, \dh, \t0
    srli \t0, \sh1, \r1
    xor \dh, \dh, \t0
    slli \dl, \sh0, (32 - \r0)
    srli \t0, \sl0, \r0
    xor \dl, \dl, \t0
    slli \t0, \sh1, (32 - \r1)
    xor \dl, \dl, \t0
    srli \t0, \sl1, \r1
    xor \dl, \dl, \t0
    xor \dl, \dl, \sl
    xor \dh, \dh, \sh
.endm

.align 4
.globl ascon_permute
.type ascon_permute,@function
ascon_permute:
    # ascon permutation
    # state in s0 .. s9
    # start round constant in t1
    # temporaries in t3, t4, t5
    # link register in t0
    li t1l, 0x4b
.LPloop:
    # round constant
    xor x2l, x2l, t1

    # s-box
    sbox x0l, x1l, x2l, x3l, x4l, t0l, t0h, t1h
    sbox x0h, x1h, x2h, x3h, x4h, t0h, x0l, t1h

    # linear layer
    linear x0l, x0h, x2l, x2h, x2l, x2h, 19, x2l, x2h, 28, t1h
    linear x2l, x2h, x4l, x4h, x4l, x4h, 1, x4l, x4h, 6, t1h
    linear x4l, x4h, x1l, x1h, x1l, x1h, 7, x1h, x1l, 9, t1h
    linear x1l, x1h, x3l, x3h, x3h, x3l, 29, x3h, x3l, 7, t1h
    linear x3l, x3h, t0l, t0h, t0l, t0h, 10, t0l, t0h, 17, t1h

    # condition
    addi t1, t1, -15
    bge t1, t1l, .LPloop

.LPend:
    jalr zero, 0(t0)

.align 4
.globl ascon_memcpy
.type ascon_memcpy,@function
ascon_memcpy:
    # memcpy that preserves registers used by ascon
    # dest in t1
    # src in t2
    # len in a4
    # temporaries in t3, t4
    # link register in t0
    li t3, 0
    j .LMcond
.LMloop:
    lbu t4, 0(t2)
    sb t4, 0(t1)
    addi t1, t1, 1
    addi t2, t2, 1
    addi t3, t3, 1
.LMcond:
    blt t3, ilen, .LMloop
.LMend:
    jalr zero, 0(t0)

.align 4
.globl ascon_duplex
.type ascon_duplex,@function
ascon_duplex:
    j .LDcond

.LDloop:
    lw t0l, 0(iptr)
    lw t0h, 4(iptr)
    lw t1l, 8(iptr)
    lw t1h, 12(iptr)
    xor x0h, x0h, t0h
    xor x0l, x0l, t0l
    xor x1h, x1h, t1h
    xor x1l, x1l, t1l

.LDsqueeze:
    beq mode, zero, .LDreset
    sw x0l, 0(optr)
    sw x0h, 4(optr)
    sw x1l, 8(optr)
    sw x1h, 12(optr)

.LDreset:
    bge mode, zero, .LDpermute
    mv x0h, t0h
    mv x0l, t0l
    mv x1h, t1h
    mv x1l, t1l

.LDpermute:
    li t1, PB_START_ROUND
    jal t0, ascon_permute

    addi optr, optr, RATE
    addi iptr, iptr, RATE
    addi ilen, ilen, -RATE

.LDcond:
    li t0, RATE
    bge ilen, t0, .LDloop

.LDend:
    sw zero, 0(sp)
    sw zero, 4(sp)
    sw zero, 8(sp)
    sw zero, 12(sp)

    mv t1, sp
    mv t2, iptr
    jal t0, ascon_memcpy

    add t1, sp, ilen
    lbu t0, 0(t1)
    xori t0, t0, 1
    sb t0, 0(t1)

    lw t0l, 0(sp)
    lw t0h, 4(sp)
    lw t1l, 8(sp)
    lw t1h, 12(sp)
    xor x0h, x0h, t0h
    xor x0l, x0l, t0l
    xor x1h, x1h, t1h
    xor x1l, x1l, t1l

.LDendsqueeze:
    beq mode, zero, .LDendreset

    mv t0h, x0h
    mv t0l, x0l
    mv t1h, x1h
    mv t1l, x1l
    sw t0l, 0(sp)
    sw t0h, 4(sp)
    sw t1l, 8(sp)
    sw t1h, 12(sp)

    mv t1, optr
    mv t2, sp
    jal t0, ascon_memcpy

.LDendreset:
    bge mode, zero, .LDreturn

    mv t1, sp
    mv t2, iptr
    jal t0, ascon_memcpy

    lw t0l, 0(sp)
    lw t0h, 4(sp)
    lw t1h, 8(sp)
    lw t1l, 12(sp)
    mv x0h, t0h
    mv x0l, t0l
    mv x1l, t1h
    mv x1h, t1l

.LDreturn:
    add optr, optr, ilen
    add iptr, iptr, ilen
    ret

.macro sw_unaligned x, off, a
    sb \x, 0+\off(\a)
    srli \x, \x, 8
    sb \x, 1+\off(\a)
    srli \x, \x, 8
    sb \x, 2+\off(\a)
    srli \x, \x, 8
    sb \x, 3+\off(\a)
.endm

.macro lw_unaligned_4x x1, x2, x3, x4, a, t0, t1, t2, t3
    andi \t0, \a, -4
    lw \x1, 0(\t0)
    lw \x2, 4(\t0)
    lw \x3, 8(\t0)
    lw \x4, 12(\t0)
    beq \t0, \a, 1f
    lw \t0, 16(\t0)
    andi \t1, \a, 3
    slli \t1, \t1, 3
    sub \t2, zero, \t1
    srl \x1, \x1, \t1
    sll \t3, \x2, \t2
    or \x1, \x1, \t3
    srl \x2, \x2, \t1
    sll \t3, \x3, \t2
    or \x2, \x2, \t3
    srl \x3, \x3, \t1
    sll \t3, \x4, \t2
    or \x3, \x3, \t3
    srl \x4, \x4, \t1
    sll \t3, \t0, \t2
    or \x4, \x4, \t3
    1:
.endm

.align 4
.globl ascon_core
.type ascon_core,@function
ascon_core:
    # ascon algorithm
    # sets up state in s0 .. s9
    # outptr in a0
    # inptr in a1
    # inlen in a2
    # adptr in a3 (later used as inptr)
    # adlen in a4 (later used as inlen)
    # nptr in a5 (later used as k1h)
    # kptr in a6 (later used as k1l)
    # mode in a7 (1 enc, 0 ad, -1 dec)
    # link register in ra
    addi sp, sp, -80
    sw ra, 76(sp)
    sw s0, 72(sp)
    sw s1, 68(sp)
    sw s2, 64(sp)
    sw s3, 60(sp)
    sw s4, 56(sp)
    sw s5, 52(sp)
    sw s6, 48(sp)
    sw s7, 44(sp)
    sw s8, 40(sp)
    sw s9, 36(sp)
    sw s10, 32(sp)
    sw s11, 28(sp)

    # sign-extend mode
    slli a7, a7, 24
    srai a7, a7, 24

    lw t0l, 0(a5)
    lw t0h, 4(a5)
    lw t1l, 8(a5)
    lw t1h, 12(a5)
    mv x3h, t0h
    mv x3l, t0l
    mv x4h, t1h
    mv x4l, t1l

    lw t0l, 0(a6)
    lw t0h, 4(a6)
    lw t1l, 8(a6)
    lw t1h, 12(a6)
    mv k0h, t0h
    mv k0l, t0l
    mv k1h, t1h
    mv k1l, t1l

    li x0h, IVh
    li x0l, IVl
    mv x1h, k0h
    mv x1l, k0l
    mv x2h, k1h
    mv x2l, k1l

    li t1, PA_START_ROUND
    jal t0, ascon_permute

    xor x3h, x3h, k0h
    xor x3l, x3l, k0l
    xor x4h, x4h, k1h
    xor x4l, x4l, k1l

    beq ilen, zero, .LCskipad

    sw optr, S_optr(sp)
    sw mode, S_mode(sp)
    mv mode, zero
    jal ra, ascon_duplex
    lw optr, S_optr(sp)
    lw mode, S_mode(sp)

    li t1, PB_START_ROUND
    jal t0, ascon_permute

.LCskipad:
    li t0, 0x80000000
    xor x4h, x4h, t0

    mv iptr, a1
    mv ilen, a2
    jal ra, ascon_duplex

    xor x2h, x2h, k0h
    xor x2l, x2l, k0l
    xor x3h, x3h, k1h
    xor x3l, x3l, k1l

    li t1, PA_START_ROUND
    jal t0, ascon_permute

    xor x3h, x3h, k0h
    xor x3l, x3l, k0l
    xor x4h, x4h, k1h
    xor x4l, x4l, k1l

    bge mode, zero, .LCencrypt
.LCdecrypt:
    lw_unaligned_4x t0l, t0h, t1l, t1h, iptr, t0, t1, t2, k0h

    xor t0, x3h, t0h
    xor t1, x3l, t0l
    xor t0, t0, t1
    xor t1, x4h, t1h
    xor t0, t0, t1
    xor t1, x4l, t1l
    xor t0, t0, t1

    beq t0, zero, .LCzeroreturn
    li a0, -1
    j .LCreturn
.LCencrypt:

    mv t0h, x3h
    mv t0l, x3l
    mv t1h, x4h
    mv t1l, x4l
    sw_unaligned t0l, 0, optr
    sw_unaligned t0h, 4, optr
    sw_unaligned t1l, 8, optr
    sw_unaligned t1h, 12, optr

.LCzeroreturn:
    li a0, 0
.LCreturn:
    lw ra, 76(sp)
    lw s0, 72(sp)
    lw s1, 68(sp)
    lw s2, 64(sp)
    lw s3, 60(sp)
    lw s4, 56(sp)
    lw s5, 52(sp)
    lw s6, 48(sp)
    lw s7, 44(sp)
    lw s8, 40(sp)
    lw s9, 36(sp)
    lw s10, 32(sp)
    lw s11, 28(sp)
    addi sp, sp, 80
    jalr zero, 0(ra)
