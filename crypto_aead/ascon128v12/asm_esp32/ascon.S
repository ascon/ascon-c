#include <xtensa/coreasm.h>
#include "api.h"

## REGISTER ALLOCATION
#define t0h a4
#define t0l a5
#define x0h a6
#define x0l a7
#define x1h a8
#define x1l a9
#define x2h a10
#define x2l a11
#define x3h a12
#define x3l a13
#define x4h a14
#define x4l a15
## OVERLAPPING REGISTER ALLOCATION
#define optr x2h
#define iptr x2l
#define ilen x3h
#define mode x3l
#define t1h x4h
#define t1l x4l

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
##   8 | |         | optr      | optr      | optr       | optr      |
##  12 | \----     | iptr      | iptr      | iptr  cur  | iptr  cur |
##  16 | state x2h | state x2h | state x2h |            |           |
##  20 | |     x2l | |     x2l | |     x2l | state x2l  | state x2l |
##  24 | |     x3h | |     x3h | |     x3h | \---- x3h  | \---- x3h |
##  28 | |     x3l | \---- x3l | \---- x3l |            |           |
##  32 | |     x4h | ilen      | ilen      | ilen  cur  | ilen  cur |
##  36 | \---- x4l | mode  cur | mode  cur | olen       | olen      |
##  40 | key   k0h | key   k0h | key   k1h |            |           |
##  44 | |     k0l | |     k0l | |     k1l | lr         | lr        |
##  48 | |     k1h | |     k1h | |     k2h +------------+-----------+
##  52 | \---- k1l | \---- k1l | |     k2l |
##  56 |           |           | \---- k0h |
##  60 | optr  cur | optr  cur | optr  cur |
##  64 | iptr  cur | iptr  cur | iptr  cur |
##  68 | ilen  cur | ilen  cur | ilen  cur |
##  72 | mode  cur | lr2       | lr2       |
##  76 | optr      | lr        | lr        |
##  80 | iptr      +-----------+-----------+
##  84 | ilen      |           |           |
##  88 | lr2       |           |           |
##  92 | lr        +-----------+-----------+
##  96 +-----------+ kptr  arg | kptr  arg |
## 100 |           | mode  arg | mode  arg |
## 104 |           +-----------+-----------+
## 108 +-----------+
## 112 | kptr  arg |
## 116 | mode  arg |
## 120 +-----------+

## ASCON128
#define RATE 8
#define PA_ROUNDS 12
#define PA_START_ROUND 0xf0
#define PB_ROUNDS 6
#define PB_START_ROUND 0x96
#define IVh (((8 * CRYPTO_KEYBYTES) << 24) | ((8 * RATE) << 16) | (PA_ROUNDS << 8) | (PB_ROUNDS << 0))
#define IVl 0

#define S_state 16
#define S_key 40
#define S_optr_cur 60
#define S_iptr_cur 64
#define S_ilen_cur 68
#define S_mode_cur 36
#define S_optr 8
#define S_iptr 12
#define S_ilen 32
#define S_lr2 72
#define S_lr 76
#define S_kptr_arg 96
#define S_mode_arg 100

.macro sbox x0, x1, x2, x3, x4, r0, t0, t1, t2
    xor \t1, \x0, \x4
    xor \t2, \x3, \x4
    movi \t0, -1
    xor \x4, \x4, \t0
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
    xor \x0, \x0, \t0
    movi \t0, -1
    xor \t1, \t1, \t0
    and \x1, \x1, \t1
    xor \x1, \x1, \t2
    mov \r0, \x0
.endm

.macro linear dl, dh, sl, sh, sl0, sh0, r0, sl1, sh1, r1, t0
    ssai \r0
    src \dl, \sh0, \sl0
    src \dh, \sl0, \sh0
    xor \dl, \dl, \sl
    xor \dh, \dh, \sh
    ssai \r1
    src \t0, \sh1, \sl1
    src \sh, \sl1, \sh1
    xor \dl, \dl, \t0
    xor \dh, \dh, \sh
.endm

.align 4
.globl ascon_permute
.type ascon_permute,@function
ascon_permute:
    # ascon permutation
    # state in a6 .. a9 and sp + 16 .. sp + 36
    # start round in a2
    # temporaries in a3, a4, a5
    l32i x2h, a1, (S_state + 0)
    l32i x2l, a1, (S_state + 4)
    l32i x3h, a1, (S_state + 8)
    l32i x3l, a1, (S_state + 12)
.globl ascon_permute_noload
.type ascon_permute_noload,@function
ascon_permute_noload:
    # state in a6 .. a15
    # start round constant in a2
    # round count in a3
    # temporaries in a3, a4, a5

    # ESP32 zero-overhead looping
    floop a3, Ploop
.LPloop:
    # round constant
    xor x2l, x2l, a2

    # s-box
    sbox x0l, x1l, x2l, x3l, x4l, t0l, t0h, t0l, a3
    sbox x0h, x1h, x2h, x3h, x4h, t0h, t0h, x0l, a3

    # linear layer
    linear x0l, x0h, x2l, x2h, x2l, x2h, 19, x2l, x2h, 28, a3
    linear x2l, x2h, x4l, x4h, x4l, x4h, 1, x4l, x4h, 6, a3
    linear x4l, x4h, x1l, x1h, x1l, x1h, 7, x1h, x1l, 9, a3
    linear x1l, x1h, x3l, x3h, x3h, x3l, 29, x3h, x3l, 7, a3
    linear x3l, x3h, t0l, t0h, t0l, t0h, 10, t0l, t0h, 17, a3

    # condition
    addi a2, a2, -15

    floopend a3, Ploop
.LPend:
    s32i x2h, a1, (S_state + 0)
    s32i x2l, a1, (S_state + 4)
    s32i x3h, a1, (S_state + 8)
    s32i x3l, a1, (S_state + 12)
    ret

.align 4
.globl ascon_rev8
.type ascon_rev8,@function
ascon_rev8:
    # ascon bytereverse one block
    # arguments and results in a4, a5, a14, a15
    # temporaries in a2
    ssai 8
    srli a2, t1h, 16
    src a2, a2, t1h
    src a2, a2, a2
    src t1h, t1h, a2

    srli a2, t1l, 16
    src a2, a2, t1l
    src a2, a2, a2
    src t1l, t1l, a2

.globl ascon_rev8_half
.type ascon_rev8_half,@function
ascon_rev8_half:
    ssai 8
    srli a2, t0h, 16
    src a2, a2, t0h
    src a2, a2, a2
    src t0h, t0h, a2

    srli a2, t0l, 16
    src a2, a2, t0l
    src a2, a2, a2
    src t0l, t0l, a2

    ret

.align 4
.globl ascon_memcpy
.type ascon_memcpy,@function
ascon_memcpy:
    # memcpy that preserves registers used by ascon
    # dest in a2
    # src in a3
    # temporaries in a4, a5
    movi a4, 0
    j .LMcond
.LMloop:
    l8ui a5, a3, 0
    s8i a5, a2, 0
    addi a2, a2, 1
    addi a3, a3, 1
    addi a4, a4, 1
.LMcond:
    bltu a4, ilen, .LMloop
.LMend:
    ret

.align 4
.globl ascon_duplex
.type ascon_duplex,@function
ascon_duplex:
    s32i a0, a1, S_lr2
    j .LDcond

.LDloop:
    l32i t0h, iptr, 0
    l32i t0l, iptr, 4
    call0 ascon_rev8_half
    xor x0h, x0h, t0h
    xor x0l, x0l, t0l

.LDsqueeze:
    beqz a13, .LDreset

    # ascon_rev8
    # inlined here to preserve registers
    ssai 8
    srli a2, x0h, 16
    src a2, a2, x0h
    src a2, a2, a2
    src a2, x0h, a2
    s32i a2, optr, 0

    srli a2, x0l, 16
    src a2, a2, x0l
    src a2, a2, a2
    src a2, x0l, a2
    s32i a2, optr, 4

.LDreset:
    bgez mode, .LDpermute
    mov x0h, t0h
    mov x0l, t0l

.LDpermute:
    s32i optr, a1, S_optr_cur
    s32i iptr, a1, S_iptr_cur
    s32i ilen, a1, S_ilen_cur
    movi a2, PB_START_ROUND
    movi a3, PB_ROUNDS
    call0 ascon_permute
    l32i optr, a1, S_optr_cur
    l32i iptr, a1, S_iptr_cur
    l32i ilen, a1, S_ilen_cur
    l32i mode, a1, S_mode_cur

    addi optr, optr, RATE
    addi iptr, iptr, RATE
    addi ilen, ilen, -RATE

.LDcond:
    bgeui ilen, RATE, .LDloop

.LDend:
    movi a2, 0
    s32i a2, a1, 0
    s32i a2, a1, 4

    mov a2, a1
    mov a3, iptr
    call0 ascon_memcpy

    movi a4, 0x80
    add a2, a1, ilen
    l8ui a3, a2, 0
    xor a3, a3, a4
    s8i a3, a2, 0

    l32i t0h, a1, 0
    l32i t0l, a1, 4
    call0 ascon_rev8_half
    xor x0h, x0h, t0h
    xor x0l, x0l, t0l

.LDendsqueeze:
    beqz mode, .LDendreset

    mov t0h, x0h
    mov t0l, x0l
    call0 ascon_rev8_half
    s32i t0h, a1, 0
    s32i t0l, a1, 4

    mov a2, optr
    mov a3, a1
    call0 ascon_memcpy

.LDendreset:
    bgez mode, .LDreturn

    mov a2, a1
    mov a3, iptr
    call0 ascon_memcpy

    l32i t0h, a1, 0
    l32i t0l, a1, 4
    call0 ascon_rev8_half
    mov x0h, t0h
    mov x0l, t0l

.LDreturn:
    add optr, optr, ilen
    add iptr, iptr, ilen
    l32i a0, a1, S_lr2
    ret

.align 4
.globl ascon_core
.type ascon_core,@function
ascon_core:
    abi_entry 80, 4
    s32i a0, a1, S_lr
    s32i a2, a1, S_optr
    s32i a3, a1, S_iptr
    s32i a4, a1, S_ilen
    s32i a5, a1, S_iptr_cur
    s32i a6, a1, S_ilen_cur

    # load key
    l32i a2, a1, S_kptr_arg
    l32i t0h, a2, 0
    l32i t0l, a2, 4
    l32i t1h, a2, 8
    l32i t1l, a2, 12
    call0 ascon_rev8
    s32i t0h, a1, (S_key + 0)
    s32i t0l, a1, (S_key + 4)
    s32i t1h, a1, (S_key + 8)
    s32i t1l, a1, (S_key + 12)
    mov x1h, t0h
    mov x1l, t0l
    mov x2h, t1h
    mov x2l, t1l

    # load nonce
    # a7 is not clobbered by ascon_rev8
    # a7 does not overlap x1, x2, t0, or t1
    # x4 overlaps t1, move unnecessary
    mov a2, a7
    l32i t0h, a2, 0
    l32i t0l, a2, 4
    l32i t1h, a2, 8
    l32i t1l, a2, 12
    call0 ascon_rev8
    mov x3h, t0h
    mov x3l, t0l

    # load IV
    # this clobbers a7
    movi x0h, IVh
    movi x0l, IVl

    movi a2, PA_START_ROUND
    movi a3, PA_ROUNDS
    call0 ascon_permute_noload

    # xor key
    # x4 overlaps t1, do in two steps
    l32i t0h, a1, (S_key + 0)
    l32i t0l, a1, (S_key + 4)
    xor x3h, x3h, t0h
    xor x3l, x3l, t0l
    l32i t0h, a1, (S_key + 8)
    l32i t0l, a1, (S_key + 12)
    xor x4h, x4h, t0h
    xor x4l, x4l, t0l

    # save state
    s32i x2h, a1, (S_state + 0)
    s32i x2l, a1, (S_state + 4)
    s32i x3h, a1, (S_state + 8)
    s32i x3l, a1, (S_state + 12)

    l32i ilen, a1, S_ilen_cur
    beqz ilen, .LCskipad

    l32i iptr, a1, S_iptr_cur
    movi mode, 0
    s32i mode, a1, S_mode_cur
    call0 ascon_duplex

    movi a2, PB_START_ROUND
    movi a3, PB_ROUNDS
    call0 ascon_permute

.LCskipad:
    movi a2, 1
    xor x4l, x4l, a2

    l32i optr, a1, S_optr
    l32i iptr, a1, S_iptr
    l32i ilen, a1, S_ilen
    l8ui mode, a1, S_mode_arg
    slli mode, mode, 24
    srai mode, mode, 24
    s32i mode, a1, S_mode_cur
    call0 ascon_duplex
    s32i optr, a1, S_optr_cur
    s32i iptr, a1, S_iptr_cur

    # restore state
    l32i x2h, a1, (S_state + 0)
    l32i x2l, a1, (S_state + 4)
    l32i x3h, a1, (S_state + 8)
    l32i x3l, a1, (S_state + 12)

    # xor key
    # x4 overlaps t1, do in two steps
    l32i t0h, a1, (S_key + 0)
    l32i t0l, a1, (S_key + 4)
    xor x1h, x1h, t0h
    xor x1l, x1l, t0l
    l32i t0h, a1, (S_key + 8)
    l32i t0l, a1, (S_key + 12)
    xor x2h, x2h, t0h
    xor x2l, x2l, t0l

    movi a2, PA_START_ROUND
    movi a3, PA_ROUNDS
    call0 ascon_permute_noload

    # xor key
    # x4 overlaps t1, do in two steps
    l32i t0h, a1, (S_key + 0)
    l32i t0l, a1, (S_key + 4)
    xor x3h, x3h, t0h
    xor x3l, x3l, t0l
    l32i t0h, a1, (S_key + 8)
    l32i t0l, a1, (S_key + 12)
    xor x4h, x4h, t0h
    xor x4l, x4l, t0l

    l32i a2, a1, S_mode_cur
    bgez a2, .LCencrypt
.LCdecrypt:

    # save x4 into x0
    # x0 is no longer needed
    # x4 overlaps t1
    mov x0h, x4h
    mov x0l, x4l

    l32i a2, a1, S_iptr_cur
    l32i t0h, a2, 0
    l32i t0l, a2, 4
    l32i t1h, a2, 8
    l32i t1l, a2, 12
    call0 ascon_rev8

    # check tag
    # x4 is in x0
    xor a2, x3h, t0h
    xor a3, x3l, t0l
    xor a2, a2, a3
    xor a3, x0h, t1h
    xor a2, a2, a3
    xor a3, x0l, t1l
    xor a2, a2, a3

    beqz a2, .LCzeroreturn
    movi a2, -1
    j .LCreturn
.LCencrypt:

    # store tag
    # x4 overlaps t1, move unnecessary
    mov t0h, x3h
    mov t0l, x3l
    call0 ascon_rev8
    l32i a2, a1, S_optr_cur
    s32i t0h, a2, 0
    s32i t0l, a2, 4
    s32i t1h, a2, 8
    s32i t1l, a2, 12

.LCzeroreturn:
    movi a2, 0
.LCreturn:
    l32i a0, a1, S_lr
    abi_return
