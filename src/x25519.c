/*
 * Airbot — Executable Information System
 * x25519.c — X25519 (RFC 7748) scalar multiplication
 *
 * Source: implemented from RFC 7748 §5 using the Montgomery ladder and the
 * radix-2^25.5 field representation from the public-domain ref10/donna
 * approach described in the RFC and in Bernstein's Curve25519 paper.
 *
 * This is NOT a new cryptographic construction. X25519 is a standard,
 * widely reviewed primitive; only the arithmetic is written here because the
 * project has no external dependencies and TinyCC has no crypto library.
 * Correctness is verified against the RFC 7748 §6.1 test vectors AND against
 * an independent implementation (Python `cryptography`, which wraps OpenSSL).
 *
 * Constant-time properties: the ladder is branch-free with respect to the
 * scalar (cswap is arithmetic, not a conditional jump). This mitigates simple
 * timing analysis. It has NOT been validated against cache-timing or
 * power-analysis attacks, and no claim of side-channel resistance is made.
 */

#include "x25519.h"
#include <string.h>

/* Field elements: 10 limbs, radix 2^25.5 (alternating 26/25 bits). */
typedef int32_t fe[10];

static void fe_0(fe h) { memset(h, 0, sizeof(fe)); }
static void fe_1(fe h) { memset(h, 0, sizeof(fe)); h[0] = 1; }

static void fe_copy(fe h, const fe f) { memcpy(h, f, sizeof(fe)); }

static void fe_add(fe h, const fe f, const fe g) {
    int i; for (i = 0; i < 10; i++) h[i] = f[i] + g[i];
}

static void fe_sub(fe h, const fe f, const fe g) {
    int i; for (i = 0; i < 10; i++) h[i] = f[i] - g[i];
}

/* Conditional swap driven by arithmetic on b (0 or 1), never a branch. */
static void fe_cswap(fe f, fe g, unsigned int b) {
    int32_t mask = (int32_t)(-(int32_t)b);
    int i;
    for (i = 0; i < 10; i++) {
        int32_t x = (f[i] ^ g[i]) & mask;
        f[i] ^= x;
        g[i] ^= x;
    }
}

static void fe_mul(fe h, const fe f, const fe g) {
    int64_t f0=f[0],f1=f[1],f2=f[2],f3=f[3],f4=f[4],f5=f[5],f6=f[6],f7=f[7],f8=f[8],f9=f[9];
    int64_t g0=g[0],g1=g[1],g2=g[2],g3=g[3],g4=g[4],g5=g[5],g6=g[6],g7=g[7],g8=g[8],g9=g[9];
    int64_t g1_19=19*g1, g2_19=19*g2, g3_19=19*g3, g4_19=19*g4, g5_19=19*g5;
    int64_t g6_19=19*g6, g7_19=19*g7, g8_19=19*g8, g9_19=19*g9;
    int64_t f1_2=2*f1, f3_2=2*f3, f5_2=2*f5, f7_2=2*f7, f9_2=2*f9;
    int64_t h0,h1,h2,h3,h4,h5,h6,h7,h8,h9;
    int64_t c0,c1,c2,c3,c4,c5,c6,c7,c8,c9;

    h0=f0*g0+f1_2*g9_19+f2*g8_19+f3_2*g7_19+f4*g6_19+f5_2*g5_19+f6*g4_19+f7_2*g3_19+f8*g2_19+f9_2*g1_19;
    h1=f0*g1+f1*g0+f2*g9_19+f3*g8_19+f4*g7_19+f5*g6_19+f6*g5_19+f7*g4_19+f8*g3_19+f9*g2_19;
    h2=f0*g2+f1_2*g1+f2*g0+f3_2*g9_19+f4*g8_19+f5_2*g7_19+f6*g6_19+f7_2*g5_19+f8*g4_19+f9_2*g3_19;
    h3=f0*g3+f1*g2+f2*g1+f3*g0+f4*g9_19+f5*g8_19+f6*g7_19+f7*g6_19+f8*g5_19+f9*g4_19;
    h4=f0*g4+f1_2*g3+f2*g2+f3_2*g1+f4*g0+f5_2*g9_19+f6*g8_19+f7_2*g7_19+f8*g6_19+f9_2*g5_19;
    h5=f0*g5+f1*g4+f2*g3+f3*g2+f4*g1+f5*g0+f6*g9_19+f7*g8_19+f8*g7_19+f9*g6_19;
    h6=f0*g6+f1_2*g5+f2*g4+f3_2*g3+f4*g2+f5_2*g1+f6*g0+f7_2*g9_19+f8*g8_19+f9_2*g7_19;
    h7=f0*g7+f1*g6+f2*g5+f3*g4+f4*g3+f5*g2+f6*g1+f7*g0+f8*g9_19+f9*g8_19;
    h8=f0*g8+f1_2*g7+f2*g6+f3_2*g5+f4*g4+f5_2*g3+f6*g2+f7_2*g1+f8*g0+f9_2*g9_19;
    h9=f0*g9+f1*g8+f2*g7+f3*g6+f4*g5+f5*g4+f6*g3+f7*g2+f8*g1+f9*g0;

    c0=(h0+(int64_t)(1<<25))>>26; h1+=c0; h0-=c0<<26;
    c4=(h4+(int64_t)(1<<25))>>26; h5+=c4; h4-=c4<<26;
    c1=(h1+(int64_t)(1<<24))>>25; h2+=c1; h1-=c1<<25;
    c5=(h5+(int64_t)(1<<24))>>25; h6+=c5; h5-=c5<<25;
    c2=(h2+(int64_t)(1<<25))>>26; h3+=c2; h2-=c2<<26;
    c6=(h6+(int64_t)(1<<25))>>26; h7+=c6; h6-=c6<<26;
    c3=(h3+(int64_t)(1<<24))>>25; h4+=c3; h3-=c3<<25;
    c7=(h7+(int64_t)(1<<24))>>25; h8+=c7; h7-=c7<<25;
    c4=(h4+(int64_t)(1<<25))>>26; h5+=c4; h4-=c4<<26;
    c8=(h8+(int64_t)(1<<25))>>26; h9+=c8; h8-=c8<<26;
    c9=(h9+(int64_t)(1<<24))>>25; h0+=c9*19; h9-=c9<<25;
    c0=(h0+(int64_t)(1<<25))>>26; h1+=c0; h0-=c0<<26;

    h[0]=(int32_t)h0; h[1]=(int32_t)h1; h[2]=(int32_t)h2; h[3]=(int32_t)h3; h[4]=(int32_t)h4;
    h[5]=(int32_t)h5; h[6]=(int32_t)h6; h[7]=(int32_t)h7; h[8]=(int32_t)h8; h[9]=(int32_t)h9;
}

static void fe_sq(fe h, const fe f) { fe_mul(h, f, f); }

/* h = f * 121666, used by the ladder's a24 step. */
static void fe_mul121666(fe h, const fe f) {
    int64_t h0,h1,h2,h3,h4,h5,h6,h7,h8,h9;
    int64_t c0,c1,c2,c3,c4,c5,c6,c7,c8,c9;
    h0=(int64_t)f[0]*121666; h1=(int64_t)f[1]*121666;
    h2=(int64_t)f[2]*121666; h3=(int64_t)f[3]*121666;
    h4=(int64_t)f[4]*121666; h5=(int64_t)f[5]*121666;
    h6=(int64_t)f[6]*121666; h7=(int64_t)f[7]*121666;
    h8=(int64_t)f[8]*121666; h9=(int64_t)f[9]*121666;

    c9=(h9+(int64_t)(1<<24))>>25; h0+=c9*19; h9-=c9<<25;
    c1=(h1+(int64_t)(1<<24))>>25; h2+=c1; h1-=c1<<25;
    c3=(h3+(int64_t)(1<<24))>>25; h4+=c3; h3-=c3<<25;
    c5=(h5+(int64_t)(1<<24))>>25; h6+=c5; h5-=c5<<25;
    c7=(h7+(int64_t)(1<<24))>>25; h8+=c7; h7-=c7<<25;
    c0=(h0+(int64_t)(1<<25))>>26; h1+=c0; h0-=c0<<26;
    c2=(h2+(int64_t)(1<<25))>>26; h3+=c2; h2-=c2<<26;
    c4=(h4+(int64_t)(1<<25))>>26; h5+=c4; h4-=c4<<26;
    c6=(h6+(int64_t)(1<<25))>>26; h7+=c6; h6-=c6<<26;
    c8=(h8+(int64_t)(1<<25))>>26; h9+=c8; h8-=c8<<26;

    h[0]=(int32_t)h0; h[1]=(int32_t)h1; h[2]=(int32_t)h2; h[3]=(int32_t)h3; h[4]=(int32_t)h4;
    h[5]=(int32_t)h5; h[6]=(int32_t)h6; h[7]=(int32_t)h7; h[8]=(int32_t)h8; h[9]=(int32_t)h9;
}

/* out = z^(2^252 - 3), i.e. the inverse via Fermat. */
static void fe_invert(fe out, const fe z) {
    fe t0, t1, t2, t3;
    int i;
    fe_sq(t0, z);
    fe_sq(t1, t0); fe_sq(t1, t1);
    fe_mul(t1, z, t1);
    fe_mul(t0, t0, t1);
    fe_sq(t2, t0);
    fe_mul(t1, t1, t2);
    fe_sq(t2, t1); for (i = 1; i < 5; i++) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t2, t1); for (i = 1; i < 10; i++) fe_sq(t2, t2);
    fe_mul(t2, t2, t1);
    fe_sq(t3, t2); for (i = 1; i < 20; i++) fe_sq(t3, t3);
    fe_mul(t2, t3, t2);
    fe_sq(t2, t2); for (i = 1; i < 10; i++) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t2, t1); for (i = 1; i < 50; i++) fe_sq(t2, t2);
    fe_mul(t2, t2, t1);
    fe_sq(t3, t2); for (i = 1; i < 100; i++) fe_sq(t3, t3);
    fe_mul(t2, t3, t2);
    fe_sq(t2, t2); for (i = 1; i < 50; i++) fe_sq(t2, t2);
    fe_mul(t1, t2, t1);
    fe_sq(t1, t1); for (i = 1; i < 5; i++) fe_sq(t1, t1);
    fe_mul(out, t1, t0);
}

/* Canonical ref10 decoding. Hand-derived bit extraction is error-prone here:
   the high limbs are only exercised by large field elements, so a wrong mask
   stays invisible for small inputs such as the basepoint. */
static int64_t load_3(const uint8_t *in) {
    return (int64_t)in[0] | ((int64_t)in[1] << 8) | ((int64_t)in[2] << 16);
}

static int64_t load_4(const uint8_t *in) {
    return (int64_t)in[0] | ((int64_t)in[1] << 8) |
           ((int64_t)in[2] << 16) | ((int64_t)in[3] << 24);
}

static void fe_frombytes(fe h, const uint8_t s[32]) {
    int64_t h0 = load_4(s);
    int64_t h1 = load_3(s + 4) << 6;
    int64_t h2 = load_3(s + 7) << 5;
    int64_t h3 = load_3(s + 10) << 3;
    int64_t h4 = load_3(s + 13) << 2;
    int64_t h5 = load_4(s + 16);
    int64_t h6 = load_3(s + 20) << 7;
    int64_t h7 = load_3(s + 23) << 5;
    int64_t h8 = load_3(s + 26) << 4;
    int64_t h9 = (load_3(s + 29) & 8388607) << 2;
    int64_t c0,c1,c2,c3,c4,c5,c6,c7,c8,c9;

    c9=(h9+(int64_t)(1<<24))>>25; h0+=c9*19; h9-=c9<<25;
    c1=(h1+(int64_t)(1<<24))>>25; h2+=c1; h1-=c1<<25;
    c3=(h3+(int64_t)(1<<24))>>25; h4+=c3; h3-=c3<<25;
    c5=(h5+(int64_t)(1<<24))>>25; h6+=c5; h5-=c5<<25;
    c7=(h7+(int64_t)(1<<24))>>25; h8+=c7; h7-=c7<<25;
    c0=(h0+(int64_t)(1<<25))>>26; h1+=c0; h0-=c0<<26;
    c2=(h2+(int64_t)(1<<25))>>26; h3+=c2; h2-=c2<<26;
    c4=(h4+(int64_t)(1<<25))>>26; h5+=c4; h4-=c4<<26;
    c6=(h6+(int64_t)(1<<25))>>26; h7+=c6; h6-=c6<<26;
    c8=(h8+(int64_t)(1<<25))>>26; h9+=c8; h8-=c8<<26;

    h[0]=(int32_t)h0; h[1]=(int32_t)h1; h[2]=(int32_t)h2; h[3]=(int32_t)h3; h[4]=(int32_t)h4;
    h[5]=(int32_t)h5; h[6]=(int32_t)h6; h[7]=(int32_t)h7; h[8]=(int32_t)h8; h[9]=(int32_t)h9;
}

static void fe_tobytes(uint8_t s[32], const fe h) {
    int32_t t[10];
    int32_t q, c;
    int i;
    memcpy(t, h, sizeof(t));

    q = (19 * t[9] + (((int32_t)1) << 24)) >> 25;
    q = (t[0] + q) >> 26; q = (t[1] + q) >> 25; q = (t[2] + q) >> 26;
    q = (t[3] + q) >> 25; q = (t[4] + q) >> 26; q = (t[5] + q) >> 25;
    q = (t[6] + q) >> 26; q = (t[7] + q) >> 25; q = (t[8] + q) >> 26;
    q = (t[9] + q) >> 25;

    t[0] += 19 * q;
    c = t[0] >> 26; t[1] += c; t[0] -= c << 26;
    c = t[1] >> 25; t[2] += c; t[1] -= c << 25;
    c = t[2] >> 26; t[3] += c; t[2] -= c << 26;
    c = t[3] >> 25; t[4] += c; t[3] -= c << 25;
    c = t[4] >> 26; t[5] += c; t[4] -= c << 26;
    c = t[5] >> 25; t[6] += c; t[5] -= c << 25;
    c = t[6] >> 26; t[7] += c; t[6] -= c << 26;
    c = t[7] >> 25; t[8] += c; t[7] -= c << 25;
    c = t[8] >> 26; t[9] += c; t[8] -= c << 26;
    c = t[9] >> 25;                t[9] -= c << 25;

    s[0]=(uint8_t)(t[0]>>0);  s[1]=(uint8_t)(t[0]>>8);  s[2]=(uint8_t)(t[0]>>16);
    s[3]=(uint8_t)((t[0]>>24)|(t[1]<<2)); s[4]=(uint8_t)(t[1]>>6); s[5]=(uint8_t)(t[1]>>14);
    s[6]=(uint8_t)((t[1]>>22)|(t[2]<<3)); s[7]=(uint8_t)(t[2]>>5); s[8]=(uint8_t)(t[2]>>13);
    s[9]=(uint8_t)((t[2]>>21)|(t[3]<<5)); s[10]=(uint8_t)(t[3]>>3); s[11]=(uint8_t)(t[3]>>11);
    s[12]=(uint8_t)((t[3]>>19)|(t[4]<<6)); s[13]=(uint8_t)(t[4]>>2); s[14]=(uint8_t)(t[4]>>10);
    s[15]=(uint8_t)(t[4]>>18);
    s[16]=(uint8_t)(t[5]>>0); s[17]=(uint8_t)(t[5]>>8); s[18]=(uint8_t)(t[5]>>16);
    s[19]=(uint8_t)((t[5]>>24)|(t[6]<<1)); s[20]=(uint8_t)(t[6]>>7); s[21]=(uint8_t)(t[6]>>15);
    s[22]=(uint8_t)((t[6]>>23)|(t[7]<<3)); s[23]=(uint8_t)(t[7]>>5); s[24]=(uint8_t)(t[7]>>13);
    s[25]=(uint8_t)((t[7]>>21)|(t[8]<<4)); s[26]=(uint8_t)(t[8]>>4); s[27]=(uint8_t)(t[8]>>12);
    s[28]=(uint8_t)((t[8]>>20)|(t[9]<<6)); s[29]=(uint8_t)(t[9]>>2); s[30]=(uint8_t)(t[9]>>10);
    s[31]=(uint8_t)(t[9]>>18);
}

void x25519_scalarmult(uint8_t out[32], const uint8_t scalar[32], const uint8_t point[32]) {
    uint8_t e[32];
    fe x1, x2, z2, x3, z3, tmp0, tmp1;
    int pos;
    unsigned int swap = 0;

    /* RFC 7748 §5 scalar clamping. */
    memcpy(e, scalar, 32);
    e[0]  &= 248;
    e[31] &= 127;
    e[31] |= 64;

    fe_frombytes(x1, point);
    fe_1(x2); fe_0(z2);
    fe_copy(x3, x1); fe_1(z3);

    /* Montgomery ladder, constant number of iterations. */
    for (pos = 254; pos >= 0; pos--) {
        /* pos >> 3, not pos / 8: TinyCC has no optimizer and emits a real
           `idiv` for the division. The dividend is the PUBLIC loop counter so
           it is not a secret-dependent leak, but idiv is a variable-latency
           instruction and has no place in a crypto inner loop. The shift is
           semantically identical for pos in [0,254]. Verified by re-running
           the RFC 7748 vector, the OpenSSL differential and the machine-code
           audit (the idiv disappears). */
        unsigned int b = (unsigned int)((e[pos >> 3] >> (pos & 7)) & 1);
        swap ^= b;
        fe_cswap(x2, x3, swap);
        fe_cswap(z2, z3, swap);
        swap = b;

        fe_sub(tmp0, x3, z3);
        fe_sub(tmp1, x2, z2);
        fe_add(x2, x2, z2);
        fe_add(z2, x3, z3);
        fe_mul(z3, tmp0, x2);
        fe_mul(z2, z2, tmp1);
        fe_sq(tmp0, tmp1);
        fe_sq(tmp1, x2);
        fe_add(x3, z3, z2);
        fe_sub(z2, z3, z2);
        fe_mul(x2, tmp1, tmp0);
        fe_sub(tmp1, tmp1, tmp0);
        fe_sq(z2, z2);
        fe_mul121666(z3, tmp1);
        fe_sq(x3, x3);
        fe_add(tmp0, tmp0, z3);
        fe_mul(z3, x1, z2);
        fe_mul(z2, tmp1, tmp0);
    }
    fe_cswap(x2, x3, swap);
    fe_cswap(z2, z3, swap);

    fe_invert(z2, z2);
    fe_mul(x2, x2, z2);
    fe_tobytes(out, x2);

    memset(e, 0, sizeof(e));
}

void x25519_base(uint8_t out[32], const uint8_t scalar[32]) {
    static const uint8_t basepoint[32] = { 9 };
    x25519_scalarmult(out, scalar, basepoint);
}

int x25519_is_zero(const uint8_t p[32]) {
    /* All-zero shared secret means a small-order / contributory-behaviour
       failure; RFC 7748 §6.1 requires rejecting it. Compared without an
       early exit. */
    uint8_t acc = 0;
    int i;
    for (i = 0; i < 32; i++) acc |= p[i];
    return acc == 0;
}
