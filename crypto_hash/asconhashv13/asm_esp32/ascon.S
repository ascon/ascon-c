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

## ASCONHASH
#define RATE 8
#define PA_ROUNDS 12
#define PA_START_ROUND 0xf0
#define PB_ROUNDS 12
#define PB_START_ROUND 0xf0

#define S_state 20
#define S_iptr_cur 12
#define S_ilen_cur 32
#define S_optr 8
#define S_olen 36
#define S_lr 44

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
    l32i x2l, a1, (S_state + 0)
    l32i x3h, a1, (S_state + 4)
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
    s32i x2l, a1, (S_state + 0)
    s32i x3h, a1, (S_state + 4)
    ret

.align 4
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
.globl ascon_core
.type ascon_core,@function
ascon_core:
    abi_entry 48, 4
    s32i a0, a1, S_lr
    s32i a2, a1, S_optr
    s32i a3, a1, S_olen

    # setup absorb args
    mov iptr, a4
    mov ilen, a5

    # initialize
    movi x0h, 0xee9398aa
    movi x0l, 0xdb67f03d
    movi x1h, 0x8bb21831
    movi x1l, 0xc60f1002
    movi x2h, 0xb48a92db
    movi t0h, 0x98d5da62
    movi t0l, 0x43189921
    movi x3l, 0xb8f8e3e8
    movi x4h, 0x348fa5c9
    movi x4l, 0xd525e140

    # save state
    s32i t0h, a1, (S_state + 0)
    s32i t0l, a1, (S_state + 4)

    j .LAcond
.LAloop:
    l32i t0h, iptr, 0
    l32i t0l, iptr, 4
    call0 ascon_rev8_half
    xor x0h, x0h, t0h
    xor x0l, x0l, t0l

.LApermute:
    s32i iptr, a1, S_iptr_cur
    s32i ilen, a1, S_ilen_cur
    movi a2, PB_START_ROUND
    movi a3, PB_ROUNDS
    call0 ascon_permute
    l32i iptr, a1, S_iptr_cur
    l32i ilen, a1, S_ilen_cur

    addi iptr, iptr, RATE
    addi ilen, ilen, -RATE

.LAcond:
    bgeui ilen, RATE, .LAloop

.LAend:
    movi a2, 0
    s32i a2, a1, 0
    s32i a2, a1, 4

.LAendabsorb:
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

.LCpermute:
    movi a2, PA_START_ROUND
    movi a3, PA_ROUNDS
    call0 ascon_permute

    # setup squeeze args
    l32i iptr, a1, S_optr
    l32i ilen, a1, S_olen

    j .LScond
.LSloop:
    mov t0h, x0h
    mov t0l, x0l
    call0 ascon_rev8_half
    s32i t0h, iptr, 0
    s32i t0l, iptr, 4

.LSpermute:
    s32i iptr, a1, S_iptr_cur
    s32i ilen, a1, S_ilen_cur
    movi a2, PB_START_ROUND
    movi a3, PB_ROUNDS
    call0 ascon_permute
    l32i iptr, a1, S_iptr_cur
    l32i ilen, a1, S_ilen_cur

    addi iptr, iptr, RATE
    addi ilen, ilen, -RATE

.LScond:
    bgeui ilen, RATE, .LSloop

.LSend:
    movi a2, 0
    s32i a2, a1, 0
    s32i a2, a1, 4

.LSendsqueeze:
    mov t0h, x0h
    mov t0l, x0l
    call0 ascon_rev8_half
    s32i t0h, a1, 0
    s32i t0l, a1, 4

    mov a2, iptr
    mov a3, a1
    call0 ascon_memcpy

    # return
    movi a2, 0
    l32i a0, a1, S_lr
    abi_return
