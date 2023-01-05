#define USE64 0
#define MIC 1
#define pi                                                                     \
  3.1415926535897932384626433832795028841971693993751058209749445923078164
#ifndef R123_STATIC_INLINE
#define R123_STATIC_INLINE inline
#endif
#ifndef R123_FORCE_INLINE
#define R123_FORCE_INLINE(decl) decl __attribute__((always_inline))
#endif
#ifndef R123_CUDA_DEVICE
#define R123_CUDA_DEVICE
#endif
#ifndef R123_ASSERT
#define R123_ASSERT(x)
#endif
#ifndef R123_BUILTIN_EXPECT
#define R123_BUILTIN_EXPECT(expr, likely) expr
#endif
#ifndef R123_USE_GNU_UINT128
#define R123_USE_GNU_UINT128 0
#endif
#ifndef R123_USE_MULHILO64_ASM
#define R123_USE_MULHILO64_ASM 0
#endif
#ifndef R123_USE_MULHILO64_MSVC_INTRIN
#define R123_USE_MULHILO64_MSVC_INTRIN 0
#endif
#ifndef R123_USE_MULHILO64_CUDA_INTRIN
#define R123_USE_MULHILO64_CUDA_INTRIN 0
#endif
#ifndef R123_USE_MULHILO64_OPENCL_INTRIN
#define R123_USE_MULHILO64_OPENCL_INTRIN 1
#endif
#ifndef R123_USE_AES_NI
#define R123_USE_AES_NI 0
#endif
#define R123_0x1p_32f (1.f / 4294967296.f)
#define R123_0x1p_24f (1.f / 16777216.f)
#define R123_0x1fffffep_25f (16777215.f * R123_0x1p_24f * R123_0x1p_24f)
#define R123_0x1p_64 (1. / (4294967296. * 4294967296.))
#define R123_0x1p_53 (1. / (4294967296. * 2097152.))
#define R123_0x1fffffffffffffp_54                                              \
  (9007199254740991. * R123_0x1p_53 * R123_0x1p_53)
#define R123_0x1p_32 (1. / 4294967296.)
#define R123_0x100000001p_32 (4294967297. * R123_0x1p_32 * R123_0x1p_32)
// XXX ATI APP SDK 2.4 clBuildProgram SEGVs if one uses uint64_t instead of
// ulong to mul_hi.  And gets lots of complaints from stdint.h
// on some machines.
// But these typedefs mean we cannot include stdint.h with
// these headers?  Do we need R123_64T, R123_32T, R123_8T?
typedef ulong uint64_t;
typedef uint uint32_t;
typedef uchar uint8_t;
enum r123_enum_threefry_wcnt { WCNT2 = 2, WCNT4 = 4 };
R123_CUDA_DEVICE R123_STATIC_INLINE uint64_t RotL_64(uint64_t x, uint64_t N) {
  // rotate(a, b) == a << b. But rotate is weirdly slower on CPUs.
#if MIC
  return rotate(x, (N & 63)) | (x >> ((64 - N) & 63));
#else
  return (x << (N & 63)) | (x >> ((64 - N) & 63));
#endif
}
R123_CUDA_DEVICE R123_STATIC_INLINE uint32_t RotL_32(uint32_t x, uint32_t N) {
  // rotate(a, b) == a << b. But rotate is weirdly slower on CPUs.
#if MIC
  return rotate(x, (N & 31)) | (x >> ((32 - N) & 31));
#else
  return (x << (N & 31)) | (x >> ((32 - N) & 31));
#endif
}
#define SKEIN_MK_64(hi32, lo32) ((lo32) + (((uint64_t)(hi32)) << 32))
#define SKEIN_KS_PARITY64 SKEIN_MK_64(0x1BD11BDA, 0xA9FC1A22)
#define SKEIN_KS_PARITY32 0x1BD11BDA
#define THREEFRY2_DEFAULT_ROUNDS 20
#if USE64
#define ty double
#define SKEIN_KS_PARITY SKEIN_KS_PARITY64
#define RotL RotL_64
#define uint_t uint64_t
#define result(a) (a) * R123_0x1p_64
enum r123_enum_threefry64x2 {
  /*
  // Output from skein_rot_search: (srs64_B64-X1000)
  // Random seed = 1. BlockSize = 128 bits. sampleCnt =  1024. rounds =  8,
  minHW_or=57
  // Start: Tue Mar  1 10:07:48 2011
  // rMin = 0.136. #0325[*15] [CRC=455A682F. hw_OR=64. cnt=16384. blkSize=
  128].format
  */
  R_2_0_0 = 16,
  R_2_1_0 = 42,
  R_2_2_0 = 12,
  R_2_3_0 = 31,
  R_2_4_0 = 16,
  R_2_5_0 = 32,
  R_2_6_0 = 24,
  R_2_7_0 = 21
  /* 4 rounds: minHW =  4  [  4  4  4  4 ]
  // 5 rounds: minHW =  8  [  8  8  8  8 ]
  // 6 rounds: minHW = 16  [ 16 16 16 16 ]
  // 7 rounds: minHW = 32  [ 32 32 32 32 ]
  // 8 rounds: minHW = 64  [ 64 64 64 64 ]
  // 9 rounds: minHW = 64  [ 64 64 64 64 ]
  //10 rounds: minHW = 64  [ 64 64 64 64 ]
  //11 rounds: minHW = 64  [ 64 64 64 64 ] */
};
#else
#define ty float
#define SKEIN_KS_PARITY SKEIN_KS_PARITY32
#define RotL RotL_32
#define uint_t uint32_t
#define result(a) (a) * R123_0x1p_32f
enum r123_enum_threefry32x2 {
  /* Output from skein_rot_search (srs2-X5000.out)
  // Random seed = 1. BlockSize = 64 bits. sampleCnt =  1024. rounds =  8,
  minHW_or=28
  // Start: Tue Jul 12 11:11:33 2011
  // rMin = 0.334. #0206[*07] [CRC=1D9765C0. hw_OR=32. cnt=16384. blkSize=
  64].format   */
  R_2_0_0 = 13,
  R_2_1_0 = 15,
  R_2_2_0 = 26,
  R_2_3_0 = 6,
  R_2_4_0 = 17,
  R_2_5_0 = 29,
  R_2_6_0 = 16,
  R_2_7_0 = 24
  /* 4 rounds: minHW =  4  [  4  4  4  4 ]
  // 5 rounds: minHW =  6  [  6  8  6  8 ]
  // 6 rounds: minHW =  9  [  9 12  9 12 ]
  // 7 rounds: minHW = 16  [ 16 24 16 24 ]
  // 8 rounds: minHW = 32  [ 32 32 32 32 ]
  // 9 rounds: minHW = 32  [ 32 32 32 32 ]
  //10 rounds: minHW = 32  [ 32 32 32 32 ]
  //11 rounds: minHW = 32  [ 32 32 32 32 ] */
};
#endif
struct r123array2 {
  uint_t v[2];
};
typedef struct r123array2 threefry2_ctr_t;
typedef struct r123array2 threefry2_key_t;
typedef struct r123array2 threefry2_ukey_t;
R123_CUDA_DEVICE R123_STATIC_INLINE threefry2_key_t
threefry2keyinit(threefry2_ukey_t uk) {
  return uk;
}
R123_CUDA_DEVICE R123_STATIC_INLINE threefry2_ctr_t
threefry2_R(unsigned int Nrounds, threefry2_ctr_t in, threefry2_key_t k) {
  threefry2_ctr_t X;
  uint_t ks[2 + 1];
  int i; /* avoid size_t to avoid need for stddef.h */
  ks[2] = SKEIN_KS_PARITY;
  for (i = 0; i < 2; i++) {
    ks[i] = k.v[i];
    X.v[i] = in.v[i];
    ks[2] ^= k.v[i];
  }
  /* Insert initial key before round 0 */
  X.v[0] += ks[0];
  X.v[1] += ks[1];
  if (Nrounds > 0) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_0_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 1) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_1_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 2) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_2_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 3) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_3_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 3) {
    /* InjectKey(r=1) */
    X.v[0] += ks[1];
    X.v[1] += ks[2];
    X.v[1] += 1; /* X.v[2-1] += r  */
  }
  if (Nrounds > 4) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_4_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 5) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_5_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 6) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_6_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 7) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_7_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 7) {
    /* InjectKey(r=2) */
    X.v[0] += ks[2];
    X.v[1] += ks[0];
    X.v[1] += 2;
  }
  if (Nrounds > 8) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_0_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 9) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_1_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 10) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_2_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 11) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_3_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 11) {
    /* InjectKey(r=3) */
    X.v[0] += ks[0];
    X.v[1] += ks[1];
    X.v[1] += 3;
  }
  if (Nrounds > 12) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_4_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 13) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_5_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 14) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_6_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 15) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_7_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 15) {
    /* InjectKey(r=4) */
    X.v[0] += ks[1];
    X.v[1] += ks[2];
    X.v[1] += 4;
  }
  if (Nrounds > 16) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_0_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 17) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_1_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 18) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_2_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 19) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_3_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 19) {
    /* InjectKey(r=4) */
    X.v[0] += ks[2];
    X.v[1] += ks[0];
    X.v[1] += 5;
  }
  if (Nrounds > 20) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_0_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 21) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_1_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 22) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_2_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 23) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_3_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 23) {
    /* InjectKey(r=3) */
    X.v[0] += ks[0];
    X.v[1] += ks[1];
    X.v[1] += 6;
  }
  if (Nrounds > 24) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_4_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 25) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_5_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 26) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_6_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 27) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_7_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 27) {
    /* InjectKey(r=4) */
    X.v[0] += ks[1];
    X.v[1] += ks[2];
    X.v[1] += 7;
  }
  if (Nrounds > 28) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_0_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 29) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_1_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 30) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_2_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 31) {
    X.v[0] += X.v[1];
    X.v[1] = RotL(X.v[1], R_2_3_0);
    X.v[1] ^= X.v[0];
  }
  if (Nrounds > 31) {
    /* InjectKey(r=4) */
    X.v[0] += ks[2];
    X.v[1] += ks[0];
    X.v[1] += 8;
  }
  return X;
}
enum r123_enum_threefry2 { threefry2_rounds = THREEFRY2_DEFAULT_ROUNDS };
#define threefry2(c, k) threefry2_R(threefry2_rounds, c, k)
__kernel void randu(__global ty *output, unsigned numel, unsigned counter,
                    unsigned lo, unsigned hi) {
  unsigned tid = get_global_id(0);
  unsigned off = get_global_size(0);
  threefry2_key_t k = {{tid, lo}};
  threefry2_ctr_t c = {{counter, hi}};
  for (int i = tid; i < numel; i += 2 * off) {
    threefry2_ctr_t r = threefry2(c, k);
    c.v[0]++;
    output[i] = result(r.v[0]);
    if (i + off < numel)
      output[i + off] = result(r.v[1]);
  }
}
__kernel void randn(__global ty *output, unsigned numel, unsigned counter,
                    unsigned lo, unsigned hi) {
  unsigned tid = get_global_id(0);
  unsigned off = get_global_size(0);
  threefry2_key_t k = {{tid, 0xdecafbad ^ lo}};
  threefry2_ctr_t c = {{counter, 0xdeadbeef ^ hi}};
  __local ty vals[512];
  unsigned id = get_local_id(0);
  for (int i = tid; i < numel; i += 2 * off) {
    threefry2_ctr_t r = threefry2(c, k);
    c.v[0]++;
    ty u1 = result(r.v[0]);
    ty u2 = result(r.v[1]);
    ty R = sqrt(-2 * log(u1));
    ty T = 2 * pi * u2;
    vals[2 * id] = sin(T);
    vals[2 * id + 1] = cos(T);
    output[i] = vals[id];
    if (i + off < numel)
      output[i + off] = vals[id + 256];
  }
}
__kernel void randi(__global unsigned *output, unsigned numel, unsigned counter,
                    unsigned lo, unsigned hi) {
  unsigned tid = get_global_id(0);
  unsigned off = get_global_size(0);
  threefry2_key_t k = {{tid, 0x12345678 ^ lo}};
  threefry2_ctr_t c = {{counter, 0xdeadbead ^ hi}};
  for (int i = tid; i < numel; i += 2 * off) {
    threefry2_ctr_t r = threefry2(c, k);
    c.v[0]++;
    output[i] = r.v[0];
    if (i + off < numel)
      output[i + off] = r.v[1];
  }
}
