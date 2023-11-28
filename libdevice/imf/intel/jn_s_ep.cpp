/*******************************************************************************
* INTEL CONFIDENTIAL
* Copyright 1996-2022 Intel Corporation.
*
* This software and the related documents are Intel copyrighted  materials,  and
* your use of  them is  governed by the  express license  under which  they were
* provided to you (License).  Unless the License provides otherwise, you may not
* use, modify, copy, publish, distribute,  disclose or transmit this software or
* the related documents without Intel's prior written permission.
*
* This software and the related documents  are provided as  is,  with no express
* or implied  warranties,  other  than those  that are  expressly stated  in the
* License.
*******************************************************************************/
#include "_imf_include_fp32.hpp"
#ifdef __LIBDEVICE_IMF_ENABLED__
namespace __imf_impl_jn_s_ep {
namespace {
typedef union {
  uint64_t u64;
  uint32_t w[2];
} dp_int;
typedef union {
  uint64_t u64[2];
  uint32_t w[4];
} dp_int2;
// Union for fp32 data type
typedef union {
  uint32_t w;
  float f;
} sp_int;
// Function API's
static inline dp_int sp_to_dp(float xin);
static inline float dp_to_sp(dp_int xin);
static inline dp_int dp_add(dp_int xin, dp_int yin);
static inline dp_int dp_sub(dp_int xin, dp_int yin);
static inline dp_int dp_mul(dp_int xin, dp_int yin);
static inline dp_int uint32_to_dp(uint32_t ix);
static inline dp_int uint64_to_dp(uint64_t ix);
static inline dp_int dp_div(dp_int xin, dp_int yin);
// dp_int to fp32 precision convert emulation
// dp_to_sp(x) = (float)x
static inline float dp_to_sp(dp_int xin) {
  dp_int x, y;
  sp_int res, res0;
  uint32_t sgn_x, expon_x, rnd_expon, bit_mask, j;
  uint32_t round_sticky, sticky, r_bit, s_bit, tail;
  x = xin;
  // sign
  sgn_x = x.w[1] & 0x80000000;
  // exponent, biased by 0x3fe
  expon_x = ((((uint32_t)x.w[1]) >> 20) & 0x7ff) - 1;
  // copy mantissa bits to y
  y.w[1] = x.w[1] & 0x000fffff;
  y.w[0] = x.w[0];
  // filter out special cases: denormals, zeroes, Inf/NaN
  if ((uint32_t)expon_x >= 0x7fe)
    goto SPECIAL_DP2SP_PATH;
  // place mantissa bits in position, for rounding
  y.w[1] |= 0x00100000;
  {
    uint32_t _tmp32;
    _tmp32 = (y).w[0] >> (32 - (3));
    (y).w[0] = (y).w[0] << (3);
    (y).w[1] = ((y).w[1] << (3)) | _tmp32;
  };
  // round and sticky bits
  sticky = (y.w[0] & 0x3fffffff) + 0x3fffffff;
  round_sticky = ((uint32_t)y.w[0] | sticky) >> 30;
  // round to SP
  res0.w = y.w[1];
  //
  // round mantissa
  //
  r_bit = round_sticky >> 1;
  s_bit = round_sticky & 1;
  res.w = res0.w + r_bit;
  // round-to-even if (r_bit==1) && (s_bit == 0)
  bit_mask = 0xffffffff ^ (r_bit & (~s_bit));
  res.w &= bit_mask;
  // normalize
  j = (res.w >> 24) & 1;
  res.w >>= j;
  // exponent, biased by 0x7e
  expon_x -= 0x380;
  rnd_expon = expon_x + j;
  // overflow/overflow ?
  if (((uint32_t)rnd_expon) >= 0xfe)
    goto DP2SP_OF_UF;
  res.w = sgn_x ^ ((rnd_expon << 23) + res.w);
  return res.f;
DP2SP_OF_UF:
  // underflow ?
  if (((int32_t)rnd_expon) < 0)
    goto DP2SP_UNDERFLOW;
  res.w = sgn_x ^ 0x7f800000;
  return res.f;
DP2SP_UNDERFLOW:
  if (((int32_t)expon_x) < -24) {
    round_sticky = 1;
    res.w = 0;
  } else {
    // gradual underflow
    // bit tail that is shifted out
    tail = res0.w << (32 + expon_x);
    sticky = ((tail & 0x3fffffff) | round_sticky) + 0x3fffffff;
    round_sticky = ((uint32_t)(tail | sticky)) >> 30;
    res.w = res0.w >> (-expon_x);
  }
  // and now rounding phase
  r_bit = round_sticky >> 1;
  s_bit = round_sticky & 1;
  res.w = res.w + r_bit;
  // round-to-even if (r_bit==1) && (s_bit == 0)
  bit_mask = 0xffffffff ^ (r_bit & (~s_bit));
  res.w &= bit_mask;
  res.w ^= sgn_x;
  return res.f;
SPECIAL_DP2SP_PATH:
  // 0 or denormal?
  if (((int32_t)expon_x) < 0) {
    res.w = sgn_x;
    return res.f;
  }
  // Inf?
  if (!(y.w[1] | y.w[0])) {
    res.w = sgn_x ^ 0x7f800000;
    return res.f;
  }
  // quietize sNaN
  y.w[1] |= 0x00080000;
  // place mantissa bits in SP position
  {
    uint32_t _tmp32;
    _tmp32 = (y).w[0] >> (32 - (3));
    (y).w[0] = (y).w[0] << (3);
    (y).w[1] = ((y).w[1] << (3)) | _tmp32;
  };
  res.w = sgn_x | 0x7f800000 | y.w[1];
  return res.f;
} // float dp_to_sp (dp_int xin)
// fp32 to dp_int precision convert emulation
// sp_to_dp(x) = (dp_int)x
static inline dp_int sp_to_dp(float xin) {
  sp_int x;
  dp_int res;
  uint32_t expon_x, sgn_x, mantissa_x;
  x.f = xin;
  // exponent, biased by 0x7e
  expon_x = ((x.w >> 23) & 0xff) - 1;
  // sign
  sgn_x = x.w & 0x80000000;
  // mantissa (no leading bit)
  mantissa_x = x.w & 0x007fffff;
  // filter out special cases: denormals, zeroes, Inf/NaN
  if ((uint32_t)expon_x >= 0xfe)
    goto SPECIAL_SP2DP_PATH;
MAIN_SP2DP_PATH:
  expon_x += 0x381;
  res.w[1] = mantissa_x;
  res.w[0] = 0;
  // shift mantissa bits into DP position
  {
    uint32_t _tmp32;
    _tmp32 = (res).w[1] << (32 - (3));
    (res).w[1] = (res).w[1] >> (3);
    (res).w[0] = ((res).w[0] >> (3)) | _tmp32;
  };
  res.w[1] |= (sgn_x | (expon_x << 20));
  return res;
SPECIAL_SP2DP_PATH:
  // 0 or denormal
  if (((int32_t)expon_x) < 0) {
    if (!mantissa_x) {
      // return 0
      res.w[1] = sgn_x;
      res.w[0] = 0;
      return res;
    }
    // normalize
    expon_x = 0;
    while (mantissa_x < 0x00800000) {
      mantissa_x += mantissa_x;
      expon_x--;
    }
    mantissa_x &= 0x007fffff;
    goto MAIN_SP2DP_PATH;
  }
  // NaN or Inf
  // Inf?
  if (!mantissa_x) {
    res.w[1] = sgn_x ^ 0x7ff00000;
    res.w[0] = 0;
    return res;
  }
  // NaN
  if (mantissa_x < 0x00400000) {
    // quietize sNaN
    mantissa_x |= 0x00400000;
  }
  expon_x = 0x7fe - 0x380;
  goto MAIN_SP2DP_PATH;
} // dp_int sp_to_dp (float xin)
// dp_add(x,y) = x + y emulation
static inline dp_int dp_add(dp_int xin, dp_int yin) {
  dp_int x, y, a, b, res;
  uint32_t sgn_x, sgn_y, expon_x, expon_y, carry1, carry2, carry3, carry4;
  uint32_t sgn, round_sticky, sticky, r_bit, s_bit, carry, m53, tail, tmp,
      sgn_mask;
  int32_t biased_rnd_expon, shift, bit_mask, x_bits, y_bits, e_diff;
  // inputs
  a = xin;
  b = yin;
  // order inputs: x=maxabs(a, b), y=minabs(a,b)
  {
    uint32_t _tmp_a, _tmp_b;
    _tmp_a = a.w[1] & 0x7fffffff;
    _tmp_b = b.w[1] & 0x7fffffff;
    if ((_tmp_a > _tmp_b) || ((_tmp_a == _tmp_b) && (a.w[0] >= b.w[0]))) {
      x.u64 = a.u64;
      y.u64 = b.u64;
    } else {
      x.u64 = b.u64;
      y.u64 = a.u64;
    }
  };
  // signs
  sgn_x = x.w[1] & 0x80000000;
  sgn_y = y.w[1] & 0x80000000;
  // exponents, biased by 0x3fe
  expon_x = ((((uint32_t)x.w[1]) >> 20) & 0x7ff) - 1;
  expon_y = ((((uint32_t)y.w[1]) >> 20) & 0x7ff) - 1;
  // filter out special cases: denormals, zeroes, Inf/NaN
  if (((uint32_t)expon_x >= 0x7fe) || ((uint32_t)expon_y >= 0x7fe))
    goto SPECIAL_ADD_PATH;
  // x, y will now hold mantissa_x, mantissa_y
  x.w[1] = (x.w[1] & 0x000fffff) | 0x00100000;
  y.w[1] = (y.w[1] & 0x000fffff) | 0x00100000;
ADD_MAIN_PATH:
  //
  // sign, biased exponent (by 0x3fe)
  //
  sgn = sgn_x ^ sgn_y;
  sgn_mask = ((int32_t)sgn) >> 31;
  // apply sign to mantissa_y
  {
    uint32_t _tmp32;
    _tmp32 = (y.w[0]) + (sgn_mask);
    (carry1) = 0;
    if (((uint32_t)(y.w[0])) > _tmp32)
      (carry1) = 1;
    (y.w[0]) = _tmp32;
  };
  y.w[1] += (carry1 + sgn_mask);
  y.w[0] ^= sgn_mask;
  y.w[1] ^= sgn_mask;
  // exponent difference
  e_diff = expon_x - expon_y;
  //
  // mantissa_y = mantissa_y >> e_diff
  // This can take a few branches,
  //  in particular if the HW only supports shift amounts in the range 1..31
  //
  tail = 0;
  if (e_diff) {
    if (e_diff > 55) {
      // |y| << |x|
      {
        uint32_t _tmp32;
        _tmp32 = (x.w[0]) + (sgn_mask);
        (carry2) = 0;
        if (((uint32_t)(x.w[0])) > _tmp32)
          (carry2) = 1;
        (res.w[0]) = _tmp32;
      };
      res.w[1] = x.w[1] + (carry2 + sgn_mask);
      if (res.w[1] < 0x00100000) {
        {
          uint32_t _tmp32;
          _tmp32 = (res).w[0] >> (32 - (1));
          (res).w[0] = (res).w[0] << (1);
          (res).w[1] = ((res).w[1] << (1)) | _tmp32;
        };
        res.w[0] |= (sgn_mask & 1);
        expon_x--;
      }
      round_sticky = (sgn_mask & 2) | 1;
      goto ADD_MAIN_ROUND;
    }
    if (e_diff >= 32) {
      tail = y.w[0];
      y.w[0] = y.w[1];
      y.w[1] = sgn_mask;
      e_diff -= 32;
    }
    // now e_diff < 32
    if (e_diff) {
      shift = 32 - e_diff;
      sticky = tail << shift;
      tail = (((uint32_t)tail) >> e_diff) | (y.w[0] << shift);
      y.w[0] = (((uint32_t)y.w[0]) >> e_diff) | (y.w[1] << shift);
      y.w[1] = ((int32_t)y.w[1]) >> e_diff;
      // add sticky bit to tail
      sticky = (((uint32_t)sticky) >> 1) + 0x7fffffff;
      tail |= (((uint32_t)sticky) >> 31);
    }
  }
  // now add mantissas
  {
    uint32_t _tmp32;
    _tmp32 = (x.w[0]) + (y.w[0]);
    (carry3) = 0;
    if (((uint32_t)(x.w[0])) > _tmp32)
      (carry3) = 1;
    (res.w[0]) = _tmp32;
  };
  res.w[1] = x.w[1] + y.w[1] + carry3;
  // normalize
  if (res.w[1] >= 0x00200000) {
    tail = (tail & 1) | (((uint32_t)tail) >> 1) | (res.w[0] << 31);
    {
      uint32_t _tmp32;
      _tmp32 = (res).w[1] << (32 - (1));
      (res).w[1] = (res).w[1] >> (1);
      (res).w[0] = ((res).w[0] >> (1)) | _tmp32;
    };
    expon_x++;
  } else if (res.w[1] < 0x00100000) {
    {
      uint32_t _tmp32;
      _tmp32 = (res).w[0] >> (32 - (1));
      (res).w[0] = (res).w[0] << (1);
      (res).w[1] = ((res).w[1] << (1)) | _tmp32;
    };
    res.w[0] |= (((uint32_t)tail) >> 31);
    tail <<= 1;
    expon_x--;
    // still not normalized? --> cancellation case
    if (res.w[1] < 0x00100000) {
      // tail == 0 here
      if (!res.w[1]) {
        // result == 0?
        if (!res.w[0])
          goto ZERO_RESULT;
        while (!res.w[1]) {
          {
            uint32_t _tmp32;
            _tmp32 = (res).w[0] >> (32 - (21));
            (res).w[0] = (res).w[0] << (21);
            (res).w[1] = ((res).w[1] << (21)) | _tmp32;
          };
          expon_x -= 21;
        }
      }
      if (res.w[1] < 0x00100000) {
        tmp = res.w[1];
        shift = 0;
        do {
          tmp += tmp;
          shift++;
        } while (tmp < 0x00100000);
        {
          uint32_t _tmp32;
          _tmp32 = (res).w[0] >> (32 - (shift));
          (res).w[0] = (res).w[0] << (shift);
          (res).w[1] = ((res).w[1] << (shift)) | _tmp32;
        };
        expon_x -= shift;
      }
    }
  }
  sticky = (tail & 0x3fffffff) + 0x3fffffff;
  round_sticky = ((uint32_t)(tail | sticky)) >> 30;
  // set Inexact flag if (round_sticky !=0)
ADD_MAIN_ROUND:
  r_bit = round_sticky >> 1;
  s_bit = round_sticky & 1;
  // add with carry-out
  {
    uint32_t _tmp32;
    _tmp32 = (res.w[0]) + (r_bit);
    (carry) = 0;
    if (((uint32_t)(res.w[0])) > _tmp32)
      (carry) = 1;
    (res.w[0]) = _tmp32;
  };
  res.w[1] = res.w[1] + carry;
  // round-to-even if (r_bit==1) && (s_bit == 0)
  bit_mask = 0xffffffff ^ (r_bit & (~s_bit));
  res.w[0] &= bit_mask;
  // normalize if necessary after rounding (i.e. mantisa >= 2^53)
  m53 = res.w[1] >> 21;
  biased_rnd_expon = expon_x + m53;
  if (m53) {
    uint32_t _tmp32;
    _tmp32 = (res).w[1] << (32 - (1));
    (res).w[1] = (res).w[1] >> (1);
    (res).w[0] = ((res).w[0] >> (1)) | _tmp32;
  };
  // overflow/overflow ?
  if (((uint32_t)biased_rnd_expon) >= 0x7fe)
    goto ADD_OF_UF;
  res.w[1] = sgn_x ^ (res.w[1] + (biased_rnd_expon << 20));
  return res;
ZERO_RESULT:
  return res;
ADD_OF_UF:
  if (((int32_t)biased_rnd_expon) >= 0x7fe)
    goto ADD_OVERFLOW;
  // underflow ?
  if (((int32_t)biased_rnd_expon) < 0)
    goto ADD_UNDERFLOW;
ADD_OVERFLOW:
  res.w[1] = sgn_x ^ 0x7ff00000;
  res.w[0] = 0;
  return res;
ADD_UNDERFLOW:
  if (((int32_t)expon_x) <= -32) {
    res.w[0] = res.w[1];
    res.w[1] = 0;
    expon_x += 32;
  }
  if (expon_x) {
    shift = -expon_x;
    {
      uint32_t _tmp32;
      _tmp32 = (res).w[1] << (32 - (shift));
      (res).w[1] = (res).w[1] >> (shift);
      (res).w[0] = ((res).w[0] >> (shift)) | _tmp32;
    };
  }
  res.w[1] = sgn_x ^ res.w[1];
  return res;
SPECIAL_ADD_PATH:
  // process non-Inf/non-NaN cases first
  if (((int32_t)expon_x < 0x7fe) && ((int32_t)expon_y < 0x7fe)) {
    // |y| > 0?  (|x|>=|y|)
    if ((y.w[0] | (y.w[1] & 0x7fffffff))) {
      // y is denormal
      expon_y = 0;
      y.w[1] = y.w[1] & 0x000fffff;
      x.w[1] = x.w[1] & 0x000fffff;
      // x is denormal?
      if (((int32_t)expon_x) < 0)
        expon_x = 0;
      else
        x.w[1] |= 0x00100000;
      goto ADD_MAIN_PATH;
    }
    // x is normal?
    if (((int32_t)expon_x) >= 0)
      return x;
    return x;
  }
  //
  // at least one input is Inf or NaN
  //
  x = xin;
  y = yin;
  // signs
  sgn_x = x.w[1] & 0x80000000;
  sgn_y = y.w[1] & 0x80000000;
  // exponents, biased by 0x3fe
  expon_x = ((((uint32_t)x.w[1]) >> 20) & 0x7ff) - 1;
  expon_y = ((((uint32_t)y.w[1]) >> 20) & 0x7ff) - 1;
  // x is NaN ?
  if ((expon_x == 0x7fe) && (x.w[0] | (x.w[1] & 0x000fffff))) {
    res = x;
    // sNaN?
    if (!(x.w[1] & 0x00080000)) {
      res.w[1] |= 0x00080000;
    }
    // have to check whether y might be SNaN
    return res;
  }
  // y is NaN ?
  if ((expon_y == 0x7fe) && (y.w[0] | (y.w[1] & 0x000fffff))) {
    res = y;
    // sNaN?
    if (!(y.w[1] & 0x00080000)) {
      res.w[1] |= 0x00080000;
    }
    return res;
  }
  // x is Inf?
  if (expon_x == 0x7fe) {
    // y is Inf of opposite sign?
    if ((expon_y == 0x7fe) && (sgn_x ^ sgn_y)) {
      // Inf-Inf, return NaN_Indefinite
      res.w[1] = 0xfff80000;
      res.w[0] = 0;
      return res;
    }
    return x;
  }
  // y is Inf
  return y;
} // dp_int dp_add (dp_int xin, dp_int yin)
// dp_sub(x,y) = x - y emulation
static inline dp_int dp_sub(dp_int xin, dp_int yin) {
  dp_int y;
  uint32_t yh, tmp, mask, carry;
  y = yin;
  yh = y.w[1] & 0x7fffffff;
  // change sign of y, unless it is a NaN
  // generate bit mask to facilitate SIMD execution
  // will switch sign bit if ((yh,y.w[LOW]) - 0x7ff0000000000001 < 0)
  {
    uint32_t _tmp32;
    _tmp32 = (y.w[0]) - (1);
    (carry) = 0;
    if (((uint32_t)(y.w[0])) < _tmp32)
      (carry) = 1;
    (tmp) = _tmp32;
  };
  mask = yh - 0x7ff00000 - carry;
  // retain sign only
  mask &= 0x80000000;
  y.w[1] ^= mask;
  return dp_add(xin, y);
} // dp_int dp_sub (dp_int xin, dp_int yin)
// dp_mul(x,y) = x * y emulation
static inline dp_int dp_mul(dp_int xin, dp_int yin) {
  dp_int x, y, xhyl, xlyh, xlyl, xhyh, Ph, Pl, res;
  uint32_t sgn_x, sgn_y, expon_x, expon_y, carry1, carry2, carry3, carry4;
  uint32_t sgn, round_sticky, sticky, r_bit, s_bit, carry, m53, tail;
  int32_t bit_mask, x_bits, y_bits;
  volatile int32_t biased_expon, biased_rnd_expon, shift;
  // inputs
  x = xin;
  y = yin;
  // signs
  sgn_x = x.w[1] & 0x80000000;
  sgn_y = y.w[1] & 0x80000000;
  // exponents, biased by 0x3fe
  expon_x = ((((uint32_t)x.w[1]) >> 20) & 0x7ff) - 1;
  expon_y = ((((uint32_t)y.w[1]) >> 20) & 0x7ff) - 1;
  // filter out special cases: denormals, zeroes, Inf/NaN
  if (((uint32_t)expon_x >= 0x7fe) || ((uint32_t)expon_y >= 0x7fe))
    goto SPECIAL_MUL_PATH;
MUL_MAIN_PATH:
  //
  // sign, biased exponent (by 0x3fe)
  //
  sgn = sgn_x ^ sgn_y;
  biased_expon = expon_x + expon_y - 0x3fe + 1;
  // x, y will now hold mantissa_x, mantissa_y
  x.w[1] = (x.w[1] & 0x000fffff) | 0x00100000;
  y.w[1] = (y.w[1] & 0x000fffff) | 0x00100000;
  // mantissa_x * 2^11 (shift mantissa_x left by 11)
  // this ensures the mantissa product will be in [2^(51+64), 2^(53+64))
  {
    uint32_t _tmp32;
    _tmp32 = (x).w[0] >> (32 - (11));
    (x).w[0] = (x).w[0] << (11);
    (x).w[1] = ((x).w[1] << (11)) | _tmp32;
  };
  // y.w[HIGH] < 2^21, x.w[HIGH] < 2^32
  //
  // now perform 64x64-bit multiplication
  //
  // x.w[HIGH] * y.w[LOW] < 2^(32+32)
  (xhyl).u64 = (((uint64_t)((uint32_t)(x.w[1]))) * ((uint32_t)(y.w[0])));
  // y.w[HIGH] * x.w[LOW] < 2^(21+32)
  (xlyh).u64 = (((uint64_t)((uint32_t)(y.w[1]))) * ((uint32_t)(x.w[0])));
  // x.w[LOW] * y.w[LOW] < 2^(32+32)
  (xlyl).u64 = (((uint64_t)((uint32_t)(x.w[0]))) * ((uint32_t)(y.w[0])));
  // x.w[HIGH] * y.w[HIGH] < 2^(21+32)
  (xhyh).u64 = (((uint64_t)((uint32_t)(x.w[1]))) * ((uint32_t)(y.w[1])));
  // accumulate products
  Pl.w[0] = xlyl.w[0];
  // add with carry-out
  {
    uint32_t _tmp32;
    _tmp32 = (xlyl.w[1]) + (xlyh.w[0]);
    (carry1) = 0;
    if (((uint32_t)(xlyl.w[1])) > _tmp32)
      (carry1) = 1;
    (Pl.w[1]) = _tmp32;
  };
  // no overflow here
  Ph.w[0] = xlyh.w[1] + carry1;
  // add with carry-out
  {
    uint32_t _tmp32;
    _tmp32 = (Pl.w[1]) + (xhyl.w[0]);
    (carry2) = 0;
    if (((uint32_t)(Pl.w[1])) > _tmp32)
      (carry2) = 1;
    (Pl.w[1]) = _tmp32;
  };
  // no overflow here
  Ph.w[0] = Ph.w[0] + carry2;
  // add with carry-out
  // can also combine this line and the one above into a carry-in, carry-out ADD
  // i.e. ADC32_cout(Ph.w[LOW], carry3, Ph.w[LOW], xhyl.w[HIGH], carry2) where
  // carry2 is in, carry3 is out
  {
    uint32_t _tmp32;
    _tmp32 = (Ph.w[0]) + (xhyl.w[1]);
    (carry3) = 0;
    if (((uint32_t)(Ph.w[0])) > _tmp32)
      (carry3) = 1;
    (Ph.w[0]) = _tmp32;
  };
  // no overflow here
  Ph.w[1] = xhyh.w[1] + carry3;
  // add with carry-out
  {
    uint32_t _tmp32;
    _tmp32 = (Ph.w[0]) + (xhyh.w[0]);
    (carry4) = 0;
    if (((uint32_t)(Ph.w[0])) > _tmp32)
      (carry4) = 1;
    (Ph.w[0]) = _tmp32;
  };
  // no overflow here
  Ph.w[1] = Ph.w[1] + carry4;
  //
  // full product is now in Ph.w[HIGH], Ph.w[LOW], Pl.w[HIGH], Pl.w[LOW]
  // (Ph.w[HIGH], Ph.w[LOW]) in [2^51, 2^53)
  //
  // normalize (Ph.w[HIGH], Ph.w[LOW]) to [2^52, 2^53)
  if (!(Ph.w[1] & 0x000100000)) {
    {
      uint32_t _tmp32;
      _tmp32 = (Ph).w[0] >> (32 - (1));
      (Ph).w[0] = (Ph).w[0] << (1);
      (Ph).w[1] = ((Ph).w[1] << (1)) | _tmp32;
    };
    // least significant mantissa bit
    Ph.w[0] |= (((uint32_t)Pl.w[1]) >> 31);
    // round bit in bit 31 of Pl.w[HIGH]
    Pl.w[1] <<= 1;
    biased_expon--;
  }
  // sticky bits
  sticky = (((uint32_t)Pl.w[0]) >> 30) | ((Pl.w[0] | Pl.w[1]) & 0x3fffffff);
  // will compress into one bit (bit 30)
  sticky = sticky + 0x3fffffff;
  // round_sticky
  round_sticky = ((uint32_t)(Pl.w[1] | sticky)) >> 30;
  r_bit = round_sticky >> 1;
  s_bit = round_sticky & 1;
  // add with carry-out
  {
    uint32_t _tmp32;
    _tmp32 = (Ph.w[0]) + (r_bit);
    (carry) = 0;
    if (((uint32_t)(Ph.w[0])) > _tmp32)
      (carry) = 1;
    (res.w[0]) = _tmp32;
  };
  res.w[1] = Ph.w[1] + carry;
  // round-to-even if (r_bit==1) && (s_bit == 0)
  bit_mask = 0xffffffff ^ (r_bit & (~s_bit));
  res.w[0] &= bit_mask;
  // normalize if necessary after rounding (i.e. mantisa >= 2^53)
  m53 = res.w[1] >> 21;
  biased_rnd_expon = biased_expon + m53;
  if (m53) {
    uint32_t _tmp32;
    _tmp32 = (res).w[1] << (32 - (1));
    (res).w[1] = (res).w[1] >> (1);
    (res).w[0] = ((res).w[0] >> (1)) | _tmp32;
  };
  // overflow/overflow ?
  if (((uint32_t)biased_rnd_expon) >= 0x7fe)
    goto MUL_OF_UF;
  res.w[1] = sgn ^ (res.w[1] + (biased_rnd_expon << 20));
  return res;
MUL_OF_UF:
  if (((int32_t)biased_rnd_expon) >= 0x7fe)
    goto MUL_OVERFLOW;
  // underflow ?
  if (((int32_t)biased_rnd_expon) < 0)
    goto MUL_UNDERFLOW;
MUL_OVERFLOW:
  res.w[1] = sgn ^ 0x7ff00000;
  res.w[0] = 0;
  return res;
MUL_UNDERFLOW:
  if (((int32_t)biased_expon) < -53) {
    round_sticky = 1;
    res.w[1] = res.w[0] = 0;
  } else {
    // gradual underflow
    res.w[0] = Ph.w[0];
    res.w[1] = Ph.w[1];
    if (((int32_t)biased_expon) <= -32) {
      // SHR by 32
      sticky = ((res.w[0] & 0x3fffffff) | round_sticky) + 0x3fffffff;
      round_sticky = ((uint32_t)(res.w[0] | sticky)) >> 30;
      res.w[0] = res.w[1];
      res.w[1] = 0;
      biased_expon += 32;
    }
    // now shift count in [0, 32)
    if (((int32_t)biased_expon) < 0) {
      // bit tail that is shifted out
      tail = res.w[0] << (32 + biased_expon);
      sticky = ((tail & 0x3fffffff) | round_sticky) + 0x3fffffff;
      round_sticky = ((uint32_t)(tail | sticky)) >> 30;
      shift = -biased_expon;
      {
        uint32_t _tmp32;
        _tmp32 = (res).w[1] << (32 - (shift));
        (res).w[1] = (res).w[1] >> (shift);
        (res).w[0] = ((res).w[0] >> (shift)) | _tmp32;
      };
    }
  }
  r_bit = round_sticky >> 1;
  s_bit = round_sticky & 1;
  // add with carry-out
  {
    uint32_t _tmp32;
    _tmp32 = (res.w[0]) + (r_bit);
    (carry) = 0;
    if (((uint32_t)(res.w[0])) > _tmp32)
      (carry) = 1;
    (res.w[0]) = _tmp32;
  };
  res.w[1] = res.w[1] + carry;
  // round-to-even if (r_bit==1) && (s_bit == 0)
  bit_mask = 0xffffffff ^ (r_bit & (~s_bit));
  res.w[0] &= bit_mask;
  res.w[1] = sgn ^ res.w[1];
  return res;
SPECIAL_MUL_PATH:
  // x is NaN ?
  if ((expon_x == 0x7fe) && (x.w[0] | (x.w[1] & 0x000fffff))) {
    res = x;
    // sNaN?
    if (!(x.w[1] & 0x00080000)) {
      res.w[1] |= 0x00080000;
    }
    return res;
  }
  // y is NaN ?
  if ((expon_y == 0x7fe) && (y.w[0] | (y.w[1] & 0x000fffff))) {
    res = y;
    // sNaN?
    if (!(y.w[1] & 0x00080000)) {
      res.w[1] |= 0x00080000;
    }
    return res;
  }
  // x is Inf?
  if (expon_x == 0x7fe) {
    // y is 0?
    if ((((int32_t)expon_y) < 0) && (!(y.w[0] | (y.w[1] & 0x7fffffff)))) {
      // Inf*0, return NaN_Indefinite
      res.w[1] = 0xfff80000;
      res.w[0] = 0;
      return res;
    }
    // return Inf with proper sign
    res = x;
    res.w[1] ^= sgn_y;
    return res;
  }
  // y is Inf?
  if (expon_y == 0x7fe) {
    // x is 0?
    if ((((int32_t)expon_x) < 0) && (!(x.w[0] | (x.w[1] & 0x7fffffff)))) {
      // Inf*0, return NaN_Indefinite
      res.w[1] = 0xfff80000;
      res.w[0] = 0;
      return res;
    }
    // return Inf with proper sign
    res = y;
    res.w[1] ^= sgn_x;
    // set Denormal flag if necessary
    return res;
  }
  // either x or y is 0 or denormal
  x_bits = x.w[0] | (x.w[1] & 0x7fffffff);
  y_bits = y.w[0] | (y.w[1] & 0x7fffffff);
  if (!x_bits || !y_bits) {
    // DAZ or x==0 or y==0
    // return 0 with proper sign
    res.w[1] = sgn_x ^ sgn_y;
    res.w[0] = 0;
    // set Denormal flag if necessary
    return res;
  }
  // x or y may be denormal, but not zero
  // x denormal?
  if (((int32_t)expon_x) < 0) {
    // must count leading zeroes
    if (!(x.w[1] & 0x000fffff)) {
      shift = 21;
      tail = x.w[0];
      while (!(tail & 0x80000000)) {
        tail <<= 1;
        shift++;
      }
    } else {
      tail = x.w[1];
      shift = 0;
      while (!(tail & 0x00100000)) {
        tail <<= 1;
        shift++;
      }
    }
    // update exponent, normalize mantissa
    // initial value (expon_x==-1) has to be corected to expon_x=0
    expon_x = -shift;
    if (shift >= 32) {
      x.w[1] = x.w[0];
      x.w[0] = 0;
      shift -= 32;
    }
    if (shift > 0) {
      uint32_t _tmp32;
      _tmp32 = (x).w[0] >> (32 - (shift));
      (x).w[0] = (x).w[0] << (shift);
      (x).w[1] = ((x).w[1] << (shift)) | _tmp32;
    };
  }
  // y denormal?
  if (((int32_t)expon_y) < 0) {
    // must count leading zeroes
    if (!(y.w[1] & 0x000fffff)) {
      shift = 21;
      tail = y.w[0];
      while (!(tail & 0x80000000)) {
        tail <<= 1;
        shift++;
      }
    } else {
      tail = y.w[1];
      shift = 0;
      while (!(tail & 0x00100000)) {
        tail <<= 1;
        shift++;
      }
    }
    // update exponent, normalize mantissa
    // initial value (expon_y==-1) has to be corected to expon_y=0
    expon_y = -shift;
    if (shift >= 32) {
      y.w[1] = y.w[0];
      y.w[0] = 0;
      shift -= 32;
    }
    if (shift > 0) {
      uint32_t _tmp32;
      _tmp32 = (y).w[0] >> (32 - (shift));
      (y).w[0] = (y).w[0] << (shift);
      (y).w[1] = ((y).w[1] << (shift)) | _tmp32;
    };
  }
  // return to main path
  goto MUL_MAIN_PATH;
} // dp_int dp_mul (dp_int xin, dp_int yin)
// uint32 to dp_int precision convert emulation
// uint32_to_dp(x) = (dp_int)x
static inline dp_int uint32_to_dp(uint32_t ix) {
  dp_int res;
  uint32_t expon_x, lead_bits, shift_amount;
  // x== 0?
  if (!ix) {
    res.w[0] = res.w[1] = 0;
    return res;
  }
  // need log2(|ix|)
  expon_x = 0;
  lead_bits = ix;
  // |input|  >= 2^16 ?
  if (lead_bits >= (1 << 16)) {
    expon_x += 16;
    lead_bits = ((uint32_t)lead_bits) >> 16;
  }
  // leading bits  >= 2^8 ?
  if (lead_bits >= (1 << 8)) {
    expon_x += 8;
    lead_bits = ((uint32_t)lead_bits) >> 8;
  }
  while (lead_bits > 1) {
    expon_x++;
    lead_bits = ((uint32_t)lead_bits) >> 1;
  }
  res.w[0] = ix;
  res.w[1] = 0;
  shift_amount = 52 - expon_x;
  if (shift_amount >= 32) {
    res.w[1] = res.w[0];
    res.w[0] = 0;
    shift_amount -= 32;
  }
  // now shift_amount < 32
  if (shift_amount) {
    uint32_t _tmp32;
    _tmp32 = (res).w[0] >> (32 - (shift_amount));
    (res).w[0] = (res).w[0] << (shift_amount);
    (res).w[1] = ((res).w[1] << (shift_amount)) | _tmp32;
  };
  // add bias - 1
  expon_x += 0x3fe;
  res.w[1] = ((expon_x << 20) + res.w[1]);
  return res;
} // dp_int uint32_to_dp (UINT32 ix)
// uint64 to dp_int precision convert emulation
// uint64_to_dp(x) = (dp_int)x
static inline dp_int uint64_to_dp(uint64_t ix) {
  dp_int xin, res;
  uint32_t expon_x, lead_bits, sticky, round_sticky, r_bit, s_bit, bit_mask,
      carry;
  int shift_amount;
  xin.u64 = (uint64_t)ix;
  // x== 0?
  if (!(xin.w[1] | xin.w[0])) {
    res.w[0] = res.w[1] = 0;
    return res;
  }
  // need log2(|ix|)
  if (!xin.w[1]) {
    expon_x = 0;
    lead_bits = xin.w[0];
  } else {
    expon_x = 32;
    lead_bits = xin.w[1];
  }
  // |input|  >= 2^16 ?
  if (lead_bits >= (1 << 16)) {
    expon_x += 16;
    lead_bits = ((uint32_t)lead_bits) >> 16;
  }
  // leading bits  >= 2^8 ?
  if (lead_bits >= (1 << 8)) {
    expon_x += 8;
    lead_bits = ((uint32_t)lead_bits) >> 8;
  }
  while (lead_bits > 1) {
    expon_x++;
    lead_bits = ((uint32_t)lead_bits) >> 1;
  }
  res.w[0] = xin.w[0];
  res.w[1] = xin.w[1];
  shift_amount = 52 - expon_x;
  if (((int32_t)shift_amount) < 0) {
    // shift amount in (0, 11]
    shift_amount = -shift_amount;
    sticky = res.w[0] << (32 - shift_amount);
    {
      uint32_t _tmp32;
      _tmp32 = (res).w[1] << (32 - (shift_amount));
      (res).w[1] = (res).w[1] >> (shift_amount);
      (res).w[0] = ((res).w[0] >> (shift_amount)) | _tmp32;
    };
    r_bit = ((uint32_t)sticky) >> 31;
    sticky &= 0x7fffffff;
    s_bit = ((uint32_t)(sticky + 0x7fffffff)) >> 31;
    round_sticky = r_bit | s_bit;
    //
    // round mantissa
    //
    // add with carry-out
    {
      uint32_t _tmp32;
      _tmp32 = (res.w[0]) + (r_bit);
      (carry) = 0;
      if (((uint32_t)(res.w[0])) > _tmp32)
        (carry) = 1;
      (res.w[0]) = _tmp32;
    };
    res.w[1] = res.w[1] + carry;
    // round-to-even if (r_bit==1) && (s_bit == 0)
    bit_mask = 0xffffffff ^ (r_bit & (~s_bit));
    res.w[0] &= bit_mask;
  } else {
    if (shift_amount >= 32) {
      res.w[1] = res.w[0];
      res.w[0] = 0;
      shift_amount -= 32;
    }
    // now shift_amount < 32
    if (shift_amount) {
      uint32_t _tmp32;
      _tmp32 = (res).w[0] >> (32 - (shift_amount));
      (res).w[0] = (res).w[0] << (shift_amount);
      (res).w[1] = ((res).w[1] << (shift_amount)) | _tmp32;
    };
  }
  // add bias - 1
  expon_x += 0x3fe;
  res.w[1] = ((expon_x << 20) + res.w[1]);
  return res;
} // dp_int uint64_to_dp (UINT64 ix)
static const uint32_t Inv_TableC0[128] = {
    0x7fffffd, // 0 - 0x7fffffe,
    0x7f01fc0, 0x7e07e08, 0x7d11968,
    0x7c1f079, // 4 - 0x7c1f078,
    0x7b301ea, 0x7a44c6c, 0x795ceb0,
    0x7878786, // 8 - same
    0x77975b9, // 9 - 0x77975ba,
    0x76b981e, 0x75ded92, 0x7507504, 0x7432d64, 0x73615a2, 0x7292cc0, 0x71c71c6,
    0x70fe3be, 0x70381c0, 0x6f74ae0, 0x6eb3e42, 0x6df5b0c, 0x6d3a06c, 0x6c80d90,
    0x6bca1ae, 0x6b15c08, 0x6a63bd8, 0x69b406a, 0x6906908, 0x685b4fc, 0x67b23a6,
    0x670b450, 0x6666666, 0x65c393e, 0x6522c3e, 0x6483ed0, 0x63e7062, 0x634c064,
    0x62b2e44, 0x621b97e, 0x6186186, 0x60f25de, 0x6060606, 0x5fd0180, 0x5f417d0,
    0x5eb4882, 0x5e2931e, 0x5d9f736, 0x5d1745e, 0x5c90a1e, 0x5c0b814, 0x5b87ddc,
    0x5b05b06, 0x5a84f32, 0x5a05a04, 0x5987b1a, 0x590b214, 0x588fea0, 0x5816058,
    0x579d6ec, 0x572620a, 0x56b015c, 0x563b48c, 0x55c7b4c, 0x5555556, 0x54e4254,
    0x54741fa, 0x5405402, 0x539782a, 0x532ae22, 0x52bf5a8, 0x5254e7a, 0x51eb852,
    0x51832f2, 0x511be18,
    0x50b5989, // 75 - 0x50b598a,
    0x5050506, 0x4fec050, 0x4f88b2e, 0x4f26566, 0x4ec4ec4, 0x4e6470a, 0x4e04e06,
    0x4da637c, 0x4d4873e,
    0x4ceb917, // 85 - 0x4ceb918,
    0x4c8f8d2, 0x4c3463e, 0x4bda130, 0x4b80970, 0x4b27ed2, 0x4ad012c,
    0x4a79049, // 92 - 0x4a79048,
    0x4a22c04, 0x49cd430, 0x497889e, 0x4924924,
    0x48d159d, // 97 - 0x48d159c,
    0x487ede0, 0x482d1c0, 0x47dc120, 0x478bbce, 0x473c1aa, 0x46ed290, 0x469ee58,
    0x46514de, 0x4604602, 0x45b81a2, 0x456c798, 0x45217c4, 0x44d7204,
    0x448d639, // 111 - 0x448d638,
    0x4444444, 0x43fbc04, 0x43b3d5a, 0x436c82c, 0x4325c54, 0x42df9ba, 0x429a044,
    0x4254fce, 0x4210842, 0x41cc984, 0x4189376, 0x41465fe, 0x4104104, 0x40c2470,
    0x4081020, 0x4040404};
static const uint32_t Inv_TableC1[128] = {
    0xc000a00, // 0 - 0xc000800,
    0xc0fd800, 0xc1f4c00, 0xc2e6400,
    0xc3d2800, // 4 - same
    0xc4b9400, 0xc59ac00, 0xc677800,
    0xc74f400, // 8 - same
    0xc822400, // 9 - same
    0xc8f0c00, 0xc9bb000, 0xca80c00, 0xcb42400, 0xcbffc00, 0xccb9400, 0xcd6f000,
    0xce21000, 0xcecf400, 0xcf7a000, 0xd021400, 0xd0c5400, 0xd165c00, 0xd203400,
    0xd29dc00, 0xd335000, 0xd3c9800, 0xd45b000, 0xd4e9c00, 0xd576000, 0xd5ff400,
    0xd686400, 0xd70a800, 0xd78c400, 0xd80bc00, 0xd889000, 0xd903c00, 0xd97c400,
    0xd9f2c00, 0xda67000, 0xdad9400, 0xdb49800, 0xdbb7c00, 0xdc24000, 0xdc8e800,
    0xdcf7000, 0xdd5e000, 0xddc3000, 0xde26400, 0xde88000, 0xdee8000, 0xdf46400,
    0xdfa3000, 0xdffe800, 0xe058400, 0xe0b0800, 0xe107800, 0xe15cc00, 0xe1b1000,
    0xe203c00, 0xe255400, 0xe2a5400, 0xe2f4400, 0xe342000, 0xe38e400, 0xe3d9800,
    0xe423c00, 0xe46cc00, 0xe4b4800, 0xe4fb400, 0xe541000, 0xe585800, 0xe5c9400,
    0xe60bc00, 0xe64d800,
    0xe68e200, // 75 - 0xe68e000,
    0xe6cdc00, 0xe70c800, 0xe74a800, 0xe787800, 0xe7c3800, 0xe7fec00, 0xe839000,
    0xe872c00, 0xe8ab800,
    0xe8e3600, // 85 - 0xe8e3400,
    0xe91a800, 0xe951000, 0xe986800, 0xe9bb800, 0xe9efc00, 0xea23000,
    0xea56000, // 92 - same
    0xea88000, 0xeab9400, 0xeaea000, 0xeb1a400,
    0xeb49a00, // 97 - 0xeb49c00,
    0xeb78800, 0xeba6c00, 0xebd4400, 0xec01400, 0xec2dc00, 0xec59800, 0xec84c00,
    0xecaf800, 0xecd9c00, 0xed03400, 0xed2c600, 0xed55000, 0xed7d000,
    0xeda4a00, // 111- 0xeda4c00,
    0xedcbc00, 0xedf2400, 0xee18800, 0xee3e000, 0xee63400, 0xee88000, 0xeeac400,
    0xeed0000, 0xeef3800, 0xef16800, 0xef39000, 0xef5b400, 0xef7d000, 0xef9e400,
    0xefbf400, 0xefdfc00};
// dp_div(x,y) = x / y emulation
static inline dp_int dp_div(dp_int xin, dp_int yin) {
  dp_int x, y, y0, res, res1, Q_y;
  int index;
  uint32_t sgn_x, sgn_y, expon_x, expon_y, C0, C1, f, fC1, R, eps0, R_eps, eps;
  uint32_t sgn, round_sticky, sticky, r_bit, s_bit, carry, m53, tail;
  int32_t biased_expon, biased_rnd_expon, bit_mask, sgn_eps, shift, x_bits,
      y_bits;
  uint64_t R_eps_64, Q, Q_eps;
  // inputs
  x = xin;
  y = yin;
  // signs
  sgn_x = x.w[1] & 0x80000000;
  sgn_y = y.w[1] & 0x80000000;
  // exponents, biased by 0x3fe
  expon_x = ((((uint32_t)x.w[1]) >> 20) & 0x7ff) - 1;
  expon_y = ((((uint32_t)y.w[1]) >> 20) & 0x7ff) - 1;
  // filter out special cases: denormals, zeroes, Inf/NaN
  if (((uint32_t)expon_x >= 0x7fe) || ((uint32_t)expon_y >= 0x7fe))
    goto SPECIAL_DIV_PATH;
DIV_MAIN_PATH:
  //
  // sign, biased exponent (by 0x3fe)
  //
  sgn = sgn_x ^ sgn_y;
  biased_expon = expon_x - expon_y + 0x3fe;
  // x, y will now hold mantissa_x, mantissa_y
  x.w[1] = (x.w[1] & 0x000fffff) | 0x00100000;
  y.w[1] = (y.w[1] & 0x000fffff) | 0x00100000;
  index = (y.w[1] >> (20 - 7)) & 0x7f;
  // sc 2^30
  C0 = Inv_TableC0[index] << 3;
  // scale 2^31
  C1 = Inv_TableC1[index] << 5;
  // y0.w[HIGH] holds upper 32 bits of mant_y
  {
    uint32_t _tmp32;
    _tmp32 = (y).w[0] >> (32 - (11));
    (y0).w[0] = (y).w[0] << (11);
    (y0).w[1] = ((y).w[1] << (11)) | _tmp32;
  };
  // fractional bits, sc 2^31
  f = y0.w[1] & 0x00ffff00;
  // 32x32 MUL, sc 2^62
  // but note f has only 16 nonzero bits, so it could be rewritten as a 16x32
  // MUL + shifts if needed sc 2^30, as only high 32 bits are retained
  fC1 = ((((int64_t)((int32_t)(f))) * ((int32_t)(C1))) >> 32);
  // starting RCP approx, sc 2^30
  // constant added to boost accuracy
  R = C0 + fC1 + 0x5448;
  // |y0.w[HIGH]*R - 1|*2^(31+30-32) < 2^(31+30-14-32) = 2^15
  // eps0, sc 2^29
  eps0 = ((((uint64_t)((uint32_t)(y0.w[1]))) * ((uint32_t)(R))) >> 32) -
         0x20000000;
  // R - R*eps, will be accurate to ~28 bits
  // R*eps, 32x16 MUL, sc 2^(30+29)=2^59
  R_eps_64 = (((int64_t)((int32_t)(eps0))) * ((int32_t)(R)));
  // R*eps, sc 2^31
  R_eps = (uint32_t)(((int64_t)R_eps_64) >> 28);
  // R - R*eps, sc 2^31
  R = R + R - R_eps;
  // mant_x, sc 2^62
  {
    uint32_t _tmp32;
    _tmp32 = (x).w[0] >> (32 - (10));
    (x).w[0] = (x).w[0] << (10);
    (x).w[1] = ((x).w[1] << (10)) | _tmp32;
  };
  // Q = x*R, sc 2^(62+31-32)=2^61
  // 32x64-bit MUL
  Q = (((uint64_t)((uint32_t)(x.w[1]))) * ((uint32_t)(R))) +
      ((((uint64_t)((uint32_t)(x.w[0]))) * ((uint32_t)(R))) >> 32);
  // mant_y, sc 2^59
  {
    uint32_t _tmp32;
    _tmp32 = (y0).w[1] << (32 - (4));
    (y0).w[1] = (y0).w[1] >> (4);
    (y0).w[0] = ((y0).w[0] >> (4)) | _tmp32;
  };
  // |y*R - 1|*2^(31+59) < ~2^(31+59-27)=2^63
  // 32x64-bit MUL (partial)
  // high 32 bits of y_low*R + low 32 bits of y_high*R
  // scale is  2^(31+59-32)=2^58, |eps| < 2^31
  eps = ((((uint64_t)((uint32_t)(y0.w[0]))) * ((uint32_t)(R))) >> 32) +
        (((uint32_t)y0.w[1]) * ((uint32_t)R));
  // sign mask of eps
  sgn_eps = ((int32_t)eps) >> 31;
  // Q*eps, sc 2^(61+58-32) = 2^(61+26)
  Q_eps =
      (((int64_t)((int32_t)((int32_t)(Q >> 32)))) * ((int32_t)(eps))) +
      (int64_t)(((((uint64_t)((uint32_t)((uint32_t)Q))) * ((uint32_t)(eps))) >>
                 32) -
                (sgn_eps & (uint32_t)Q));
  // Q - Q*eps, sc 2^61
  Q = Q - (((int64_t)Q_eps) >> 26);
  // sc 2^61
  res.u64 = Q;
  // normalize if Q<1
  if (res.w[1] < 0x20000000) {
    {
      uint32_t _tmp32;
      _tmp32 = (res).w[0] >> (32 - (1));
      (res).w[0] = (res).w[0] << (1);
      (res).w[1] = ((res).w[1] << (1)) | _tmp32;
    };
    {
      uint32_t _tmp32;
      _tmp32 = (y0).w[1] << (32 - (1));
      (y0).w[1] = (y0).w[1] >> (1);
      (y0).w[0] = ((y0).w[0] >> (1)) | _tmp32;
    };
    biased_expon--;
  }
  // add 2^(-53) to quotient estimate
  {
    uint32_t _tmp32;
    _tmp32 = (res.w[0]) + (0x100);
    (carry) = 0;
    if (((uint32_t)(res.w[0])) > _tmp32)
      (carry) = 1;
    (res.w[0]) = _tmp32;
  };
  res.w[1] += carry;
  // truncate to 1 + 52 bits, sc 2^52
  {
    uint32_t _tmp32;
    _tmp32 = (res).w[1] << (32 - (9));
    (res).w[1] = (res).w[1] >> (9);
    (res).w[0] = ((res).w[0] >> (9)) | _tmp32;
  };
  // |Q*y - x| < y*2^(-52)
  // so |Q*y - x|*2^(59+52) < 2^60
  // get lower 64 bits of Q*y  (64x64-bit MUL)
  Q_y.u64 = (((uint64_t)((uint32_t)(res.w[0]))) * ((uint32_t)(y0.w[0])));
  Q_y.w[1] += (((uint32_t)res.w[0]) * ((uint32_t)y0.w[1]));
  Q_y.w[1] += (((uint32_t)res.w[1]) * ((uint32_t)y0.w[0]));
  // subtract lower 64 bits of x*2^(59+52)
  Q_y.w[1] -= (x.w[0] << 17);
  // exact quotient?
  if (!(Q_y.w[1] | Q_y.w[0]))
    round_sticky = 0;
  else {
    // y*2^(-53+59+52)
    {
      uint32_t _tmp32;
      _tmp32 = (y0).w[1] << (32 - (1));
      (y0).w[1] = (y0).w[1] >> (1);
      (y0).w[0] = ((y0).w[0] >> (1)) | _tmp32;
    };
    // sticky=1 for all normal inexact cases
    round_sticky = 1;
    // based on sign of (Q*y-x), add or subtract y*2^(-53)
    if (((int32_t)Q_y.w[1]) < 0) {
      {
        uint32_t _tmp32;
        _tmp32 = (Q_y.w[0]) + (y0.w[0]);
        (carry) = 0;
        if (((uint32_t)(Q_y.w[0])) > _tmp32)
          (carry) = 1;
        (Q_y.w[0]) = _tmp32;
      };
      Q_y.w[1] = Q_y.w[1] + y0.w[1] + carry;
    } else {
      {
        uint32_t _tmp32;
        _tmp32 = (Q_y.w[0]) - (y0.w[0]);
        (carry) = 0;
        if (((uint32_t)(Q_y.w[0])) < _tmp32)
          (carry) = 1;
        (Q_y.w[0]) = _tmp32;
      };
      Q_y.w[1] = Q_y.w[1] - y0.w[1] - carry;
      // also adjust leading mantissa bits
      {
        uint32_t _tmp32;
        _tmp32 = (res.w[0]) - (1);
        (carry) = 0;
        if (((uint32_t)(res.w[0])) < _tmp32)
          (carry) = 1;
        (res.w[0]) = _tmp32;
      };
      res.w[1] -= carry;
    }
    // set round bit if needed, based on sign of Q_y.w[HIGH]
    round_sticky |= ((Q_y.w[1] >> 30) & 2);
  }
  r_bit = round_sticky >> 1;
  s_bit = round_sticky & 1;
  // add with carry-out
  {
    uint32_t _tmp32;
    _tmp32 = (res.w[0]) + (r_bit);
    (carry) = 0;
    if (((uint32_t)(res.w[0])) > _tmp32)
      (carry) = 1;
    (res1.w[0]) = _tmp32;
  };
  res1.w[1] = res.w[1] + carry;
  // round-to-even if (r_bit==1) && (s_bit == 0)
  bit_mask = 0xffffffff ^ (r_bit & (~s_bit));
  res1.w[0] &= bit_mask;
  // normalize if necessary after rounding (i.e. mantisa >= 2^53)
  m53 = res1.w[1] >> 21;
  biased_rnd_expon = biased_expon + m53;
  if (m53) {
    uint32_t _tmp32;
    _tmp32 = (res1).w[1] << (32 - (1));
    (res1).w[1] = (res1).w[1] >> (1);
    (res1).w[0] = ((res1).w[0] >> (1)) | _tmp32;
  };
  // overflow/overflow ?
  if (((uint32_t)biased_rnd_expon) >= 0x7fe)
    goto DIV_OF_UF;
  res1.w[1] = sgn ^ (res1.w[1] + (biased_rnd_expon << 20));
  return res1;
DIV_OF_UF:
  if (((int32_t)biased_rnd_expon) >= 0x7fe)
    goto DIV_OVERFLOW;
  // underflow ?
  if (((int32_t)biased_rnd_expon) < 0)
    goto DIV_UNDERFLOW;
DIV_OVERFLOW:
  res.w[1] = sgn ^ 0x7ff00000;
  res.w[0] = 0;
  return res;
DIV_UNDERFLOW:
  if (((int32_t)biased_expon) < -53) {
    round_sticky = 1;
    res.w[1] = res.w[0] = 0;
  } else {
    // gradual underflow
    if (((int32_t)biased_expon) <= -32) {
      // SHR by 32
      sticky = ((res.w[0] & 0x3fffffff) | round_sticky) + 0x3fffffff;
      round_sticky = ((uint32_t)(res.w[0] | sticky)) >> 30;
      res.w[0] = res.w[1];
      res.w[1] = 0;
      biased_expon += 32;
    }
    // now shift count in [0, 32)
    if (((int32_t)biased_expon) < 0) {
      // bit tail that is shifted out
      tail = res.w[0] << (32 + biased_expon);
      sticky = ((tail & 0x3fffffff) | round_sticky) + 0x3fffffff;
      round_sticky = ((uint32_t)(tail | sticky)) >> 30;
      shift = -biased_expon;
      {
        uint32_t _tmp32;
        _tmp32 = (res).w[1] << (32 - (shift));
        (res).w[1] = (res).w[1] >> (shift);
        (res).w[0] = ((res).w[0] >> (shift)) | _tmp32;
      };
    }
  }
  // and now rounding phase
  r_bit = round_sticky >> 1;
  s_bit = round_sticky & 1;
  // add with carry-out
  {
    uint32_t _tmp32;
    _tmp32 = (res.w[0]) + (r_bit);
    (carry) = 0;
    if (((uint32_t)(res.w[0])) > _tmp32)
      (carry) = 1;
    (res.w[0]) = _tmp32;
  };
  res.w[1] = res.w[1] + carry;
  // round-to-even if (r_bit==1) && (s_bit == 0)
  bit_mask = 0xffffffff ^ (r_bit & (~s_bit));
  res.w[0] &= bit_mask;
  res.w[1] = sgn ^ res.w[1];
  return res;
SPECIAL_DIV_PATH:
  // x is NaN ?
  if ((expon_x == 0x7fe) && (x.w[0] | (x.w[1] & 0x000fffff))) {
    res = x;
    // sNaN?
    if (!(x.w[1] & 0x00080000)) {
      res.w[1] |= 0x00080000;
    }
    return res;
  }
  // y is NaN ?
  if ((expon_y == 0x7fe) && (y.w[0] | (y.w[1] & 0x000fffff))) {
    res = y;
    // sNaN?
    if (!(y.w[1] & 0x00080000)) {
      res.w[1] |= 0x00080000;
    }
    return res;
  }
  // x is Inf?
  if (expon_x == 0x7fe) {
    // y is Inf?
    if (expon_y == 0x7fe) {
      res.w[1] = 0xfff80000;
      res.w[0] = 0;
      return res;
    }
    // return Inf with proper sign
    res = x;
    res.w[1] ^= sgn_y;
    return res;
  }
  // y is Inf?
  if (expon_y == 0x7fe) {
    // return 0 with proper sign
    res.w[1] = sgn_x ^ sgn_y;
    res.w[0] = 0;
    return res;
  }
  // y is 0?
  if ((((int32_t)expon_y) < 0) && (!(y.w[0] | (y.w[1] & 0x7fffffff)))) {
    // x is 0?
    if ((((int32_t)expon_x) < 0) && (!(x.w[0] | (x.w[1] & 0x7fffffff)))) {
      // Inf*0, return NaN_Indefinite
      res.w[1] = 0xfff80000;
      res.w[0] = 0;
      return res;
    }
    // return Inf with proper sign
    res.w[0] = 0;
    res.w[1] = sgn_x ^ sgn_y ^ 0x7ff00000;
    return res;
  }
  // either x is 0 or denormal, or y denormal
  x_bits = x.w[0] | (x.w[1] & 0x7fffffff);
  y_bits = y.w[0] | (y.w[1] & 0x7fffffff);
  if (((((int32_t)expon_x) < 0)) || !x_bits) {
    // DAZ or x==0
    // return 0 with proper sign
    res.w[1] = sgn_x ^ sgn_y;
    res.w[0] = 0;
    return res;
  }
  // x or y may be denormal, but not zero
  // x denormal?
  if (((int32_t)expon_x) < 0) {
    // must count leading zeroes
    if (!(x.w[1] & 0x000fffff)) {
      shift = 21;
      tail = x.w[0];
      while (!(tail & 0x80000000)) {
        tail <<= 1;
        shift++;
      }
    } else {
      tail = x.w[1];
      shift = 0;
      while (!(tail & 0x00100000)) {
        tail <<= 1;
        shift++;
      }
    }
    // update exponent, normalize mantissa
    // initial value (expon_x==-1) has to be corected to expon_x=0
    expon_x = -shift;
    if (shift >= 32) {
      x.w[1] = x.w[0];
      x.w[0] = 0;
      shift -= 32;
    }
    if (shift > 0) {
      uint32_t _tmp32;
      _tmp32 = (x).w[0] >> (32 - (shift));
      (x).w[0] = (x).w[0] << (shift);
      (x).w[1] = ((x).w[1] << (shift)) | _tmp32;
    };
  }
  // y denormal?
  if (((int32_t)expon_y) < 0) {
    // must count leading zeroes
    if (!(y.w[1] & 0x000fffff)) {
      shift = 21;
      tail = y.w[0];
      while (!(tail & 0x80000000)) {
        tail <<= 1;
        shift++;
      }
    } else {
      tail = y.w[1];
      shift = 0;
      while (!(tail & 0x00100000)) {
        tail <<= 1;
        shift++;
      }
    }
    // update exponent, normalize mantissa
    // initial value (expon_y==-1) has to be corected to expon_y=0
    expon_y = -shift;
    if (shift >= 32) {
      y.w[1] = y.w[0];
      y.w[0] = 0;
      shift -= 32;
    }
    if (shift > 0) {
      uint32_t _tmp32;
      _tmp32 = (y).w[0] >> (32 - (shift));
      (y).w[0] = (y).w[0] << (shift);
      (y).w[1] = ((y).w[1] << (shift)) | _tmp32;
    };
  }
  // return to main path
  goto DIV_MAIN_PATH;
} // dp_int dp_div (dp_int xin, dp_int yin)
/* file: _vsj0_kernel_cout.i */
/* file: _vssincos_hl_kernel_cout.i */
// unsigned 64-bit shift
// signed 64-bit shift
static const int32_t __sjn_ep___ip_h = 0x0517CC1B;
static const int32_t __sjn_ep___ip_m = 0x727220A9;
static const int32_t __sjn_ep___ip_m2 = 0x4FE13ABE;
static const int32_t __sjn_ep___ip_l = 0x48;
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc5 = {0xbcd07725u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc5l = {0x300699f8u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc4 = {0x3e70f3e7u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc4l = {0xb08f2818u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc3 = {0xbfaae9ddu};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc3l = {0xb2479ff1u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc2 = {0x4081e0f8u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc2l = {0x33e9c2f2u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc1 = {0xc09de9e6u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc1l = {0xb41bd2c9u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cc0l = {0xa97a0000u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs5 = {0xbbeea72au};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs5l = {0x2e34aadau};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs4 = {0x3da838dfu};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs4l = {0x3153b5efu};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs3 = {0xbf196963u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs3l = {0xb1a62812u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs2 = {0x402335e3u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs2l = {0x3361fa23u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs1 = {0xc0a55de7u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs1l = {0xb3c4aef7u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs0 = {0x40490fdbu};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs0l = {0xb3bbbd30u};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___cs_small = {0xBE2AAAABu};
static const union {
  uint32_t w;
  float f;
} __sjn_ep___twom24 = {0x33800000u};
static const uint32_t invpi_tbl[] = {0,          0x28BE60DB, 0x9391054A,
                                     0x7F09D5F4, 0x7D4D3770, 0x36D8A566,
                                     0x4F10E410, 0x7F9458EA, 0xF7AEF158};
static inline int __sjn_sincos_hl_ep_kernel_fp32(float xin, int n, float *psin,
                                                 float *pcos) {
  uint64_t IP, IP2, IP3;
  int64_t lN = (int64_t)n;
  int64_t IP_s, IP2_s, IP3_s;
  int32_t ip_low_s;
  uint32_t ip_low, ex;
  int_float x, Rh, Rl, res, scale, spoly, cpoly;
  int_float cpoly_h, spoly_h, cpoly_l, spoly_l, cres_h, sres_h, cres_l, sres_l;
  int mx, sgn_x, ex1, ip_h, shift, index, j, sgn_p, sgn_xp;
  float High, Low, R2h, R2l, Ph, Pl;
  x.f = xin;
  mx = (x.w & 0x007fffff) | 0x00800000;
  sgn_x = x.w & 0x80000000;
  ex = ((x.w ^ sgn_x) >> 23);
  // redirect large or very small inputs
  if (__builtin_expect(((unsigned)(ex - 0x7f + 11)) >= (16 + 11), (0 == 1))) {
    // small input (disabled)
    // if (__builtin_expect((ex < (0x7f - 12)), (1 == 1)))
    // if (__builtin_expect((ex < 0x7f - 11), (1 == 1)))
    //{
    //    psin[0] = xin;  psin[1] = _VSTATIC(__cs_small).f * xin*xin*xin;
    //    pcos[0] = 1.0f;  pcos[1] = -0.5f * xin*xin;
    //    return;
    //}
    // Inf/NaN
    if (ex == 0xff) // if (xa.w >= 0x7f800000)
    {
      x.w |= 0x00400000;
      psin[0] = pcos[0] = x.f;
      psin[1] = pcos[1] = x.f;
      return 0;
    }
    ex1 = ex - 0x7f - 23;
    index = 1 + (ex1 >> 5);
    j = ex1 & 0x1f; // expon % 32
    // x/Pi, scaled by 2^(63-j)
    ip_low = (((uint32_t)invpi_tbl[index]) * ((uint32_t)mx));
    IP = (((uint64_t)((uint32_t)(invpi_tbl[index + 1]))) * ((uint32_t)(mx))) +
         (((uint64_t)ip_low) << 32);
    // scaled by 2^(127-j)
    IP3 = (((uint64_t)((uint32_t)(invpi_tbl[index + 3]))) * ((uint32_t)(mx)));
    // scaled by 2^(95-j)
    IP2 = (((uint64_t)((uint32_t)(invpi_tbl[index + 2]))) * ((uint32_t)(mx))) +
          (IP3 >> 32);
    IP = IP + (IP2 >> 32);
    IP3 = (uint32_t)IP3;
    // scale 2^63
    IP <<= j;
    // shift low part by 32-j, j in [0,31]
    ip_low = (uint32_t)IP2;
    // IP3 scale:  2^127
    IP3 |= (((uint64_t)ip_low) << 32);
    IP3 <<= j;
    ip_low >>= (31 - j);
    ip_low >>= 1;
    IP |= (uint64_t)ip_low;
  } else // main path
  {
    // products are really unsigned; operands are small enough so that signed
    // MUL works as well x*2^(23-ex)*(1/Pi)*2^28 p[k] products fit in 31 bits
    // each
    IP_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sjn_ep___ip_h)));
    IP = (uint64_t)IP_s;
    IP2_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sjn_ep___ip_m)));
    IP2 = (uint64_t)IP2_s;
    IP3_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sjn_ep___ip_m2)));
    IP3 = (uint64_t)IP3_s;
    // (x/Pi)*2^63
    IP <<= (ex - 0x7f + 12);
    IP += (IP2 >> (20 + 0x7f - ex));
    IP += (IP3 >> (52 + 0x7f - ex));
    IP2 <<= (44 - 0x7f + ex);
    // additional  low bits
    IP3 <<= (12 - 0x7f + ex);
    IP3 += IP2;
    IP = (IP3 < IP2) ? (IP + 1) : IP;
  }
  // add correction by n
  IP += (uint64_t)(lN << 61);
  // return to 32-bit, scale 2^31
  ip_h = IP >> 32;
  // fix sign bit
  sgn_xp = ((ip_h + 0x20000000) & 0xc0000000);
  // reduced argument (signed, high-low), scale 2^32
  ip_h <<= 2;
  Rh.f = (float)ip_h;
  // reduced argument will need to be normalized
  shift = 2 + 30 + 0x7f - ((Rh.w >> 23) & 0xff);
  // correction for shift=0
  shift = (shift >= 2) ? shift : 2;
  // normalize
  IP = (IP << shift) | (IP3 >> (64 - shift));
  ip_h = IP >> 32;
  Rh.f = (float)ip_h;
  ip_h -= ((int)Rh.f);
  ip_low = (uint32_t)IP;
  ip_h = (ip_h << 24) | (ip_low >> 8);
  Rl.f = (float)ip_h;
  // adjust scale
  scale.w = (0x7f - 31 - shift) << 23;
  Rh.f = __fma(Rh.f, scale.f, 0.0f);
  Rl.f = __fma(Rl.f, scale.f, 0.0f);
  // scale Rl by 2^(-24)
  Rl.f = __fma(Rl.f, __sjn_ep___twom24.f, 0.0f);
  //(Rh+Rl)^2
  R2h = Rh.f;
  R2l = Rl.f;
  {
    float __ph, __phl;
    __ph = __fma(R2h, Rh.f, 0.0f);
    __phl = __fma(R2h, Rh.f, -__ph);
    R2l = __fma(R2l, Rh.f, __phl);
    R2l = __fma(R2h, Rl.f, R2l);
    R2h = __ph;
  };
  // polynomial evaluation
  spoly_h.f = __sjn_ep___cs5.f;
  spoly_l.f = __sjn_ep___cs5l.f;
  cpoly_h.f = __sjn_ep___cc5.f;
  cpoly_l.f = __sjn_ep___cc5l.f;
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __sjn_ep___cs4.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cs4.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __sjn_ep___cs4l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __sjn_ep___cc4.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cc4.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __sjn_ep___cc4l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __sjn_ep___cs3.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cs3.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __sjn_ep___cs3l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __sjn_ep___cc3.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cc3.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __sjn_ep___cc3l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __sjn_ep___cs2.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cs2.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __sjn_ep___cs2l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __sjn_ep___cc2.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cc2.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __sjn_ep___cc2l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __sjn_ep___cs1.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cs1.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __sjn_ep___cs1l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __sjn_ep___cc1.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cc1.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __sjn_ep___cc1l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, R2h, 0.0f);
    __phl = __fma(spoly_h.f, R2h, -__ph);
    spoly_l.f = __fma(spoly_l.f, R2h, __phl);
    spoly_l.f = __fma(spoly_h.f, R2l, spoly_l.f);
    spoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(spoly_h.f, 1.0f, __sjn_ep___cs0.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cs0.f);
    __ahl = __fma(spoly_h.f, 1.0f, -__ahh);
    spoly_l.f = (spoly_l.f + __sjn_ep___cs0l.f) + __ahl;
    spoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(cpoly_h.f, R2h, 0.0f);
    __phl = __fma(cpoly_h.f, R2h, -__ph);
    cpoly_l.f = __fma(cpoly_l.f, R2h, __phl);
    cpoly_l.f = __fma(cpoly_h.f, R2l, cpoly_l.f);
    cpoly_h.f = __ph;
  };
  {
    float __ph, __ahl, __ahh;
    __ph = __fma(cpoly_h.f, 1.0f, __sjn_ep___cc0.f);
    __ahh = __fma(__ph, 1.0f, -__sjn_ep___cc0.f);
    __ahl = __fma(cpoly_h.f, 1.0f, -__ahh);
    cpoly_l.f = (cpoly_l.f + __sjn_ep___cc0l.f) + __ahl;
    cpoly_h.f = __ph;
  };
  {
    float __ph, __phl;
    __ph = __fma(spoly_h.f, Rh.f, 0.0f);
    __phl = __fma(spoly_h.f, Rh.f, -__ph);
    spoly_l.f = __fma(spoly_l.f, Rh.f, __phl);
    spoly_l.f = __fma(spoly_h.f, Rl.f, spoly_l.f);
    spoly_h.f = __ph;
  };
  sgn_p = sgn_xp & 0x80000000;
  // adjust sign
  cpoly_h.w ^= sgn_p;
  cpoly_l.w ^= sgn_p;
  sgn_p = sgn_p ^ (sgn_xp << 1);
  spoly_h.w ^= sgn_p;
  spoly_l.w ^= sgn_p;
  sres_h.w = (sgn_xp & 0x40000000) ? cpoly_h.w : spoly_h.w;
  cres_h.w = (sgn_xp & 0x40000000) ? spoly_h.w : cpoly_h.w;
  sres_l.w = (sgn_xp & 0x40000000) ? cpoly_l.w : spoly_l.w;
  cres_l.w = (sgn_xp & 0x40000000) ? spoly_l.w : cpoly_l.w;
  sres_h.w ^= sgn_x;
  sres_l.w ^= sgn_x;
  pcos[0] = cres_h.f;
  pcos[1] = cres_l.f;
  psin[0] = sres_h.f;
  psin[1] = sres_l.f;
  return 0;
}
/* polynomial minimax approximation of J0(x) = 1+y*Q2(y), y=x^2, |x|=[0..2^(-2)]
 */
static const float __sjn_j0f_ep_fQ2[] = {
    -0x1p-2f, //           -0.25 [0xbe800000]  -.24999999999999192312749585e-00
              //           [0xbfcffffffffffedd]
    0x1p-6f,  //        0.015625 [0x3c800000]   .15624999995864828628011622e-01
              //        [0x3f8fffffffdba06c]
    -0x1.c71c5cp-12f, //  -0.00043402746 [0xb9e38e2e]
                      //  -.43402744692099296756199189e-03 [0xbf3c71c5b0a9df3a]
    0x1.c68ae6p-18f,  //   6.7732121e-06 [0x36e34573]
                      //   .67732122096095540643738083e-05 [0x3edc68ae653d320e]
};
/* polynomial minimax approximation of J0(x) = 1+y*Q3(y), y=x^2, |x|=[0..2^(-8)]
 */
static const float __sjn_j0f_ep_fQ3[] = {
    -0x1p-2f, //           -0.25 [0xbe800000] -.2499999999999873712130948e-00
              //           [0xbfcffffffffffe39]
    0x1.fffff2p-7f, //     0.015624993 [0x3c7ffff9]
                    //     .1562499337726327118536229e-01 [0x3f8fffff1c71caab]
};
/* polynomial minimax approximation of J0(x) = Q1(x-z0) on interval (0,s1). */
static const float __sjn_j0f_ep_fZ0[] = {
    0x1.33d152p+1f,  //       2.4048254 [0x4019e8a9]
    0x1.d2e368p-24f, //    1.087059e-07 [0x33e971b4] Low part
};
static const float __sjn_j0f_ep_fQ1[] = {
    -0x1.19b792p-54f, //  -6.1087652e-17 [0xa48cdbc9]
    -0x1.09cdb4p-1f,  //     -0.51914752 [0xbf04e6da]
    0x1.ba1deep-4f,   //       0.1079387 [0x3ddd0ef7]
    0x1.cfae86p-5f,   //     0.056601774 [0x3d67d743]
    -0x1.1bb1ccp-7f,  //   -0.0086576696 [0xbc0dd8e6]
    -0x1.1f9926p-9f,  //   -0.0021942004 [0xbb0fcc93]
    0x1.153838p-12f,  //   0.00026437722 [0x398a9c1c]
    0x1.6ed3dcp-15f,  //   4.3729255e-05 [0x383769ee]
    -0x1.232edcp-18f, //  -4.3389655e-06 [0xb691976e]
    -0x1.1cd86p-21f,  //  -5.3056556e-07 [0xb50e6c30]
    0x1.805988p-25f,  //   4.4744198e-08 [0x33402cc4]
    0x1.2bcba6p-28f,  //   4.3625987e-09 [0x3195e5d3]
    -0x1.5c973cp-32f, //  -3.1704145e-10 [0xafae4b9e]
    -0x1.1327d2p-35f, //  -3.1281564e-11 [0xae0993e9]
};
/* polynomial minimax approximation of J0(x) = P1(x-z1) on interval (s1,s2). */
static const float __sjn_j0f_ep_fZ1[] = {
    0x1.6148f6p+2f,   //       5.5200782 [0x40b0a47b]
    -0x1.34f46ep-24f, //  -7.1934146e-08 [0xb39a7a37] Low part
};
static const float __sjn_j0f_ep_fP1[] = {
    -0x1.fbb40ap-56f, //   -2.752265e-17 [0xa3fdda05]
    0x1.5c6e6p-2f,    //       0.3402648 [0x3eae3730]
    -0x1.f8f72ep-6f,  //    -0.030820651 [0xbcfc7b97]
    -0x1.b2150cp-5f,  //    -0.052988552 [0xbd590a86]
    0x1.2f7ffcp-8f,   //    0.0046310415 [0x3b97bffe]
    0x1.27e31ap-9f,   //    0.0022574395 [0x3b13f18d]
    -0x1.6f63b8p-13f, //  -0.00017518498 [0xb937b1dc]
    -0x1.863d84p-15f, //  -4.6520268e-05 [0xb8431ec2]
    0x1.ad60e8p-19f,  //   3.1991194e-06 [0x3656b074]
    0x1.3296cep-21f,  //   5.7106666e-07 [0x35194b67]
    -0x1.2b4334p-25f, //  -3.4838742e-08 [0xb315a19a]
    -0x1.34006ap-28f, //  -4.4820134e-09 [0xb19a0035]
    0x1.d368acp-33f,  //   2.1255295e-10 [0x2f69b456]
};
/* polynomial minimax approximation of J0(x) = P2(x-z2) on interval (s2,s3). */
static const float __sjn_j0f_ep_fZ2[] = {
    0x1.14eb56p+3f,  //       8.6537275 [0x410a75ab]
    0x1.999bdap-22f, //   3.8147792e-07 [0x34cccded] Low part
};
static const float __sjn_j0f_ep_fP2[] = {
    -0x1.6e8eecp-54f, //  -7.9484659e-17 [0xa4b74776]
    -0x1.15f798p-2f,  //     -0.27145231 [0xbe8afbcc]
    0x1.00f7fcp-6f,   //     0.015684124 [0x3c807bfe]
    0x1.68b984p-5f,   //     0.044033773 [0x3d345cc2]
    -0x1.48e634p-9f,  //    -0.002509302 [0xbb24731a]
    -0x1.0e0d5ap-9f,  //   -0.0020603344 [0xbb0706ad]
    0x1.d79582p-14f,  //   0.00011243439 [0x38ebcac1]
    0x1.77feb8p-15f,  //   4.4822096e-05 [0x383bff5c]
    -0x1.3315c8p-19f, //  -2.2879622e-06 [0xb6198ae4]
    -0x1.309016p-21f, //  -5.6729249e-07 [0xb518480b]
    0x1.cbc15ap-26f,  //   2.6761279e-08 [0x32e5e0ad]
    0x1.36bdecp-28f,  //   4.5218895e-09 [0x319b5ef6]
    -0x1.85c2dep-33f, //  -1.7724287e-10 [0xaf42e16f]
};
/* polynomial minimax approximation of J0(x) = P3(x-z3) on interval (s3,s4). */
static const float __sjn_j0f_ep_fZ3[] = {
    0x1.79544p+3f,   //       11.791534 [0x413caa20]
    0x1.04e56cp-26f, //   1.5186156e-08 [0x328272b6] Low part
};
static const float __sjn_j0f_ep_fP3[] = {
    -0x1.2d8ed4p-54f, //  -6.5389951e-17 [0xa496c76a]
    0x1.dc13e6p-3f,   //      0.23245983 [0x3e6e09f3]
    -0x1.42ff0cp-7f,  //   -0.0098570641 [0xbc217f86]
    -0x1.38d1dep-5f,  //     -0.03818601 [0xbd1c68ef]
    0x1.a55e98p-10f,  //    0.0016073971 [0x3ad2af4c]
    0x1.e2e164p-10f,  //    0.0018420427 [0x3af170b2]
    -0x1.3dfbd8p-14f, //  -7.5813237e-05 [0xb89efdec]
    -0x1.5ce5e8p-15f, //  -4.1591891e-05 [0xb82e72f4]
    0x1.bb020ep-20f,  //   1.6503335e-06 [0x35dd8107]
    0x1.22ed3ap-21f,  //   5.4189314e-07 [0x3511769d]
    -0x1.5ee806p-26f, //  -2.0425437e-08 [0xb2af7403]
    -0x1.2eb18p-28f,  //  -4.4047681e-09 [0xb19758c0]
    0x1.39fb7p-33f,   //   1.4278256e-10 [0x2f1cfdb8]
};
/* polynomial minimax approximation of J0(x) = P4(x-z4) on interval (s4,s5). */
static const float __sjn_j0f_ep_fZ4[] = {
    0x1.ddca14p+3f,   //       14.930918 [0x416ee50a]
    -0x1.0d8e2ep-25f, //  -3.1380377e-08 [0xb306c717] Low part
};
static const float __sjn_j0f_ep_fP4[] = {
    -0x1.50be2ep-53f, //  -1.4603895e-16 [0xa5285f17]
    -0x1.a701dp-3f,   //     -0.20654643 [0xbe5380e8]
    0x1.c54b92p-8f,   //    0.0069167358 [0x3be2a5c9]
    0x1.17798ap-5f,   //     0.034115572 [0x3d0bbcc5]
    -0x1.2a214ep-10f, //   -0.0011372761 [0xba9510a7]
    -0x1.b541ecp-10f, //    -0.001668005 [0xbadaa0f6]
    0x1.cc0b46p-15f,  //   5.4841523e-05 [0x386605a3]
    0x1.41f1aep-15f,  //   3.8378723e-05 [0x3820f8d7]
    -0x1.4b127ap-20f, //    -1.23334e-06 [0xb5a5893d]
    -0x1.11cc2ap-21f, //  -5.0998761e-07 [0xb508e615]
    0x1.0fd2b2p-26f,  //   1.5822183e-08 [0x3287e959]
    0x1.219112p-28f,  //   4.2137498e-09 [0x3190c889]
    -0x1.f88bbp-34f,  //   -1.147204e-10 [0xaefc45d8]
};
/* polynomial pade approximation P0(x) = PP(256/x^2) in point 256/x^2 = 0.5 */
static const float __sjn_j0f_ep_fPP[] = {
    0x1p+0f,          //               1 [0x3f800000]
    -0x1.2p-12f,      //   -0.0002746582 [0xb9900000]
    0x1.cb5f86p-20f,  //   1.7112983e-06 [0x35e5afc3]
    -0x1.24f578p-25f, //    -3.41049e-08 [0xb3127abc]
    0x1.7ca5eep-30f,  //   1.3847899e-09 [0x30be52f7]
    -0x1.47a91p-34f,  //   -7.450135e-11 [0xaea3d488]
};
/* polynomial pade approximation Q0(x) = QP(256/x^2)*(16/x)) in point 256/x^2 =
 * 0.5 */
static const float __sjn_j0f_ep_fQP[] = {
    -0x1p-7f,         //      -0.0078125 [0xbc000000]
    0x1.2cp-16f,      //   1.7881393e-05 [0x37960000]
    -0x1.d11ca8p-23f, //  -2.1658462e-07 [0xb4688e54]
    0x1.b9d68ep-28f,  //   6.4295906e-09 [0x31dceb47]
    -0x1.7a8362p-32f, //  -3.4425576e-10 [0xafbd41b1]
    0x1.845fecp-36f,  //   2.2076545e-11 [0x2dc22ff6]
};
static const float __sjn_j0f_ep_fPP_MP[] = {
    0x1.fffffep-1f,
    0x1.ffffcp-25f, // HI + LO:      0.99999994 +   5.9604531e-08 [0x3f7fffff +
                    // 0x337fffe0]
    -0x1.1ffffep-12f,
    -0x1.e81b16p-36f, // HI + LO:  -0.00027465817 +  -2.7745603e-11 [0xb98fffff
                      // + 0xadf40d8b]
    0x1.cb5f86p-20f,
    0x1.449b0ep-45f, // HI + LO:   1.7112983e-06 +    3.603847e-14 [0x35e5afc3 +
                     // 0x29224d87]
    -0x1.24f578p-25f,
    -0x1.081276p-50f, // HI + LO:    -3.41049e-08 +  -9.1618419e-16 [0xb3127abc
                      // + 0xa684093b]
    0x1.7ca5eep-30f,
    0x1.0136f8p-55f, // HI + LO:   1.3847899e-09 +   2.7887276e-17 [0x30be52f7 +
                     // 0x24009b7c]
    -0x1.47a91p-34f,
    -0x1.1e8214p-59f, // HI + LO:   -7.450135e-11 +  -1.9414545e-18 [0xaea3d488
                      // + 0xa20f410a]
};
static const float __sjn_j0f_ep_fQP_MP[] = {
    -0x1.fffffep-8f,
    -0x1.fff4e4p-32f, // HI + LO:   -0.0078124995 +  -4.6562182e-10 [0xbbffffff
                      // + 0xaffffa72]
    0x1.2bfffep-16f,
    0x1.790014p-40f, // HI + LO:   1.7881392e-05 +   1.3393741e-12 [0x3795ffff +
                     // 0x2bbc800a]
    -0x1.d11ca8p-23f,
    -0x1.2ce594p-49f, // HI + LO:  -2.1658462e-07 +  -2.0878909e-15 [0xb4688e54
                      // + 0xa71672ca]
    0x1.b9d68ep-28f,
    0x1.5d6bdcp-54f, // HI + LO:   6.4295906e-09 +   7.5768672e-17 [0x31dceb47 +
                     // 0x24aeb5ee]
    -0x1.7a8362p-32f,
    -0x1.e14eecp-58f, // HI + LO:  -3.4425576e-10 +  -6.5229437e-18 [0xafbd41b1
                      // + 0xa2f0a776]
    0x1.845fecp-36f,
    0x1.b972dap-62f, // HI + LO:   2.2076545e-11 +   3.7392154e-19 [0x2dc22ff6 +
                     // 0x20dcb96d]
};
static const float __sjn_j0f_ep_fptonpi_MP[] = {
    0x1.45f306p-1f, 0x1.b9391p-26f // HI + LO:      0.63661975 +   2.5682553e-08
                                   // [0x3f22f983 + 0x32dc9c88]
};
static inline int __sjn_j0_ep_kernel_fp32(const float *a, float *r) {
  int nRet = 0;
  float xf = *a;
  uint32_t ix = ((*(uint32_t *)&xf) & ~0x80000000);
  float fax = __fabs(xf);
  if (ix < 0x7f800000) /* finite x */
  {
    if (ix < 0x4183c3da) /* 0 <= |x| < 16.4706306457 */
    {
      if (ix < 0x40753aac) /* 0 <= |x| < 3.83170604 */
      {
        if (ix < 0x3e800000) /* 0 <= |x| < 2^(-2) */
        {
          if (ix < 0x3b800000) /* 0 <= |x| < 2^(-8) */
          {
            if (ix < 0x38800000) /* 0 <= |x| < 2^(-14) */
            {
              if (ix < 0x32000000) /* 0 <= |x| < 2^(-27) */
              {
                *r = (1.0f - fax);
                return nRet;
              } else /* 2^(-27) <= |x| < 2^(-14) */
              {
                *r = (1.0f - xf * xf);
                return nRet;
              }
            } else /* 2^(-14) <= |x| < 2^(-8) */
            {
              float fx, fy, fresult;
              fx = xf, fy = fx * fx;
              fresult =
                  (__sjn_j0f_ep_fQ3[1] * fy + __sjn_j0f_ep_fQ3[0]) * fy + 1.0f;
              *r = fresult;
              return nRet;
            }
          } else /* 2^(-8) <= |x| < 2^(-2) */
          {
            float fx, fy, fz, fresult;
            fx = xf, fy = fx * fx, fz = fy * fy;
            fresult = (__sjn_j0f_ep_fQ2[3] * fz + __sjn_j0f_ep_fQ2[1]) * fz +
                      (__sjn_j0f_ep_fQ2[2] * fz + __sjn_j0f_ep_fQ2[0]) * fy +
                      1.0f;
            *r = fresult;
            return nRet;
          }
        } else /* 2^(-2) <= |x| < 3.8317060470 */
        {
          float pax = fax, px, py, pz, pp, pq, presult;
          px = pax - __sjn_j0f_ep_fZ0[0];
          px = px - __sjn_j0f_ep_fZ0[1];
          presult =
              __sjn_j0f_ep_fQ1[0] +
              px *
                  (__sjn_j0f_ep_fQ1[1] +
                   px *
                       (__sjn_j0f_ep_fQ1[2] +
                        px *
                            (__sjn_j0f_ep_fQ1[3] +
                             px *
                                 (__sjn_j0f_ep_fQ1[4] +
                                  px *
                                      (__sjn_j0f_ep_fQ1[5] +
                                       px *
                                           (__sjn_j0f_ep_fQ1[6] +
                                            px *
                                                (__sjn_j0f_ep_fQ1[7] +
                                                 px *
                                                     (__sjn_j0f_ep_fQ1[8] +
                                                      px *
                                                          (__sjn_j0f_ep_fQ1[9] +
                                                           px *
                                                               (__sjn_j0f_ep_fQ1
                                                                    [10] +
                                                                px *
                                                                    (__sjn_j0f_ep_fQ1
                                                                         [11] +
                                                                     px *
                                                                         (__sjn_j0f_ep_fQ1
                                                                              [12] +
                                                                          px *
                                                                              __sjn_j0f_ep_fQ1
                                                                                  [13]))))))))))));
          *r = (float)presult;
          return nRet;
        }
      } else /* 3.831706047 <= |x| < 16.4706306457 */
      {
        const float *pP;
        const float *pZ;
        float px, py, pz, pp, pq, presult, pax = fax;
        if (ix < 0x4122c687) /* 3.83170604705 <= |x| < 10.173468589 */
        {
          if (ix < 0x40e07fb0) /* 7.0155868 */
          {
            pP = __sjn_j0f_ep_fP1;
            pZ = __sjn_j0f_ep_fZ1;
          } else {
            pP = __sjn_j0f_ep_fP2;
            pZ = __sjn_j0f_ep_fZ2;
          }
        } else /* 10.173468589782714843750 <= |x| < 16.4706306457519531250 */
        {
          if (ix < 0x41552dd8) /* 13.323692321777343750 */
          {
            pP = __sjn_j0f_ep_fP3;
            pZ = __sjn_j0f_ep_fZ3;
          } else {
            pP = __sjn_j0f_ep_fP4;
            pZ = __sjn_j0f_ep_fZ4;
          }
        }
        px = pax - pZ[0];
        px = px - pZ[1];
        presult =
            pP[0] +
            px *
                (pP[1] +
                 px *
                     (pP[2] +
                      px *
                          (pP[3] +
                           px *
                               (pP[4] +
                                px *
                                    (pP[5] +
                                     px *
                                         (pP[6] +
                                          px *
                                              (pP[7] +
                                               px *
                                                   (pP[8] +
                                                    px *
                                                        (pP[9] +
                                                         px *
                                                             (pP[10] +
                                                              px *
                                                                  (pP[11] +
                                                                   px *
                                                                       (pP[12]))))))))))));
        *r = (float)presult;
        return nRet;
      }
    } else /* finite |x| >= 16.4706306457519 Hancels asymptotic forms */
    {
      const float ptonpi = 0x1.45f306p-1f; // 0.63661975 [0x3f22f983] 2/Pi
      float px = fax, pxi = 1.0f / px, py = 16.0f * pxi, pz = py * py, pp, pq,
            presult, ps, pc, psq = __sqrt(ptonpi * pxi);
      float mp_s[2], mp_c[2], mp_pp[2], mp_qp[2], mp_res[2], mp_pz[2], mp_py[2],
          mp_pxi[2];
      __sjn_sincos_hl_ep_kernel_fp32(fax, -1, &mp_s[0], &mp_c[0]);
      {
        mp_pxi[0] = 1.0f / px;
        mp_pxi[1] = __fma(-(mp_pxi[0]), px, 1.0f);
        mp_pxi[1] = __fma(mp_pxi[0], mp_pxi[1], 0.0f);
      };
      mp_py[0] = mp_pxi[0] * 16.0f;
      mp_py[1] = mp_pxi[1] * 16.0f;
      ;
      {
        mp_pz[0] = __fma(mp_py[0], mp_py[0], 0.0f);
        mp_pz[1] = __fma(mp_py[0], mp_py[0], -mp_pz[0]);
        mp_pz[1] = __fma(mp_py[1], mp_py[0], mp_pz[1]);
        mp_pz[1] = __fma(mp_py[0], mp_py[1], mp_pz[1]);
      };
      mp_pp[0] = __sjn_j0f_ep_fPP_MP[10];
      mp_pp[1] = __sjn_j0f_ep_fPP_MP[11];
      {
        float __ph, __phl;
        __ph = __fma(mp_pp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_pp[0], mp_pz[0], -__ph);
        mp_pp[1] = __fma(mp_pp[1], mp_pz[0], __phl);
        mp_pp[1] = __fma(mp_pp[0], mp_pz[1], mp_pp[1]);
        mp_pp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[8]))
                   ? (__sjn_j0f_ep_fPP_MP[8])
                   : (mp_pp[0]);
        __ah = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[8]))
                   ? (mp_pp[0])
                   : (__sjn_j0f_ep_fPP_MP[8]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_pp[1] = (mp_pp[1] + __sjn_j0f_ep_fPP_MP[9]) + __ahl;
        mp_pp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_pp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_pp[0], mp_pz[0], -__ph);
        mp_pp[1] = __fma(mp_pp[1], mp_pz[0], __phl);
        mp_pp[1] = __fma(mp_pp[0], mp_pz[1], mp_pp[1]);
        mp_pp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[6]))
                   ? (__sjn_j0f_ep_fPP_MP[6])
                   : (mp_pp[0]);
        __ah = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[6]))
                   ? (mp_pp[0])
                   : (__sjn_j0f_ep_fPP_MP[6]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_pp[1] = (mp_pp[1] + __sjn_j0f_ep_fPP_MP[7]) + __ahl;
        mp_pp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_pp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_pp[0], mp_pz[0], -__ph);
        mp_pp[1] = __fma(mp_pp[1], mp_pz[0], __phl);
        mp_pp[1] = __fma(mp_pp[0], mp_pz[1], mp_pp[1]);
        mp_pp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[4]))
                   ? (__sjn_j0f_ep_fPP_MP[4])
                   : (mp_pp[0]);
        __ah = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[4]))
                   ? (mp_pp[0])
                   : (__sjn_j0f_ep_fPP_MP[4]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_pp[1] = (mp_pp[1] + __sjn_j0f_ep_fPP_MP[5]) + __ahl;
        mp_pp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_pp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_pp[0], mp_pz[0], -__ph);
        mp_pp[1] = __fma(mp_pp[1], mp_pz[0], __phl);
        mp_pp[1] = __fma(mp_pp[0], mp_pz[1], mp_pp[1]);
        mp_pp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[2]))
                   ? (__sjn_j0f_ep_fPP_MP[2])
                   : (mp_pp[0]);
        __ah = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[2]))
                   ? (mp_pp[0])
                   : (__sjn_j0f_ep_fPP_MP[2]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_pp[1] = (mp_pp[1] + __sjn_j0f_ep_fPP_MP[3]) + __ahl;
        mp_pp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_pp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_pp[0], mp_pz[0], -__ph);
        mp_pp[1] = __fma(mp_pp[1], mp_pz[0], __phl);
        mp_pp[1] = __fma(mp_pp[0], mp_pz[1], mp_pp[1]);
        mp_pp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[0]))
                   ? (__sjn_j0f_ep_fPP_MP[0])
                   : (mp_pp[0]);
        __ah = (__fabs(mp_pp[0]) <=
                __fabs(__sjn_j0f_ep_fPP_MP[0]))
                   ? (mp_pp[0])
                   : (__sjn_j0f_ep_fPP_MP[0]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_pp[1] = (mp_pp[1] + __sjn_j0f_ep_fPP_MP[1]) + __ahl;
        mp_pp[0] = __ph;
      };
      mp_qp[0] = __sjn_j0f_ep_fQP_MP[10];
      mp_qp[1] = __sjn_j0f_ep_fQP_MP[11];
      {
        float __ph, __phl;
        __ph = __fma(mp_qp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_qp[0], mp_pz[0], -__ph);
        mp_qp[1] = __fma(mp_qp[1], mp_pz[0], __phl);
        mp_qp[1] = __fma(mp_qp[0], mp_pz[1], mp_qp[1]);
        mp_qp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[8]))
                   ? (__sjn_j0f_ep_fQP_MP[8])
                   : (mp_qp[0]);
        __ah = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[8]))
                   ? (mp_qp[0])
                   : (__sjn_j0f_ep_fQP_MP[8]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_qp[1] = (mp_qp[1] + __sjn_j0f_ep_fQP_MP[9]) + __ahl;
        mp_qp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_qp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_qp[0], mp_pz[0], -__ph);
        mp_qp[1] = __fma(mp_qp[1], mp_pz[0], __phl);
        mp_qp[1] = __fma(mp_qp[0], mp_pz[1], mp_qp[1]);
        mp_qp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[6]))
                   ? (__sjn_j0f_ep_fQP_MP[6])
                   : (mp_qp[0]);
        __ah = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[6]))
                   ? (mp_qp[0])
                   : (__sjn_j0f_ep_fQP_MP[6]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_qp[1] = (mp_qp[1] + __sjn_j0f_ep_fQP_MP[7]) + __ahl;
        mp_qp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_qp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_qp[0], mp_pz[0], -__ph);
        mp_qp[1] = __fma(mp_qp[1], mp_pz[0], __phl);
        mp_qp[1] = __fma(mp_qp[0], mp_pz[1], mp_qp[1]);
        mp_qp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[4]))
                   ? (__sjn_j0f_ep_fQP_MP[4])
                   : (mp_qp[0]);
        __ah = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[4]))
                   ? (mp_qp[0])
                   : (__sjn_j0f_ep_fQP_MP[4]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_qp[1] = (mp_qp[1] + __sjn_j0f_ep_fQP_MP[5]) + __ahl;
        mp_qp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_qp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_qp[0], mp_pz[0], -__ph);
        mp_qp[1] = __fma(mp_qp[1], mp_pz[0], __phl);
        mp_qp[1] = __fma(mp_qp[0], mp_pz[1], mp_qp[1]);
        mp_qp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[2]))
                   ? (__sjn_j0f_ep_fQP_MP[2])
                   : (mp_qp[0]);
        __ah = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[2]))
                   ? (mp_qp[0])
                   : (__sjn_j0f_ep_fQP_MP[2]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_qp[1] = (mp_qp[1] + __sjn_j0f_ep_fQP_MP[3]) + __ahl;
        mp_qp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_qp[0], mp_pz[0], 0.0f);
        __phl = __fma(mp_qp[0], mp_pz[0], -__ph);
        mp_qp[1] = __fma(mp_qp[1], mp_pz[0], __phl);
        mp_qp[1] = __fma(mp_qp[0], mp_pz[1], mp_qp[1]);
        mp_qp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[0]))
                   ? (__sjn_j0f_ep_fQP_MP[0])
                   : (mp_qp[0]);
        __ah = (__fabs(mp_qp[0]) <=
                __fabs(__sjn_j0f_ep_fQP_MP[0]))
                   ? (mp_qp[0])
                   : (__sjn_j0f_ep_fQP_MP[0]);
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_qp[1] = (mp_qp[1] + __sjn_j0f_ep_fQP_MP[1]) + __ahl;
        mp_qp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_qp[0], mp_py[0], 0.0f);
        __phl = __fma(mp_qp[0], mp_py[0], -__ph);
        mp_qp[1] = __fma(mp_qp[1], mp_py[0], __phl);
        mp_qp[1] = __fma(mp_qp[0], mp_py[1], mp_qp[1]);
        mp_qp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_pp[0], mp_c[0], 0.0f);
        __phl = __fma(mp_pp[0], mp_c[0], -__ph);
        mp_pp[1] = __fma(mp_pp[1], mp_c[0], __phl);
        mp_pp[1] = __fma(mp_pp[0], mp_c[1], mp_pp[1]);
        mp_pp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_qp[0], mp_s[0], 0.0f);
        __phl = __fma(mp_qp[0], mp_s[0], -__ph);
        mp_qp[1] = __fma(mp_qp[1], mp_s[0], __phl);
        mp_qp[1] = __fma(mp_qp[0], mp_s[1], mp_qp[1]);
        mp_qp[0] = __ph;
      };
      {
        float __ph, __ahl, __ahh;
        float __ah, __bh;
        __bh = (__fabs(mp_pp[0]) <=
                __fabs((-mp_qp[0])))
                   ? ((-mp_qp[0]))
                   : (mp_pp[0]);
        __ah = (__fabs(mp_pp[0]) <=
                __fabs((-mp_qp[0])))
                   ? (mp_pp[0])
                   : ((-mp_qp[0]));
        __ph = __fma(__ah, 1.0f, __bh);
        __ahh = __fma(__ph, 1.0f, -__bh);
        __ahl = __fma(__ah, 1.0f, -__ahh);
        mp_pp[1] = (mp_pp[1] + (-mp_qp[1])) + __ahl;
        mp_pp[0] = __ph;
      };
      {
        float __ph, __phl;
        __ph = __fma(mp_pp[0], psq, 0.0f);
        __phl = __fma(mp_pp[0], psq, -__ph);
        mp_pp[1] = __fma(mp_pp[1], psq, __phl);
        mp_pp[0] = __ph;
      };
      *r = mp_pp[0] + mp_pp[1];
      return nRet;
    }
  } else /* INF or NaN */
  {
    if (ix <= 0x7f800000) /* INF */
    {
      *r = 0.0f;
      return nRet;
    } else /* NaN */
    {
      *r = xf * 1.0f; /* raise invalid on SNaN, return QNaN */
      return nRet;
    }
  }
}
/* file: _vsj1_kernel_cout.i */
/* file: _vssincos_kernel_cout.i */
// #define _SP_SINCOS_MP_
/////////////////////////////////////////////////////////////////////
//
//  Multi-precision macros
//
/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// unsigned 64-bit shift
// signed 64-bit shift
static const int32_t __sjn_sincosf_ep___ip_h = 0x0517CC1B;
static const int32_t __sjn_sincosf_ep___ip_m = 0x727220A9;
static const int32_t __sjn_sincosf_ep___ip_l = 0x28;
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cc4 = {0x3e6ce1b2u};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cc3 = {0xbfaae2beu};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cc2 = {0x4081e0eeu};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cc1 = {0xc09de9e6u};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cc1l = {0xb3e646a5u};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cc0 = {0x3f800000u};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cs3 = {0xbf16c981u};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cs2 = {0x40232f49u};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cs1 = {0xc0a55dddu};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cs0 = {0x40490fdbu};
static const union {
  uint32_t w;
  float f;
} __sjn_sincosf_ep___cs0l = {0xb3d195e9u};
static const uint32_t __sjn_sincosf_ep_invpi_tbl[] = {
    0,          0x28BE60DB, 0x9391054A, 0x7F09D5F4,
    0x7D4D3770, 0x36D8A566, 0x4F10E410, 0x7F9458EA};
static inline int __sjn_sincos_ep_kernel_fp32(float xf, int n, float *psin,
                                              float *pcos) {
  int nRet = 0;
  float xin = xf;
  uint64_t IP, IP2, N64 = (uint64_t)n;
  int64_t IP_s, IP2_s;
  int32_t ip_low_s;
  uint32_t ip_low;
  int_float x, Rh, Rl, res, scale, cres, sres, spoly, cpoly, cpoly_l;
  int mx, sgn_x, ex, ip_h, shift, index, j, sgn_p, sgn_xp;
  float High, Low, R2h, R2l, Ph, Pl;
  x.f = xin;
  mx = (x.w & 0x007fffff) | 0x00800000;
  sgn_x = x.w & 0x80000000;
  ex = ((x.w ^ sgn_x) >> 23);
  // redirect large or very small inputs
  if (__builtin_expect(((unsigned)(ex - 0x7f + 12)) > (20 + 12), (0 == 1))) {
    // small input: to be updated
    if (__builtin_expect((ex < 0x7f - 11), (1 == 1))) {
      psin[0] = xin;
      pcos[0] = 1.0f;
      return nRet;
    }
    // Inf/NaN
    if (ex == 0xff) {
      nRet = ((x.w << 1) == 0xff000000) ? 1 : nRet;
      x.w |= 0x00400000;
      psin[0] = x.f;
      pcos[0] = x.f;
      return nRet;
    }
    ex = ex - 0x7f - 23;
    index = 1 + (ex >> 5);
    // expon % 32
    j = ex & 0x1f;
    // x/Pi, scaled by 2^(63-j)
    ip_low = (((uint32_t)__sjn_sincosf_ep_invpi_tbl[index]) * ((uint32_t)mx));
    IP = (((uint64_t)((uint32_t)(__sjn_sincosf_ep_invpi_tbl[index + 1]))) *
          ((uint32_t)(mx))) +
         (((uint64_t)ip_low) << 32);
    // scaled by 2^(95-j)
    IP2 = (((uint64_t)((uint32_t)(__sjn_sincosf_ep_invpi_tbl[index + 2]))) *
           ((uint32_t)(mx))) +
          ((((uint64_t)((uint32_t)(__sjn_sincosf_ep_invpi_tbl[index + 3]))) *
            ((uint32_t)(mx))) >>
           32);
    IP = IP + (IP2 >> 32);
    // scale 2^63
    IP <<= j;
    // shift low part by 32-j, j in [0,31]
    ip_low = (uint32_t)IP2;
    ip_low >>= (31 - j);
    ip_low >>= 1;
    IP |= (uint64_t)ip_low;
  } else // main path
  {
    // products are really unsigned; operands are small enough so that signed
    // MUL works as well x*(23-ex)*(1/Pi)*2^28 p[k] products fit in 31 bits each
    IP_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sjn_sincosf_ep___ip_h)));
    IP = (uint64_t)IP_s;
    IP2_s = (((int64_t)((int32_t)(mx))) * ((int32_t)(__sjn_sincosf_ep___ip_m)));
    IP2 = (uint64_t)IP2_s;
    // scale (23-ex)*2^(28+32+7)
    ip_low_s = (((int32_t)mx) * ((int32_t)__sjn_sincosf_ep___ip_l));
    ip_low = (uint32_t)ip_low_s;
    IP2 = (IP2 << 7) + ip_low;
    // (x/Pi)*2^63
    IP <<= (ex - 0x7f + 12);
    // IP3 = IP2 << (37 -0x7f + ex);
    IP2 >>= (27 + 0x7f - ex);
    IP += IP2;
  }
  // add (n*Pi/4)*(2^63/Pi)
  IP += (N64 << 61);
  // return to 32-bit, scale 2^31
  ip_h = IP >> 32;
  // sign bit
  sgn_xp = ((ip_h + 0x20000000) & 0xc0000000);
  // reduced argument (signed, high-low), scale 2^32
  ip_h <<= 2;
  Rh.f = (float)ip_h;
  // reduced argument will need to be normalized
  shift = 2 + 30 + 0x7f - ((Rh.w >> 23) & 0xff);
  // correction for shift=0
  shift = (shift >= 2) ? shift : 2;
  // normalize
  IP <<= shift; // IP = (IP << shift) | (IP3 >> (64-shift));
  ip_h = IP >> 32;
  Rh.f = (float)ip_h;
  // adjust scale
  scale.w = (0x7f - 31 - shift) << 23;
  Rh.f = __fma(Rh.f, scale.f, 0.0f);
  // (Rh)^2
  R2h = __fma(Rh.f, Rh.f, 0.0f);
  cpoly.f = __fma(__sjn_sincosf_ep___cc4.f, R2h,
                                   __sjn_sincosf_ep___cc3.f);
  spoly.f = __fma(__sjn_sincosf_ep___cs3.f, R2h,
                                   __sjn_sincosf_ep___cs2.f);
  cpoly.f = __fma(cpoly.f, R2h, __sjn_sincosf_ep___cc2.f);
  spoly.f = __fma(spoly.f, R2h, __sjn_sincosf_ep___cs1.f);
  cpoly.f = __fma(cpoly.f, R2h, __sjn_sincosf_ep___cc1.f);
  spoly.f = __fma(spoly.f, R2h, __sjn_sincosf_ep___cs0.f);
  cpoly.f = __fma(cpoly.f, R2h, __sjn_sincosf_ep___cc0.f);
  spoly.f = __fma(spoly.f, Rh.f, 0.0f);
  sgn_p = sgn_xp & 0x80000000;
  // adjust sign
  spoly.w ^= sgn_p ^ (sgn_xp << 1);
  cpoly.w ^= sgn_p;
  sres.w = (sgn_xp & 0x40000000) ? cpoly.w : spoly.w;
  sres.w ^= sgn_x;
  cres.w = (sgn_xp & 0x40000000) ? spoly.w : cpoly.w;
  pcos[0] = cres.f;
  psin[0] = sres.f;
  return nRet;
}
/* Q2: polynomial minimax approximation J1(x) = y+y*z*Q2(z), y=x/2, z=y^2,
 * |x|=[0..2^(-2)], max.err .14e-11 */
static const float __sjn_j1f_ep_fQ2[] = {
    -0x1p-1f,       //            -0.5 [0xbf000000]   -0x1.fffffffff9f45p-2 , //
                    //            -0.499999999998625377     [0xBFDFFFFFFFFF9F45]
    0x1.555556p-4f, //     0.083333336 [0x3daaaaab]   0x1.55555524f34dbp-4  , //
                    //     0.0833333326292690496     [0x3FB55555524F34DB]
    -0x1.c71b8p-8f, //   -0.0069443882 [0xbbe38dc0]   -0x1.c71b7fc14bb27p-8 , //
                    //   -0.00694438809413958687   [0xBF7C71B7FC14BB27]
    0x1.6a933ap-12f, //   0.00034577856 [0x39b5499d]   0x1.6a933942150ecp-12 ,
                     //   // 0.000345778553428445793   [0x3F36A933942150EC]
};
/* Q1: polynomial minimax approximation J1(x) = Q1(x) on interval (0,s1). */
static const float __sjn_j1f_ep_fQ1[] = {
    0x1p-1f,          //             0.5 [0x3f000000]
    0x1.0aae7p-33f,   //   1.2127266e-10 [0x2f055738]
    -0x1p-4f,         //         -0.0625 [0xbd800000]
    0x1.7f25d8p-28f,  //   5.5755347e-09 [0x31bf92ec]
    0x1.5554cap-9f,   //    0.0026041504 [0x3b2aaa65]
    0x1.06d88p-25f,   //   3.0599267e-08 [0x33036c40]
    -0x1.c76fbp-15f,  //  -5.4292235e-05 [0xb863b7d8]
    0x1.22ffb2p-25f,  //    3.387672e-08 [0x33117fd9]
    0x1.610ccep-21f,  //    6.576069e-07 [0x35308667]
    0x1.26b396p-27f,  //   8.5769427e-09 [0x321359cb]
    -0x1.13d196p-27f, //  -8.0273805e-09 [0xb209e8cb]
    0x1.bba226p-32f,  //   4.0348222e-10 [0x2fddd113]
};
static const float __sjn_j1f_ep_fZ1[] = {
    0x1.ea7558p+1f,   //        3.831706 [0x40753aac]
    -0x1.4a121ep-24f, //   -7.685059e-08 [0x40753aac]
};
static const float __sjn_j1f_ep_fZ2[] = {
    0x1.c0ff6p+2f,    //       7.0155869 [0x40e07fb0]
    -0x1.8971b6p-23f, //  -1.8321172e-07 [0x40e07fb0]
};
static const float __sjn_j1f_ep_fZ3[] = {
    0x1.458d0ep+3f,   //       10.173469 [0x4122c687]
    -0x1.e8407ap-22f, //  -4.5471998e-07 [0x4122c687]
};
static const float __sjn_j1f_ep_fZ4[] = {
    0x1.aa5bbp+3f,    //       13.323692 [0x41552dd8]
    -0x1.9de34cp-22f, //  -3.8546312e-07 [0x41552dd8]
};
static const float __sjn_j1f_ep_fZ5[] = {
    0x1.0787b4p+4f,   //       16.470631 [0x4183c3da]
    -0x1.3f5ee8p-21f, //  -5.9487434e-07 [0x4183c3da]
};
/* P1: polynomial minimax approximation J1(x) = P1(x-z2) on interval (s1,s2). */
static const float __sjn_j1f_ep_fP1[] = {
    -0x1.1b9c1cp-54f, //  -6.1498073e-17 [0xa48dce0e]
    -0x1.9c6cf6p-2f,  //      -0.4027594 [0xbece367b]
    0x1.ae8a3ap-5f,   //     0.052556146 [0x3d57451d]
    0x1.b589d2p-5f,   //     0.053410444 [0x3d5ac4e9]
    -0x1.537546p-8f,  //   -0.0051797195 [0xbba9baa3]
    -0x1.24b33ep-9f,  //    -0.002233125 [0xbb12599f]
    0x1.6e4c84p-13f,  //   0.00017466492 [0x39372642]
    0x1.839f2ap-15f,  //   4.6208112e-05 [0x3841cf95]
    -0x1.97ad98p-19f, //  -3.0374385e-06 [0xb64bd6cc]
    -0x1.334224p-21f, //   -5.723133e-07 [0xb519a112]
    0x1.190906p-25f,  //   3.2716809e-08 [0x330c8483]
    0x1.38e59cp-28f,  //   4.5532493e-09 [0x319c72ce]
    -0x1.274c5cp-32f, //  -2.6857222e-10 [0xaf93a62e]
};
/* P2: polynomial minimax approximation J1(x) = P2(x-z3) on interval (s2,s3). */
static const float __sjn_j1f_ep_fP2[] = {
    0x1.04977p-55f,   //   2.8253393e-17 [0x24024bb8]
    0x1.33518cp-2f,   //      0.30011576 [0x3e99a8c6]
    -0x1.5e70dcp-6f,  //    -0.021389212 [0xbcaf386e]
    -0x1.80c83cp-5f,  //    -0.046970479 [0xbd40641e]
    0x1.9a4b2ap-9f,   //    0.0031302918 [0x3b4d2595]
    0x1.13fbc2p-9f,   //    0.0021055865 [0x3b09fde1]
    -0x1.0735c2p-13f, //   -0.0001255083 [0xb9039ae1]
    -0x1.79689cp-15f, //  -4.4990615e-05 [0xb83cb44e]
    0x1.426118p-19f,  //   2.4019128e-06 [0x3621308c]
    0x1.2fd368p-21f,  //   5.6591966e-07 [0x3517e9b4]
    -0x1.d6b132p-26f, //   -2.739789e-08 [0xb2eb5899]
    -0x1.36165p-28f,  //   -4.512362e-09 [0xb19b0b28]
    0x1.e551e2p-33f,  //   2.2069792e-10 [0x2f72a8f1]
};
/* P3: polynomial minimax approximation J1(x) = P3(x-z4) on interval (s3,s4). */
static const float __sjn_j1f_ep_fP3[] = {
    0x1.0212f4p-53f,  //   1.1192177e-16 [0x2501097a]
    -0x1.ff6546p-3f,  //     -0.24970488 [0xbe7fb2a3]
    0x1.9224p-7f,     //     0.012272358 [0x3c491200]
    0x1.4b0c5ep-5f,   //     0.040411171 [0x3d25862f]
    -0x1.f91aa2p-10f, //    -0.001926819 [0xbafc8d51]
    -0x1.f51c1ap-10f, //   -0.0019115821 [0xbafa8e0d]
    0x1.6b4ce6p-14f,  //   8.6617561e-05 [0x38b5a673]
    0x1.63c34ep-15f,  //   4.2410244e-05 [0x3831e1a7]
    -0x1.e381b4p-20f, //  -1.8012026e-06 [0xb5f1c0da]
    -0x1.2569e2p-21f, //  -5.4652543e-07 [0xb512b4f1]
    0x1.75ea44p-26f,  //   2.1764723e-08 [0x32baf522]
    0x1.2fa7fcp-28f,  //   4.4187791e-09 [0x3197d3fe]
    -0x1.8a59bap-33f, //  -1.7932984e-10 [0xaf452cdd]
};
/* P4: polynomial minimax approximation J1(x) = P4(x-z5) on interval (s4,s5). */
static const float __sjn_j1f_ep_fP4[] = {
    -0x1.05dcc6p-54f, //  -5.6782356e-17 [0xa482ee63]
    0x1.bf3338p-3f,   //      0.21835941 [0x3e5f999c]
    -0x1.0c83a2p-7f,  //   -0.0081944028 [0xbc0641d1]
    -0x1.251858p-5f,  //     -0.03577821 [0xbd128c2c]
    0x1.59eb18p-10f,  //    0.0013195737 [0x3aacf58c]
    0x1.c5bcd8p-10f,  //    0.0017308719 [0x3ae2de6c]
    -0x1.04141ap-14f, //  -6.2007552e-05 [0xb8820a0d]
    -0x1.4a650ap-15f, //  -3.9386116e-05 [0xb8253285]
    0x1.6c4f28p-20f,  //   1.3571575e-06 [0x35b62794]
    0x1.1654fap-21f,  //   5.1843364e-07 [0x350b2a7d]
    -0x1.267d82p-26f, //  -1.7141589e-08 [0xb2933ec1]
    -0x1.247a6ep-28f, //  -4.2561186e-09 [0xb1923d37]
    0x1.3f25fap-33f,  //   1.4513186e-10 [0x2f1f92fd]
};
/* P5: polynomial minimax approximation J1(x) = PS(x-z6) on interval (s5,s6). */
static const float __sjn_j1f_ep_fP5[] = {
    -0x1.6eb906p-52f, //  -3.1808128e-16 [0xa5b75c83]
    -0x1.925c7p-3f,   //     -0.19646537 [0xbe492e38]
    0x1.86dd32p-8f,   //     0.005964112 [0x3bc36e99]
    0x1.09463cp-5f,   //     0.032382123 [0x3d04a31e]
    -0x1.fda02cp-11f, //  -0.00097203383 [0xba7ed016]
    -0x1.9f4bdap-10f, //   -0.0015842296 [0xbacfa5ed]
    0x1.8779e6p-15f,  //   4.6667596e-05 [0x3843bcf3]
    0x1.32c912p-15f,  //   3.6571673e-05 [0x38196489]
    -0x1.19e174p-20f, //  -1.0500873e-06 [0xb58cf0ba]
    -0x1.064b94p-21f, //  -4.8856293e-07 [0xb50325ca]
    0x1.d52916p-27f,  //   1.3654367e-08 [0x326a948b]
    0x1.171c38p-28f,  //   4.0615884e-09 [0x318b8e1c]
    -0x1.03f3f6p-33f, //  -1.1821293e-10 [0xaf01f9fb]
};
/* PP: polynomial pade approximation P1(x) = PP(256/x^2) in point 256/x^2 = 0.5
 */
static const float __sjn_j1f_ep_fPP[] = {
    0x1p+0f,          //               1 [0x3f800000]
    0x1.ep-12f,       //   0.00045776367 [0x39f00000]
    -0x1.274fbep-19f, //  -2.2002421e-06 [0xb613a7df]
    0x1.5a3d1ep-25f,  //   4.0307494e-08 [0x332d1e8f]
    -0x1.afbe9cp-30f, //  -1.5706776e-09 [0xb0d7df4e]
    0x1.6be2b6p-34f,  //   8.2738004e-11 [0x2eb5f15b]
};
/* QP: polynomial pade approximation Q1(x) = QP(256/x^2)*(16/x)) in point
 * 256/x^2 = 0.5 */
static const float __sjn_j1f_ep_fQP[] = {
    0x1.8p-6f,        //       0.0234375 [0x3cc00000]
    -0x1.a4p-16f,     //  -2.5033951e-05 [0xb7d20000]
    0x1.1c3c46p-22f,  //   2.6471488e-07 [0x348e1e23]
    -0x1.fdd85cp-28f, //  -7.4192235e-09 [0xb1feec2e]
    0x1.a76f66p-32f,  //   3.8511203e-10 [0x2fd3b7b3]
    -0x1.ab6366p-36f, //  -2.4294211e-11 [0xadd5b1b3]
};
static inline int __sjn_j1_ep_kernel_fp32(const float *a, float *r) {
  int nRet = 0;
  int32_t sign;
  uint32_t iax;
  float xf = *a;
  float resultf;
  iax = ((*(int32_t *)&xf) & ~0x80000000);
  sign = (((uint32_t)(*(int32_t *)&xf)) >> 31);
  if (iax < 0x7f800000) /* finite x */
  {
    if (iax < 0x4190918a) /* x < 18.0710639953613281250 */
    {
      if (iax < 0x4019e8a9) /*  x < 2.40482544898986816406250 */
      {
        if (iax < 0x3e800000) /* 0 <= |x| < 2^(-2) */
        {
          if (iax < 0x3b800000) /* 0 <= |x| < 2^(-8) */
          {
            if (iax < 0x32000000) /* |x| < 2^(-27) */
            {
              if (iax != 0) {
                /* return x/2 */
                const float small_value_32[] = {
                    0x1.000000p-100f,
                    -0x1.000000p-100f}; /* +2^(-100),-2^(-100) */
                resultf =
                    (xf * 0.5f - (small_value_32[(sign)] * small_value_32[0]));
              } else /* Zero */
              {
                resultf = xf;
              }
              *r = resultf;
              return nRet;
            } else /* |x| >= 2^(-27) */
            {
              float fx, fy, fresult;
              fx = (xf * 0.5f);
              fy = fx * fx;
              fresult =
                  ((__sjn_j1f_ep_fQ2[1] * fy + __sjn_j1f_ep_fQ2[0]) * fy) * fx +
                  fx;
              *r = fresult;
              return nRet;
            }
          } else /* |x| >= 2^(-8) */
          {
            float fx, fy, fz, fresult;
            fx = xf * 0.5f;
            fy = fx * fx;
            fz = fy * fy;
            fresult = ((__sjn_j1f_ep_fQ2[3] * fz + __sjn_j1f_ep_fQ2[1]) * fz +
                       (__sjn_j1f_ep_fQ2[2] * fz + __sjn_j1f_ep_fQ2[0]) * fy) *
                          fx +
                      fx;
            *r = fresult;
            return nRet;
          }
        } else /* |x| >= 2^(-2) */
        {
          float px = __fabs(xf), presult;
          presult =
              px *
              (__sjn_j1f_ep_fQ1[0] +
               px *
                   (__sjn_j1f_ep_fQ1[1] +
                    px *
                        (__sjn_j1f_ep_fQ1[2] +
                         px *
                             (__sjn_j1f_ep_fQ1[3] +
                              px *
                                  (__sjn_j1f_ep_fQ1[4] +
                                   px *
                                       (__sjn_j1f_ep_fQ1[5] +
                                        px *
                                            (__sjn_j1f_ep_fQ1[6] +
                                             px *
                                                 (__sjn_j1f_ep_fQ1[7] +
                                                  px *
                                                      (__sjn_j1f_ep_fQ1[8] +
                                                       px *
                                                           (__sjn_j1f_ep_fQ1
                                                                [9] +
                                                            px *
                                                                (__sjn_j1f_ep_fQ1
                                                                     [10] +
                                                                 px *
                                                                     (__sjn_j1f_ep_fQ1
                                                                          [11]))))))))))));
          *r = sign ? -presult : presult;
          return nRet;
        }
      } else /*  x >= 2.40482544898986816406250 */
      {
        const float *pP;
        const float *pZ;
        float px = __fabs(xf), py, pz, pp, pq, presult;
        if (iax < 0x416ee50a) /* x < 14.93091773986816406250 */
        {
          if (iax < 0x410a75ab) /* x < 8.653727531433105468750 */
          {
            if (iax < 0x40b0a47b) /* 5.5200781822204589843750 */
            {
              pP = __sjn_j1f_ep_fP1;
              pZ = __sjn_j1f_ep_fZ1;
            } else {
              pP = __sjn_j1f_ep_fP2;
              pZ = __sjn_j1f_ep_fZ2;
            }
          } else {
            if (iax < 0x413caa20) /* 11.7915344238281250 */
            {
              pP = __sjn_j1f_ep_fP3;
              pZ = __sjn_j1f_ep_fZ3;
            } else {
              pP = __sjn_j1f_ep_fP4;
              pZ = __sjn_j1f_ep_fZ4;
            }
          }
        } else /* x >= 14.93091773986816406250 */
        {
          pP = __sjn_j1f_ep_fP5;
          pZ = __sjn_j1f_ep_fZ5;
        }
        px = px - pZ[0];
        px = px - pZ[1];
        presult =
            pP[0] +
            px *
                (pP[1] +
                 px *
                     (pP[2] +
                      px *
                          (pP[3] +
                           px *
                               (pP[4] +
                                px *
                                    (pP[5] +
                                     px *
                                         (pP[6] +
                                          px *
                                              (pP[7] +
                                               px *
                                                   (pP[8] +
                                                    px *
                                                        (pP[9] +
                                                         px *
                                                             (pP[10] +
                                                              px *
                                                                  (pP[11] +
                                                                   px *
                                                                       (pP[12]))))))))))));
        *r = (sign ? -presult : presult);
        return nRet;
      }
    } else /* x >= 18.0710639953613281250 Hancels asymptotic forms */
    {
      const float ptonpi = 0x1.45f306p-1f; // 0.63661975 [0x3f22f983] 2/Pi
      float px = __fabs(xf), pxi = 1.0f / px, py = 16.0f * pxi,
            pz = py * py, psn, pcs, pp, pq, presult,
            psq = __sqrt(ptonpi * pxi);
      __sjn_sincos_ep_kernel_fp32(px, -3, &psn, &pcs);
      pp = __sjn_j1f_ep_fPP[0] +
           pz * (__sjn_j1f_ep_fPP[1] +
                 pz * (__sjn_j1f_ep_fPP[2] +
                       pz * (__sjn_j1f_ep_fPP[3] +
                             pz * (__sjn_j1f_ep_fPP[4] +
                                   pz * (__sjn_j1f_ep_fPP[5])))));
      ;
      pq = py * (__sjn_j1f_ep_fQP[0] +
                 pz * (__sjn_j1f_ep_fQP[1] +
                       pz * (__sjn_j1f_ep_fQP[2] +
                             pz * (__sjn_j1f_ep_fQP[3] +
                                   pz * (__sjn_j1f_ep_fQP[4] +
                                         pz * (__sjn_j1f_ep_fQP[5]))))));
      presult = psq * (pp * pcs - pq * psn);
      *r = (float)(sign ? -presult : presult);
      return nRet;
    }
  } else /* INF or NaN */
  {
    if (iax <= 0x7f800000) /* INF */
    {
      const float zeros[] = {0x0.0p+0f, -0x0.0p+0f};
      *r = zeros[sign];
      return nRet;
    } else /* NaN */
    {
      /* raise invalid on SNaN, return QNaN */
      *r = xf * 1.0f;
      return nRet;
    }
  }
  return nRet;
}
/*
//
//  Zero points where all inputs on the left produce zero results
//
*/
static const float function_zeros[] = {
    0.0f,    0.0f,    0.0f,    0.0f,    0.0f,    0.5f,    1.6f,    3.5f,
    6.1f,    9.1f,    12.7f,   16.6f,   20.8f,   25.3f,   30.1f,   35.0f,
    40.1f,   45.4f,   50.8f,   56.3f,   62.0f,   67.7f,   73.5f,   79.4f,
    85.4f,   91.4f,   97.5f,   103.7f,  109.9f,  116.2f,  122.5f,  128.9f,
    135.3f,  141.7f,  148.2f,  154.7f,  161.3f,  167.9f,  174.5f,  181.1f,
    187.8f,  194.5f,  201.2f,  208.0f,  214.0f,  221.0f,  228.0f,  235.0f,
    242.0f,  248.0f,  255.0f,  262.0f,  269.0f,  276.0f,  283.0f,  290.0f,
    297.0f,  304.0f,  311.0f,  318.0f,  325.0f,  332.0f,  339.0f,  346.0f,
    353.0f,  360.0f,  367.0f,  374.0f,  382.0f,  389.0f,  396.0f,  403.0f,
    410.0f,  417.0f,  424.0f,  432.0f,  439.0f,  446.0f,  453.0f,  460.0f,
    468.0f,  475.0f,  482.0f,  489.0f,  497.0f,  504.0f,  511.0f,  518.0f,
    526.0f,  533.0f,  540.0f,  548.0f,  555.0f,  562.0f,  569.0f,  577.0f,
    584.0f,  591.0f,  599.0f,  606.0f,  613.0f,  621.0f,  628.0f,  635.0f,
    643.0f,  650.0f,  657.0f,  665.0f,  672.0f,  680.0f,  687.0f,  694.0f,
    702.0f,  709.0f,  717.0f,  724.0f,  731.0f,  739.0f,  746.0f,  754.0f,
    761.0f,  768.0f,  776.0f,  783.0f,  791.0f,  798.0f,  806.0f,  813.0f,
    820.0f,  828.0f,  835.0f,  843.0f,  850.0f,  858.0f,  865.0f,  873.0f,
    880.0f,  888.0f,  895.0f,  903.0f,  910.0f,  918.0f,  925.0f,  933.0f,
    940.0f,  947.0f,  955.0f,  962.0f,  970.0f,  977.0f,  985.0f,  993.0f,
    1000.0f, 1008.0f, 1015.0f, 1023.0f, 1030.0f, 1038.0f, 1045.0f, 1053.0f,
    1060.0f, 1068.0f, 1075.0f, 1083.0f, 1090.0f, 1098.0f, 1105.0f, 1113.0f,
    1120.0f, 1128.0f, 1136.0f, 1143.0f, 1151.0f, 1158.0f, 1166.0f, 1173.0f,
    1181.0f, 1188.0f, 1196.0f, 1204.0f, 1211.0f, 1219.0f, 1226.0f, 1234.0f,
    1241.0f, 1249.0f, 1257.0f, 1264.0f, 1272.0f, 1279.0f, 1287.0f, 1294.0f,
    1302.0f, 1310.0f, 1317.0f, 1325.0f, 1332.0f, 1340.0f, 1348.0f, 1355.0f,
    1363.0f, 1370.0f, 1378.0f, 1386.0f, 1393.0f, 1401.0f, 1408.0f, 1416.0f,
    1424.0f, 1431.0f, 1439.0f, 1446.0f, 1454.0f, 1462.0f, 1469.0f, 1477.0f,
    1485.0f, 1492.0f, 1500.0f, 1507.0f, 1515.0f, 1523.0f, 1530.0f, 1538.0f,
    1546.0f, 1553.0f, 1561.0f, 1568.0f, 1576.0f, 1584.0f, 1591.0f, 1599.0f,
    1607.0f, 1614.0f, 1622.0f, 1629.0f, 1637.0f, 1645.0f, 1652.0f, 1660.0f,
    1668.0f, 1675.0f, 1683.0f, 1691.0f, 1698.0f, 1706.0f, 1714.0f, 1721.0f,
    1729.0f, 1737.0f, 1744.0f, 1752.0f, 1760.0f, 1767.0f, 1775.0f, 1782.0f,
    1790.0f, 1798.0f, 1805.0f, 1813.0f, 1821.0f, 1828.0f, 1836.0f, 1844.0f,
    1851.0f, 1859.0f, 1867.0f, 1874.0f, 1882.0f, 1890.0f, 1898.0f, 1905.0f,
    1913.0f, 1921.0f, 1928.0f, 1936.0f, 1944.0f, 1951.0f, 1959.0f, 1967.0f,
    1974.0f, 1982.0f, 1990.0f, 1997.0f, 2005.0f, 2013.0f, 2020.0f, 2028.0f,
    2036.0f, 2043.0f, 2051.0f, 2059.0f, 2067.0f, 2074.0f, 2082.0f, 2090.0f,
    2097.0f, 2105.0f, 2113.0f, 2120.0f, 2128.0f, 2136.0f, 2143.0f, 2151.0f,
    2159.0f, 2167.0f, 2174.0f, 2182.0f, 2190.0f, 2197.0f, 2205.0f, 2213.0f,
    2220.0f, 2228.0f, 2236.0f, 2244.0f, 2251.0f, 2259.0f, 2267.0f, 2274.0f,
    2282.0f,
};
inline int __devicelib_imf_internal_sjn(const int *pn, const float *pa,
                                        float *pr) {
  int nRet = 0;
  // Constants
  const uint32_t un1max = 32; // Maximum 'n' for Path 1
  const uint32_t ur_hi_mask = (0xffffffffu)
                              << 6; // Mask for Path 1 to clear low bits
  const uint32_t up_hi_mask = (0xffffffffu)
                              << 3; // Mask for path 4 to clear low bits
  const uint64_t ulr_hi_mask = (0xfffffffffffffffful)
                               << 17; // Mask for path 2 to clear low bits
  const float fj0rmax =
      0.993f; // Maximum j0 result where jn is not zero in Path 2
  const float ftonpi = 0x1.45f306p-1f;        // 0.63661975 [0x3f22f983] = 2/Pi
  const float fzeros[] = {0x0p+0f, -0x0p+0f}; // Positive and negative zeros
  int n, i, k, flgh, flgs, loopcnt, maxloopcnt;
  int32_t ix;
  uint32_t usign, un, ux;
  uint32_t ur_hi, ur_prev_hi, up_hi, up_prev_hi;
  float fx, fax, fy, fres, fj0r, fx2, fz, ft, fr, fp, fq, fs, fp1, fp2, fq1,
      fq2, ftv, fn4, fx8, fv, ftwox, fr0, fr1, fnn, fdd, frr, fmpy[2], fmpz[2];
  uint64_t ulr_hi, ulr_prev_hi, ult;
  // Integer representations of fp64 variables
  // for emulation routines
  dp_int dx, dx2, dy, dz, dzox, dr, dp, dq, ds, dp1, dp1_prev, dq1, dt,
      dq1_prev, dtv, dinvx, done, dj0r, dzero, dres;
  fx = *pa;
  n = *pn;
  fres = fx;
  un = (uint32_t)n;
  ux = (*(uint32_t *)&fx) & ~0x80000000;
  ix = (*(int32_t *)&fx) ^ n;
  // even n: usign=0, odd n: usign=usign(n)*usign(x)
  usign = ((uint32_t)(ix) >> 31) & n;
  // if n=0 go to j0 kernel
  if (n == 0) {
    nRet = __sjn_j0_ep_kernel_fp32(pa, pr);
  }
  // if n=1 go to j1 kernel
  else if (n == 1) {
    nRet = __sjn_j1_ep_kernel_fp32(pa, pr);
  }
  // if x = +/-0 return +/-0
  else if (ux == 0) {
    *pr = fzeros[usign];
  }
  // Branch for finite x
  else if (ux < 0x7f800000) {
    // un = |n|
    if (n < 0)
      un = -n;
    // fax = |fx|
    fax = __fabs(fx);
    // Where jnf(n,x) = 0.0f
    i = un >> 3;
    // Get 'function zeros' from table
    if (i <= 320) {
      fy = function_zeros[i];
    }
    // Extend 'function zeros'
    else {
      i -= 320;
      fy = function_zeros[320] + 7 * i;
    }
    // Return 0 if |x| is less than 'function zero'
    if (fax < fy) {
      fres = 0.0f;
    }
    ///////////// path 1 (x <= X1) && (n<=un1max), where X1 = 0.6*n
    else if ((un <= un1max) && (fax <= 0.6f * un)) {
      // ur_prev_hi = 1.0f in hex
      ur_prev_hi = 0x3f800000;
      maxloopcnt = 24;
      fx2 = fax / 2.0f;
      fp = 1.0f;
      fq = 1.0f;
      // Forward recursion
      for (i = 1, fz = 1.0f; i <= un; i++, fz += 1.0f) {
        fp = fp * fx2;
        fq = fq * fz;
      }
      // Backward recursion
      k = 0;
      fr = 1.0f;
      fz = 1.0f;
      fx2 = (-fx2) * (fx2);
      do {
        ur_prev_hi = ur_hi;
        k = k + 1;
        fnn = fz * fx2;
        fdd = (k * (un + k));
        // Refined division fnn/fdd by 1 NR iteration
        frr = 1.0f / fdd;
        frr = frr * (2.0f - fdd * frr);
        fz = fnn * frr;
        fr = fr + fz;
        // r_hi = r with low bits cleared
        ur_hi = *(uint32_t *)&fr;
        ur_hi = ur_hi & ur_hi_mask;
      }
      // Check if high part of r on current iteration is the same as ppevious
      // one then break the loop
      while ((ur_prev_hi != ur_hi) && maxloopcnt--);
      fres = fr * fp / fq;
    }
    ///////////// path 2 (X1 < x < X2), where X1 = 0.6*n, X2 = n
    else if (fax < un) {
      maxloopcnt = 24;
      ult = un + un;
      done = uint32_to_dp(1);
      dzero = uint32_to_dp(0);
      dx = sp_to_dp(fax);
      dx2 = dp_mul(dx, dx);
      dinvx = dp_div(done, dx);
      dp1_prev = dzero;
      dq1_prev = done;
      dp1 = dx;
      dq1 = uint64_to_dp(ult);
      // ulr_prev_hi = 1.0 in hex
      ulr_prev_hi = 0x3ff0000000000000ul;
      // Run j0 kernel
      __sjn_j0_ep_kernel_fp32(&fax, &fj0r);
      // Convert result to fp64 integer representation
      // for further emulations
      dj0r = sp_to_dp(fj0r);
      // Check if j0 result is less than 1-eps
      if (fj0r <= fj0rmax) {
        /* Forward recursion */
        for (loopcnt = 0; loopcnt < maxloopcnt; loopcnt++) {
          ult = ult + 2;
          dt = uint64_to_dp(ult);
          // dp = dp1*dt - dp1prev*x2
          dp = dp_sub(dp_mul(dp1, dt), dp_mul(dp1_prev, dx2));
          // dq = dq1*dt - dq1prev*x2
          dq = dp_sub(dp_mul(dq1, dt), dp_mul(dq1_prev, dx2));
          dp1_prev = dp1;
          dp1 = dp;
          dq1_prev = dq1;
          dq1 = dq;
          // dr = dp / dq
          dr = dp_div(dp, dq);
          // r_hi = r with low bits cleared
          ulr_hi = *(uint64_t *)&dr;
          ulr_hi &= ulr_hi_mask;
          // Check if high part of r on current iteration is the same as
          // ppevious one then break the loop
          if (ulr_prev_hi != ulr_hi)
            ulr_prev_hi = ulr_hi;
          else
            break;
        }
        // z = 1 / r
        dz = dp_div(done, dr);
        // Backward recursion
        dy = done;
        loopcnt = un - 1;
        ult = 2 * loopcnt;
        do {
          dt = uint64_to_dp(ult);
          // dzox = dz * (1/dx)
          dzox = dp_mul(dz, dinvx);
          // dr = dzox* dt - dy
          dr = dp_sub(dp_mul(dzox, dt), dy);
          dy = dz;
          dz = dr;
          ult = ult - 2;
        } while (--loopcnt > 0);
        // dr = dj0r / dz
        dres = dp_div(dj0r, dz);
        fres = dp_to_sp(dres);
      } else {
        fres = 0.0f;
      }
    }
    ///////////// path 3 (X2 < x < X3), where X2 = n, X3 = 20*n + 1000
    else if (fax < (20 * un + 1000)) {
      // Call j0 & j1 kernels
      __sjn_j0_ep_kernel_fp32(&fax, &fr0);
      __sjn_j1_ep_kernel_fp32(&fax, &fr1);
      fz = fr0;
      fy = fr1;
      ftwox = (2.0f / fax);
      // Forward recursion
      for (i = 1; i < un; i++) {
        ft = fy;
        fy = ftwox * i * fy - fz;
        fz = ft;
      }
      fres = fy;
    }
    ///////////// path 4 (x >= X3), where X3 = 20*n + 1000: Hancels asymptotic
    ///forms
    else {
      fx2 = 1.0f;
      ft = 1.0f;
      fp = 1.0f;
      fp1 = 2.0f;
      fp2 = 3.0f;
      fq1 = 3.0f;
      fq2 = 5.0f;
      fn4 = 4.0f * un * un;
      fx8 = fax * 8.0f;
      flgh = 0;
      flgs = -1;
      fq = (fn4 - 1.0f) / fx8;
      fz = fq;
      up_hi = 0x3f800000;
      maxloopcnt = 24;
      do {
        up_prev_hi = up_hi;
        fz *= (fn4 - fp2 * fp2) / (fp1 * fx8);
        fr = (fn4 - fq2 * fq2) / (fq1 * fx8);
        fr *= fz;
        if (flgs > 0) {
          fp += fz;
          fq += fr;
        } else {
          fp -= fz;
          fq -= fr;
        }
        fz = fr;
        fp1 += 2.0f;
        fq1 += 2.0f;
        fp2 += 4.0f;
        fq2 += 4.0f;
        ft = (fz / fp);
        ft = (ft >= 0) ? ft : (-ft);
        if (ft < fx2) {
          fx2 = ft;
          flgh = 1;
        } else if (flgh != 0)
          break;
        flgs = -flgs;
        // p_hi = p with low bits cleared
        up_hi = *(uint32_t *)&fp;
        up_hi = up_hi & up_hi_mask;
      }
      // Check if high part of p on current iteration is the same as ppevious
      // one then break the loop
      while ((up_prev_hi != up_hi) && maxloopcnt--);
      // Run sincos(x) kernel with octant correction to -(2*n + 1)
      __sjn_sincos_hl_ep_kernel_fp32(fax, -(2 * un + 1), &fmpy[0], &fmpz[0]);
      // Multi-precision results of sincos:
      // fmpy[0] + fmpy[1], fmpz[0] + fmpz[1]
      fp1 = fp * fmpz[0];
      fp2 = fp * fmpz[1];
      fq1 = fq * fmpy[0];
      fq2 = fq * fmpy[1];
      fp = fp1 - fq1;
      fp = fp + fp2;
      fp = fp - fq2;
      fres = __sqrt(ftonpi / fax) * fp;
    }
    // Apply sign for final result
    *pr = (float)(usign ? -fres : fres);
  }
  // x = INF or NaN
  else {
    // x = INF
    if (ux <= 0x7f800000) {
      // even n: usign=0, odd n: usign=usign(n)*usign(x)
      ix = (*(int32_t *)&fax) ^ n;
      usign = ((uint32_t)(ix) >> 31) & un;
      *pr = fzeros[usign];
    }
    // x = NaN
    else {
      // raise invalid on SNaN, return QNaN
      *pr = fx * 1.0f;
    }
  }
  return nRet;
}
} /* namespace */
} /* namespace __imf_impl_jn_s_ep */
DEVICE_EXTERN_C_INLINE float __devicelib_imf_jnf(int32_t x, float y) {
  using namespace __imf_impl_jn_s_ep;
  float r;
  __devicelib_imf_internal_sjn(&x, &y, &r);
  return r;
}
#endif /*__LIBDEVICE_IMF_ENABLED__*/
