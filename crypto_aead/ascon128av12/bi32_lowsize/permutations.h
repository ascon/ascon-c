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

/* Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
u32_2 to_bit_interleaving(u64 in);

/* Credit to Henry S. Warren, Hacker's Delight, Addison-Wesley, 2002 */
u64 from_bit_interleaving(u32_2 in);

/* clang-format off */
#define ROUND(C_e, C_o) \
  do { \
    /* round constant */ \
    s.x2.e ^= C_e;                        s.x2.o ^= C_o; \
    /* s-box layer */ \
    s.x0.e ^= s.x4.e;                     s.x0.o ^= s.x4.o; \
    s.x4.e ^= s.x3.e;                     s.x4.o ^= s.x3.o; \
    s.x2.e ^= s.x1.e;                     s.x2.o ^= s.x1.o; \
    t0.e = s.x0.e;                        t0.o = s.x0.o; \
    t4.e = s.x4.e;                        t4.o = s.x4.o; \
    t3.e = s.x3.e;                        t3.o = s.x3.o; \
    t1.e = s.x1.e;                        t1.o = s.x1.o; \
    t2.e = s.x2.e;                        t2.o = s.x2.o; \
    s.x0.e = t0.e ^ (~t1.e & t2.e);       s.x0.o = t0.o ^ (~t1.o & t2.o); \
    s.x2.e = t2.e ^ (~t3.e & t4.e);       s.x2.o = t2.o ^ (~t3.o & t4.o); \
    s.x4.e = t4.e ^ (~t0.e & t1.e);       s.x4.o = t4.o ^ (~t0.o & t1.o); \
    s.x1.e = t1.e ^ (~t2.e & t3.e);       s.x1.o = t1.o ^ (~t2.o & t3.o); \
    s.x3.e = t3.e ^ (~t4.e & t0.e);       s.x3.o = t3.o ^ (~t4.o & t0.o); \
    s.x1.e ^= s.x0.e;                     s.x1.o ^= s.x0.o; \
    s.x3.e ^= s.x2.e;                     s.x3.o ^= s.x2.o; \
    s.x0.e ^= s.x4.e;                     s.x0.o ^= s.x4.o; \
    /* linear layer */ \
    t0.e  = s.x0.e ^ ROTR32(s.x0.o, 4);   t0.o  = s.x0.o ^ ROTR32(s.x0.e, 5); \
    t1.e  = s.x1.e ^ ROTR32(s.x1.e, 11);  t1.o  = s.x1.o ^ ROTR32(s.x1.o, 11); \
    t2.e  = s.x2.e ^ ROTR32(s.x2.o, 2);   t2.o  = s.x2.o ^ ROTR32(s.x2.e, 3); \
    t3.e  = s.x3.e ^ ROTR32(s.x3.o, 3);   t3.o  = s.x3.o ^ ROTR32(s.x3.e, 4); \
    t4.e  = s.x4.e ^ ROTR32(s.x4.e, 17);  t4.o  = s.x4.o ^ ROTR32(s.x4.o, 17); \
    s.x0.e ^= ROTR32(t0.o, 9);            s.x0.o ^= ROTR32(t0.e, 10); \
    s.x1.e ^= ROTR32(t1.o, 19);           s.x1.o ^= ROTR32(t1.e, 20); \
    s.x2.e ^= t2.o;                       s.x2.o ^= ROTR32(t2.e, 1); \
    s.x3.e ^= ROTR32(t3.e, 5);            s.x3.o ^= ROTR32(t3.o, 5); \
    s.x4.e ^= ROTR32(t4.o, 3);            s.x4.o ^= ROTR32(t4.e, 4); \
    s.x2.e = ~s.x2.e;                     s.x2.o = ~s.x2.o; \
  } while(0)
/* clang-format on */

void P(state *p, u8 rounds);

#endif /* PERMUTATIONS_H_ */
