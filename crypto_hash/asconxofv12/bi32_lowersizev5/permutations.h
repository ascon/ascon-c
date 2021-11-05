#ifndef PERMUTATIONS_H_
#define PERMUTATIONS_H_

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef struct {
  u32 e;
  u32 o;
} u32_2;

typedef struct {
  u32_2 x0;
  u32_2 x1;
  u32_2 x2;
  u32_2 x3;
  u32_2 x4;
} state;

#define ROTR32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define START_ROUND(x) (12 - (x))

// Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002
u32_2 to_bit_interleaving_big(u64 in);

// Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002
u64 from_bit_interleaving_big(u32_2 in);

#define to_bit_interleaving_big_immediate(out, in) \
  do { \
    u64 big_in = U64BIG(in); \
    u32 hi = (big_in) >> 32; \
    u32 lo = (u32)(big_in); \
    u32 r0, r1; \
    r0 = (lo ^ (lo >> 1)) & 0x22222222, lo ^= r0 ^ (r0 << 1); \
    r0 = (lo ^ (lo >> 2)) & 0x0C0C0C0C, lo ^= r0 ^ (r0 << 2); \
    r0 = (lo ^ (lo >> 4)) & 0x00F000F0, lo ^= r0 ^ (r0 << 4); \
    r0 = (lo ^ (lo >> 8)) & 0x0000FF00, lo ^= r0 ^ (r0 << 8); \
    r1 = (hi ^ (hi >> 1)) & 0x22222222, hi ^= r1 ^ (r1 << 1); \
    r1 = (hi ^ (hi >> 2)) & 0x0C0C0C0C, hi ^= r1 ^ (r1 << 2); \
    r1 = (hi ^ (hi >> 4)) & 0x00F000F0, hi ^= r1 ^ (r1 << 4); \
    r1 = (hi ^ (hi >> 8)) & 0x0000FF00, hi ^= r1 ^ (r1 << 8); \
    out.e = (lo & 0x0000FFFF) | (hi << 16); \
    out.o = (lo >> 16) | (hi & 0xFFFF0000); \
  } while (0)

#define from_bit_interleaving_big_immediate(out, in) \
  do { \
    u32 lo = (in.e & 0x0000FFFF) | (in.o << 16); \
    u32 hi = (in.e >> 16) | (in.o & 0xFFFF0000); \
    u32 r0, r1; \
    r0 = (lo ^ (lo >> 8)) & 0x0000FF00, lo ^= r0 ^ (r0 << 8); \
    r0 = (lo ^ (lo >> 4)) & 0x00F000F0, lo ^= r0 ^ (r0 << 4); \
    r0 = (lo ^ (lo >> 2)) & 0x0C0C0C0C, lo ^= r0 ^ (r0 << 2); \
    r0 = (lo ^ (lo >> 1)) & 0x22222222, lo ^= r0 ^ (r0 << 1); \
    r1 = (hi ^ (hi >> 8)) & 0x0000FF00, hi ^= r1 ^ (r1 << 8); \
    r1 = (hi ^ (hi >> 4)) & 0x00F000F0, hi ^= r1 ^ (r1 << 4); \
    r1 = (hi ^ (hi >> 2)) & 0x0C0C0C0C, hi ^= r1 ^ (r1 << 2); \
    r1 = (hi ^ (hi >> 1)) & 0x22222222, hi ^= r1 ^ (r1 << 1); \
    out = (u64)hi << 32 | lo; \
    out = U64BIG(out); \
  } while (0)

/* clang-format off */
#define ROUND(C_e, C_o) \
  do { \
    /* round constant */ \
    s.x2.e ^= C_e;                s.x2.o ^= C_o; \
    /* s-bos.x layer */ \
    s.x0.e ^= s.x4.e;             s.x0.o ^= s.x4.o; \
    s.x4.e ^= s.x3.e;             s.x4.o ^= s.x3.o; \
    s.x2.e ^= s.x1.e;             s.x2.o ^= s.x1.o; \
    t0.e    = s.x0.e & (~s.x4.e); t0.o = s.x0.o & (~s.x4.o); \
    s.x0.e ^= s.x2.e & (~s.x1.e); s.x0.o ^= s.x2.o & (~s.x1.o); \
    s.x2.e ^= s.x4.e & (~s.x3.e); s.x2.o ^= s.x4.o & (~s.x3.o); \
    s.x4.e ^= s.x1.e & (~s.x0.e); s.x4.o ^= s.x1.o & (~s.x0.o); \
    s.x1.e ^= s.x3.e & (~s.x2.e); s.x1.o ^= s.x3.o & (~s.x2.o); \
    s.x3.e ^= t0.e;               s.x3.o ^= t0.o; \
    s.x1.e ^= s.x0.e;             s.x1.o ^= s.x0.o; \
    s.x3.e ^= s.x2.e;             s.x3.o ^= s.x2.o; \
    s.x0.e ^= s.x4.e;             s.x0.o ^= s.x4.o; \
    /* linear layer */ \
    t0.e    = s.x0.e ^ ROTR32(s.x0.o, 4); \
    t0.o    = s.x0.o ^ ROTR32(s.x0.e, 5); \
    s.x0.e ^= ROTR32(t0.o, 9); \
    s.x0.o ^= ROTR32(t0.e, 10); \
    t0.e    = s.x1.e ^ ROTR32(s.x1.e, 11); \
    t0.o    = s.x1.o ^ ROTR32(s.x1.o, 11); \
    s.x1.e ^= ROTR32(t0.o, 19); \
    s.x1.o ^= ROTR32(t0.e, 20); \
    t0.e    = s.x2.e ^ ROTR32(s.x2.o, 2); \
    t0.o    = s.x2.o ^ ROTR32(s.x2.e, 3); \
    s.x2.e ^= t0.o; \
    s.x2.o ^= ROTR32(t0.e, 1); \
    t0.e    = s.x3.e ^ ROTR32(s.x3.o, 3); \
    t0.o    = s.x3.o ^ ROTR32(s.x3.e, 4); \
    s.x3.e ^= ROTR32(t0.e, 5); \
    s.x3.o ^= ROTR32(t0.o, 5); \
    t0.e    = s.x4.e ^ ROTR32(s.x4.e, 17); \
    t0.o    = s.x4.o ^ ROTR32(s.x4.o, 17); \
    s.x4.e ^= ROTR32(t0.o, 3); \
    s.x4.o ^= ROTR32(t0.e, 4); \
    s.x2.e  = ~s.x2.e;            s.x2.o = ~s.x2.o; \
  } while(0)
/* clang-format on */

void P(state *p, u8 rounds);

#endif  // PERMUTATIONS_H_
