///////////////////////////////////////////////////////////////////////////////
// P.S: AVR Assembler implementation (GCC) of the ASCON permutation. //
// Version 1.0.2 (17-09-21), see <http://github.com/johgrolux/> for updates. //
// Author: L. Cardoso and J. Groszschaedl, (DCS, University of Luxembourg).  //
// License: GPLv3 (see LICENSE file), other licenses available upon request. //
// Copyright (C) 2020-2021 University of Luxembourg <http://www.uni.lu/>.    //
// ------------------------------------------------------------------------- //
// This source code is free software: you can redistribute it and/or modify  //
// it under the terms of the GNU General Public License as published by the  //
// Free Software Foundation, either version 3 of the License, or (at your    //
// option) any later version. This source code is distributed in the hope    //
// that it will be useful, but WITHOUT ANY WARRANTY; without even the        //
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  //
// See the GNU General Public License for more details. You should have      //
// received a copy of the GNU General Public License along with this source  //
// code. If not, see <http://www.gnu.org/licenses/>.                         //
///////////////////////////////////////////////////////////////////////////////


// Function prototype:
// -------------------
// void P(State *s, int nr)
//
// Parameters:
// -----------
// `s`: pointer to a union containing five 64-bit state-words
// `nr`: number of rounds
//
// Return value:
// -------------
// None


#include "avr/io.h"
.section .text


///////////////////////////////////////////////////////////////////////////////
//////////////////////// REGISTER NAMES AND CONSTANTS /////////////////////////
///////////////////////////////////////////////////////////////////////////////


// Two AVR registers form a Dual-byte Register (DR) that can store a 16-bit
// operand, whereby little-endian format is used, i.e. the least-significant
// byte of the 16-bit operand is in the lower register.

// Eight dual-byte registers for sbox layer
#define DR0 r10,r11
#define DR1 r12,r13
#define DR2 r14,r15
#define DR3 r16,r17
#define DR4 r18,r19
#define DR5 r20,r21
#define DR6 r22,r23
#define DR7 r24,r25

// Register to determine loop termination
#define stop r0
// Zero register (for e.g. left-rotation)
#define zero r1
// Register for loop-counter of sbox loop
#define scnt r26
// Temporary register (INITVARS, ADDRCON)
#define tmp r26
// Register for 8-bit round-constant RCON
#define rcon r27

// Eight AVR registers form an Octa-byte Register (OR) that can store a 64-bit
// operand, whereby little-endian format is used, i.e. the least-significant
// byte of the 64-bit operand is in the lowest register.

// Two octa-byte registers for linear layer
#define OR0 r10,r11,r12,r13,r14,r15,r16,r17
#define OR1 r18,r19,r20,r21,r22,r23,r24,r25

// Octa-byte registers rotated 1 byte right
#define OR0R8 r11,r12,r13,r14,r15,r16,r17,r10
#define OR1R8 r19,r20,r21,r22,r23,r24,r25,r18

// Octa-byte registers rotated 2 bytes right
#define OR0R16 r12,r13,r14,r15,r16,r17,r10,r11
#define OR1R16 r20,r21,r22,r23,r24,r25,r18,r19

// Octa-byte registers rotated 3 bytes right
#define OR0R24 r13,r14,r15,r16,r17,r10,r11,r12
#define OR1R24 r21,r22,r23,r24,r25,r18,r19,r20

// Octa-byte registers rotated 3 bytes left
#define OR0L24 r15,r16,r17,r10,r11,r12,r13,r14
#define OR1L24 r23,r24,r25,r18,r19,r20,r21,r22

// Offset values for octa-byte registers
#define OF0  0,1,2,3,4,5,6,7
#define OF8  8,9,10,11,12,13,14,15
#define OF16 16,17,18,19,20,21,22,23
#define OF24 24,25,26,27,28,29,30,31
#define OF32 32,33,34,35,36,37,38,39


///////////////////////////////////////////////////////////////////////////////
///// MACROS FOR DUAL-BYTE ARITHMETIC/LOGICAL OPERATIONS AND LOADS/STORES /////
///////////////////////////////////////////////////////////////////////////////


// The macro `DMOV` moves a dual-byte operand: A = B.

.macro DMOV a0:req, a1:req, b0:req, b1:req
    movw    \a0, \b0
.endm


// The macro `DAND` bitwise ANDs two dual-byte operands: A = A ^ B. An implicit
// 8-bit rotation of operand B is possible, i.e the macro can also perform an
// operation of the form A = A ^ (B >>> 8).

.macro DAND a0:req, a1:req, b0:req, b1:req
    and     \a0, \b0
    and     \a1, \b1
.endm


// The macro `DIOR` bitwise ORs two dual-byte operands: A = A | B. An implicit
// 8-bit rotation of operand B is possible, i.e the macro can also perform an
// operation of the form A = A | (B >>> 8).

.macro DIOR a0:req, a1:req, b0:req, b1:req
    or      \a0, \b0
    or      \a1, \b1
.endm


// The macro `DXOR` bitwise XORs two dual-byte operands: A = A ^ B. An implicit
// 8-bit rotation of operand B is possible, i.e the macro can also perform an
// operation of the form A = A ^ (B >>> 8).

.macro DXOR a0:req, a1:req, b0:req, b1:req
    eor     \a0, \b0
    eor     \a1, \b1
.endm


// The macro `DINV` bitwise inverts a dual-byte operand, whereby the inversion
// is performed in place: A = ~A.

.macro DINV a0:req, a1:req
    com     \a0
    com     \a1
.endm


// Note: The AVR architecture uses little-endian memory format, which means the
// least-significant byte of a 16-bit word is at the lowest address.


// The macro `DLDZO` loads a dual-byte operand from RAM via Z-pointer using the
// base+offset (i.e. displacement) addressing mode: A = RAM[Z+B]. An implicit
// 8-bit rotation of the loaded operand is possible, i.e. the macro can also
// perform an operation of the form A = RAM[Z+B] >>> 8.

.macro DLDZO a0:req, a1:req, b0:req, b1:req
    ldd     \a0, Z+\b0
    ldd     \a1, Z+\b1
.endm


// The macro `DSTZI` stores a dual-byte operand to RAM via Z-pointer using the
// auto/increment addressing mode: RAM[Z++] = A. An implicit 8-bit rotation of
// the operand to be stored is possible, i.e. the macro can also perform an
// operation of the form RAM[Z+B] = (A >>> 8).

.macro DSTZI a0:req, a1:req
    st      Z+, \a0
    st      Z+, \a1
.endm


// The macro `DSTZO` stores a dual-byte operand to RAM via Z-pointer using the
// base+offset (i.e. displacement) addressing mode: RAM[Z+B] = A. An implicit
// 8-bit rotation of the operand to be stored is possible, i.e. the macro can
// also perform an operation of the form RAM[Z+B] = (A >>> 8).

.macro DSTZO a0:req, a1:req, b0:req, b1:req
    std     Z+\b0, \a0
    std     Z+\b1, \a1
.endm


///////////////////////////////////////////////////////////////////////////////
///// MACROS FOR OCTA-BYTE ARITHMETIC/LOGICAL OPERATIONS AND LOADS/STORES /////
///////////////////////////////////////////////////////////////////////////////


// The macro `OMOV` moves an octa-byte operand: A = B. An implicit 16/32/48-bit
// rotation of operand B is possible, i.e the macro can also perform operations
// of the form A = (B >>> 16) and A = (B <<< 32).

.macro OMOV a0:req, a1:req, a2:req, a3:req, a4:req, a5:req, a6:req, a7:req, \
             b0:req, b1:req, b2:req, b3:req, b4:req, b5:req, b6:req, b7:req
    movw    \a0, \b0
    movw    \a2, \b2
    movw    \a4, \b4
    movw    \a6, \b6
.endm


// The macro `OROL` rotates an octa-byte operand one bit left, whereby the
// rotation is performed in place: A = A <<< 1.

.macro OROL a0:req, a1:req, a2:req, a3:req, a4:req, a5:req, a6:req, a7:req
    lsl     \a0
    rol     \a1
    rol     \a2
    rol     \a3
    rol     \a4
    rol     \a5
    rol     \a6
    rol     \a7
    adc     \a0, zero
.endm


// The macro `OROR` rotates an octa-byte operand one bit right, whereby the
// rotation is performed in place: A = A >>> 1.

.macro OROR a0:req, a1:req, a2:req, a3:req, a4:req, a5:req, a6:req, a7:req
    bst     \a0, 0
    ror     \a7
    ror     \a6
    ror     \a5
    ror     \a4
    ror     \a3
    ror     \a2
    ror     \a1
    ror     \a0
    bld     \a7, 7
.endm


// The macro `OXOR` bitwise XORs two octa-byte operands: A = A ^ B. An implicit
// bytewise rotation of operand B is possible, i.e the macro can also perform
// operations of the form A = A ^ (B >>> 8) and A = A ^ (B <<< 16).

.macro OXOR a0:req, a1:req, a2:req, a3:req, a4:req, a5:req, a6:req, a7:req, \
            b0:req, b1:req, b2:req, b3:req, b4:req, b5:req, b6:req, b7:req
    eor     \a0, \b0
    eor     \a1, \b1
    eor     \a2, \b2
    eor     \a3, \b3
    eor     \a4, \b4
    eor     \a5, \b5
    eor     \a6, \b6
    eor     \a7, \b7
.endm


// Note: The AVR architecture uses little-endian memory format, which means the
// least-significant byte of a 64-bit word is at the lowest address.


// The macro `OLDZO` loads an octa-byte operand from RAM via Z-pointer using
// the base+offset (i.e. displacement) addressing mode: A = RAM[Z+B]. An
// implicit bytewise rotation of the loaded operand is possible, i.e. the macro
// can also perform operations of the form A = RAM[Z+B] >>> 8 and
// A = RAM[Z+B] <<< 16.

.macro OLDZO a0:req, a1:req, a2:req, a3:req, a4:req, a5:req, a6:req, a7:req, \
             b0:req, b1:req, b2:req, b3:req, b4:req, b5:req, b6:req, b7:req
    ldd     \a0, Z+\b0
    ldd     \a1, Z+\b1
    ldd     \a2, Z+\b2
    ldd     \a3, Z+\b3
    ldd     \a4, Z+\b4
    ldd     \a5, Z+\b5
    ldd     \a6, Z+\b6
    ldd     \a7, Z+\b7
.endm


// The macro `OSTZO` stores an octa-byte operand to RAM via Z-pointer using the
// base+offset (i.e. displacement) addressing mode: RAM[Z+B] = A. An implicit
// bytewise rotation of the operand to be stored is possible, i.e. the macro 
// can also perform operations of the form RAM[Z+B] = (A >>> 8) and
// RAM[Z+B] = (A <<< 16).

.macro OSTZO a0:req, a1:req, a2:req, a3:req, a4:req, a5:req, a6:req, a7:req, \
             b0:req, b1:req, b2:req, b3:req, b4:req, b5:req, b6:req, b7:req
    std     Z+\b0, \a0
    std     Z+\b1, \a1
    std     Z+\b2, \a2
    std     Z+\b3, \a3
    std     Z+\b4, \a4
    std     Z+\b5, \a5
    std     Z+\b6, \a6
    std     Z+\b7, \a7
.endm


///////////////////////////////////////////////////////////////////////////////
/////////////////// HELPER MACROS FOR THE ASCON PERMUTATION ///////////////////
///////////////////////////////////////////////////////////////////////////////


// The macro `PROLOGUE` pushes all callee-saved registers on the stack.

.macro PROLOGUE
    push    r0
    push    r10
    push    r11
    push    r12
    push    r13
    push    r14
    push    r15
    push    r16
    push    r17
.endm


// The macro `EPILOGUE` pops all callee-saved registers from the stack and
// returns to the caller.

.macro EPILOGUE
    pop     r17
    pop     r16
    pop     r15
    pop     r14
    pop     r13
    pop     r12
    pop     r11
    pop     r10
    pop     r0
    ret
.endm


// The macro `INITVARS` initializes local variables.

.macro INITVARS
    // initialize pointers and constants
    movw    ZL, r24
    ldi     tmp, 0x3C
    mov     stop, tmp
    clr     zero
    // initial RCON = 15*rounds + 0x3C
    mov     rcon, r22
    swap    rcon
    sub     rcon, r22
    add     rcon, stop
.endm


// The macro `ADDRCON` XORs a round-constant to state-word X2. 

.macro ADDRCON
    ldd     tmp, Z+16
    eor     tmp, rcon
    std     Z+16, tmp
.endm


///////////////////////////////////////////////////////////////////////////////
/////////////////// MACROS FOR NONLINEAR SUBSTITUTION LAYER ///////////////////
///////////////////////////////////////////////////////////////////////////////


// The macro `LDSLICE` loads a 16-bit slice of each of the five 64-bit words
// of the state.

.macro LDSLICE
    DLDZO   DR0, 0,1
    DLDZO   DR1, 8,9
    DLDZO   DR2, 16,17
    DLDZO   DR3, 24,25
    DLDZO   DR4, 32,33
.endm


// The macro `SBOXSLICE` computes a 16-bit slice of the non-linear substitution
// operation. This implementation uses the improved formulae of Campos et al
// (Proceedings of CANS 2020).

.macro SBOXSLICE
    DMOV    DR5, DR0        // t5 = i0
    DMOV    DR6, DR1        // t6 = i1
    DMOV    DR7, DR3        // t7 = i3
    DXOR    DR5, DR4        // t5 = t5 ^ i4 = i0 ^ i4
    DXOR    DR6, DR2        // t6 = t6 ^ i2 = i1 ^ i2
    DXOR    DR7, DR4        // t7 = t7 ^ i4 = i3 ^ i4
    DINV    DR4             // i4 = ~i4
    DIOR    DR4, DR3        // i4 = i4 | i3
    DXOR    DR4, DR6        // i4 = i4 ^ t6 -> i4 contains o2
    DXOR    DR3, DR2        // i3 = i3 ^ i2
    DIOR    DR3, DR6        // i3 = i3 | t6
    DXOR    DR3, DR5        // i3 = i3 ^ t5 -> i3 contains o1
    DXOR    DR2, DR5        // i2 = i2 ^ t5
    DIOR    DR2, DR1        // i2 = i2 | i1
    DXOR    DR2, DR7        // i2 = i2 ^ t7 -> i2 contains o0
    DINV    DR5             // t5 = ~t5
    DAND    DR1, DR5        // i1 = i1 & t5
    DXOR    DR1, DR7        // i1 = i1 ^ t7 -> i1 contains o4
    DIOR    DR0, DR7        // i0 = i0 | t7
    DXOR    DR0, DR6        // i0 = i0 ^ t6 -> i0 contains o3
.endm


// The macro `STSLICE` stores a 16-bit slice of each of the five 64-bit words
// of the state.

.macro STSLICE
    DSTZI   DR2
    DSTZO   DR3, 6,7
    DSTZO   DR4, 14,15
    DSTZO   DR0, 22,23
    DSTZO   DR1, 30,31
.endm


// The macro `SBOXLAYER` computes the complete non-linear substitution layer in
// a 16-bit slice-wise fashion. This implementation is optimized for small code
// size.

.macro SBOXLAYER
    ldi     scnt, 4         // set slice-counter to 4
.LSBOXLOOP:                 // start of sbox-loop
    LDSLICE                 // load a 16-bit slice of the state from RAM
    SBOXSLICE               // compute the sbox for this 16-bit slice
    STSLICE                 // store the 16-bit slice to RAM
    dec     scnt            // decrement slice-counter
    cpse    scnt, zero      // check whether slice-counter is 0
    rjmp    .LSBOXLOOP      // if not then jump back to start of loop
    sbiw    ZL, 8           // set Z-pointer to start-address of X0
.endm


///////////////////////////////////////////////////////////////////////////////
////////////////////// MACROS FOR LINEAR DIFFUSION LAYER //////////////////////
///////////////////////////////////////////////////////////////////////////////


// The macro `LINSWX0` computes the linear diffusion operation on state-word
// X0: X0 = X0 ^ (X0 >>> 19) ^ (X0 >>> 28).

.macro LINSWX0
    OLDZO   OR0, OF0
    OMOV    OR1, OR0
    OROR    OR1
    OROR    OR1
    OROR    OR1
    OXOR    OR0, OR1R16
    OROR    OR1
    OXOR    OR0, OR1R24
    OSTZO   OR0, OF0
.endm


// The macro `LINSWX1` computes the linear diffusion operation on state-word
// X1: X1 = X1 ^ (X1 >>> 61) ^ (X1 >>> 39).

.macro LINSWX1
    OLDZO   OR0, OF8
    OMOV    OR1, OR0
    OROL    OR1
    OXOR    OR0, OR1L24
    OROL    OR1
    OROL    OR1
    OXOR    OR0, OR1
    OSTZO   OR0, OF8
.endm


// The macro `LINSWX2` computes the linear diffusion operation on state-word
// X2: X2 = X2 ^ (X2 >>> 1) ^ (X2 >>> 6).

.macro LINSWX2
    OLDZO   OR0, OF16
    OMOV    OR1, OR0
    OROR    OR1
    OXOR    OR1, OR0
    OROL    OR0
    OROL    OR0
    OXOR    OR1, OR0R8
    OSTZO   OR1, OF16
.endm


// The macro `LINSWX3` computes the linear diffusion operation on state-word
// X3: X3 = X3 ^ (X3 >>> 10) ^ (X3 >>> 17).

.macro LINSWX3
    OLDZO   OR0, OF24
    OMOV    OR1, OR0
    OROR    OR1
    OXOR    OR0, OR1R16
    OROR    OR1
    OXOR    OR0, OR1R8
    OSTZO   OR0, OF24
.endm


// The macro `LINSWX4` computes the linear diffusion operation on state-word
// X4: X4 = X4 ^ (X4 >>> 7) ^ (X4 >>> 41).

.macro LINSWX4
    OLDZO   OR0, OF32
    OMOV    OR1, OR0
    OROR    OR1
    OXOR    OR0, OR1L24
    OROL    OR1
    OROL    OR1
    OXOR    OR0, OR1R8
    OSTZO   OR0, OF32
.endm


// The macro `LINLAYER` computes the complete linear diffusion layer.

.macro LINLAYER
    LINSWX0
    LINSWX1
    LINSWX2
    LINSWX3
    LINSWX4
.endm


///////////////////////////////////////////////////////////////////////////////
////////////////////////////// ASCON PERMUTATION //////////////////////////////
///////////////////////////////////////////////////////////////////////////////


.balign 2
.global P
.type P, @function
.func P
P:
    PROLOGUE                // push callee-saved registers
    INITVARS                // initialize local variables
.LROUNDLOOP:                // start of round-loop
    ADDRCON                 // macro for addition of round-constant
    SBOXLAYER               // macro for nonlinear substitution layer
    LINLAYER                // macro for linear diffusion layer
    subi rcon, 15           // subtract 15 from round-constant
    cpse rcon, stop         // check whether round-constant is below 0x4B
    rjmp .LROUNDLOOP        // if not then jump back to start of loop
    EPILOGUE                // pop callee-saved registers and return
.endfunc


.size P, .-P


.end
