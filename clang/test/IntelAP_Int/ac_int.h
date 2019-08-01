/**************************************************************************
 *                                                                        *
 *  Algorithmic C (tm) Datatypes                                          *
 *                                                                        *
 *  Software Version: 3.7                                                 *
 *                                                                        *
 *  Release Date    : Wed Jun  1 13:21:52 PDT 2016                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 3.7.0                                               *
 *                                                                        *
 *  Copyright 2004-2016, Mentor Graphics Corporation,                     *
 *                                                                        *
 *  All Rights Reserved.                                                  *
 *                                                                        *
 **************************************************************************
 *  Licensed under the Apache License, Version 2.0 (the "License");       *
 *  you may not use this file except in compliance with the License.      *
 *  You may obtain a copy of the License at                               *
 *                                                                        *
 *      http://www.apache.org/licenses/LICENSE-2.0                        *
 *                                                                        *
 *  Unless required by applicable law or agreed to in writing, software   *
 *  distributed under the License is distributed on an "AS IS" BASIS,     *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or       *
 *  implied.                                                              *
 *  See the License for the specific language governing permissions and   *
 *  limitations under the License.                                        *
 **************************************************************************
 *                                                                        *
 *  This file was modified by the Intel High Level Design team to         *
 *  generate efficient hardware for the Intel High Level Synthesis        *
 *  compiler. The API remains the same as defined by Mentor Graphics      *
 *  in the documentation for ac_int.h                                     *
 *                                                                        *
 *************************************************************************/

/*
//  Source:          ac_int.h
//  Description:     fast arbitrary-length bit-accurate integer types:
//                     - unsigned integer of length W:  ac_int<W,false>
//                     - signed integer of length W:  ac_int<W,true>
//  Original Author: Andres Takach, Ph.D.
//  Modified by:     Vince Bridgers, Thor Thayer, Ajaykumar Kannan
//
//  Notes:
//   - Compiler support: Works with the Intel i++ compiler. For g++, use the
//                       original ac_int.h header file.
//
//   - Most frequent migration issues:
//      - need to cast to common type when using question mark operator:
//          (a < 0) ? -a : a;  // a is ac_int<W,true>
//        change to:
//          (a < 0) ? -a : (ac_int<W+1,true>) a;
//        or
//          (a < 0) ? (ac_int<W+1,false>) -a : (ac_int<W+1,false>) a;
//
//      - left shift is not arithmetic ("a<<n" has same bitwidth as "a")
//          ac_int<W+1,false> b = a << 1;  // a is ac_int<W,false>
//        is not equivalent to b=2*a. In order to get 2*a behavior change to:
//          ac_int<W+1,false> b = (ac_int<W+1,false>)a << 1;
//
//      - only static length read/write slices are supported:
//         - read:  x.slc<4>(k) => returns ac_int for 4-bit slice x(4+k-1 DOWNTO k)
//         - write: x.set_slc(k,y) = writes bits of y to x starting at index k
*/

#ifndef __ALTR_AC_INT_H
#define __ALTR_AC_INT_H

#define AC_VERSION 3
#define AC_VERSION_MINOR 7

#ifndef __cplusplus
#error C++ is required to include this header file
#endif

// for safety
#if (defined(W) || defined(I) || defined(S) || defined(W2) || defined(I2) || defined(S2))
#error One or more of the following is defined: W, I, S, W2, I2, S2. Definition conflicts with their usage as template parameters.
#error DO NOT use defines before including third party header files.
#endif

#if (defined(true) || defined(false))
#error One or more of the following is defined: true, false. They are keywords in C++ of type bool. Defining them as 1 and 0, may result in subtle compilation problems.
#error DO NOT use defines before including third party header files.
#endif

// #ifndef __ASSERT_H__
// #define __ASSERT_H__
// #include <assert.h>
// #endif
// #include <limits>
// #ifndef AC_USER_DEFINED_ASSERT
// #if defined (__linux__)
// #include <iostream>
// #endif // linux
// #include <stdio.h>
// #else
// #if defined (__linux__)
// #include <ostream>
// #endif // linux
// #endif
// #include <algorithm>
// #include <HLS/math.h>
// #include <string>
// #include <vector>

template<int Bits> using ap_int = int __attribute__((__ap_int(Bits)));
template<unsigned int Bits> using ap_uint = unsigned int __attribute__((__ap_int(Bits)));

#ifdef __AC_NAMESPACE
namespace __AC_NAMESPACE {
#endif

#define AC_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AC_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AC_ABS(a) ((a) < 0 ? (-a) : (a))

#if defined(_MSC_VER)
typedef unsigned __int64 Ulong;
typedef signed   __int64 Slong;
#else
typedef unsigned long long Ulong;
typedef signed   long long Slong;
#endif

#define _STR_CONCAT_(a,b) a##b
#define _STR_CONCAT(a,b) _STR_CONCAT_(a,b)
// #define ACINT_STATIC_ASSERT(X) \
//   ;typedef int _STR_CONCAT(static_assert_line_, __LINE__)[(bool)(X) ? 1 : -1];

enum ac_base_mode { AC_BIN=2, AC_OCT=8, AC_DEC=10, AC_HEX=16 };
enum ac_special_val {AC_VAL_DC, AC_VAL_0, AC_VAL_MIN, AC_VAL_MAX, AC_VAL_QUANTUM};

static bool warned_undef = false;

template <int W, bool S> class ac_int;

namespace ac_private {

  // enum {long_w = std::numeric_limits<unsigned long>::digits};

  // PRIVATE FUNCTIONS in namespace: for implementing ac_int/ac_fixed

  inline double mgc_floor(double d) { return (int) d /*floor(d)*/; }

//   #define AC_ASSERT(cond, msg) ac_private::ac_assert(cond, __FILE__, __LINE__, msg)
//   inline void ac_assert(bool condition, const char *file=0, int line=0, const char *msg=0) {
//   #ifdef HLS_X86
//     #ifndef AC_USER_DEFINED_ASSERT
//     if(!condition) {
// #if defined (__linux__)
//       std::cerr << "Assert";
//       if(file)
//         std::cerr << " in file " << file << ":" << line;
//       if(msg)
//         std::cerr << " " << msg;
//       std::cerr << std::endl;
// #else
//       fprintf(stderr, "Assert");
//       if (file)
//         fprintf(stderr, " in file %s:%d", file, line);
//       if (msg)
//         fprintf(stderr, " %s", msg);
//       fprintf(stderr, "\n");
// #endif // linux
//       assert(0);
//     }
//     #else
//     AC_USER_DEFINED_ASSERT(condition, file, line, msg);
//     #endif
//   #endif
//   }

  // helper structs for statically computing log2 like functions (nbits, log2_floor, log2_ceil)
  //   using recursive templates
  template<unsigned char N>
  struct s_N {
    template<unsigned X>
    struct s_X {
      enum {
        X2 = X >> N,
        N_div_2 = N >> 1,
        nbits = X ? (X2 ? N + (int) s_N<N_div_2>::template s_X<X2>::nbits : (int) s_N<N_div_2>::template s_X<X>::nbits) : 0
      };
    };
  };
  template<> struct s_N<0> {
    template<unsigned X>
    struct s_X {
      enum {nbits = !!X };
    };
  };

  template<int N>
  inline double ldexpr32(double d) {
    double d2 = d;
    if(N < 0)
      for(int i=0; i < -N; i++)
        d2 /= (Ulong) 1 << 32;
    else
      for(int i=0; i < N; i++)
        d2 *= (Ulong) 1 << 32;
    return d2;
  }
  template<> inline double ldexpr32<0>(double d) { return d; }
  template<> inline double ldexpr32<1>(double d) { return d * ((Ulong) 1 << 32); }
  template<> inline double ldexpr32<-1>(double d) { return d / ((Ulong) 1 << 32); }
  template<> inline double ldexpr32<2>(double d) { return (d * ((Ulong) 1 << 32)) * ((Ulong) 1 << 32); }
  template<> inline double ldexpr32<-2>(double d) { return (d / ((Ulong) 1 << 32)) / ((Ulong) 1 << 32); }

  template<int N>
  inline double ldexpr(double d) {
    return ldexpr32<N/32>( N < 0 ? d/( (unsigned) 1 << (-N & 31)) : d * ( (unsigned) 1 << (N & 31)));
  }

  template<int N>
  inline double ldexpr1(double d) {
    return  N < 0 ? d/( (unsigned) 1 << (-N)) : d * ( (unsigned) 1 << (N));
  }

  // Xn-1, Xn-2, Xn-3, Xn-4, ...X(I+W-1)..XI.., X2, X1, X0
  template<int W, int I, int N>
  inline ap_uint<W> bit_slc(ap_uint<N> value) {
    // ACINT_STATIC_ASSERT(N >= W);
    const int shift_v = AC_MIN(I, N-1);
    ap_uint<N> op = (value >> (shift_v ));
    ap_uint<W> r = op;
    return r;
  }

  template<int W, int I, int N>
  inline ap_int<W> bit_slc(ap_int<N> value) {
    ap_uint<N> v = value;
    return (ap_uint<W>)(bit_slc<W, I, N>(v));
  }

  template<int N>
  inline bool ap_less_zero(ap_uint<N> value){
    return false;
  }

  template<int N>
  inline bool ap_less_zero(ap_int<N> value){
    return value < 0;
  }

  // if bits [0, B-1] all 0s
  template<int B, int N>
  inline bool ap_equal_zeros_to(ap_uint<N> value) {
    ap_uint<B> v = bit_slc<B, 0, N>(value);
    return v == 0;
  }

  template<int B, int N>
  inline bool ap_equal_zeros_to(ap_int<N> value) {
    ap_uint<N> v = value;
    return ap_equal_zeros_to<B, N>(v);
  }

  // if bits [0, B-1] all 0s
  template<int B, int N>
  inline bool ap_equal_ones_to(ap_uint<N> value) {
    // ACINT_STATIC_ASSERT(N >= B);
    if(!B) return true;
    const int B1 = AC_MAX(B, 1);
    ap_uint<B1> v = bit_slc<B1, 0, N>(value);
    return (~v) == 0;
  }

  template<int B, int N>
  inline bool ap_equal_ones_to(ap_int<N> value) {
    ap_uint<N> v = value;
    return ap_equal_ones_to<B, N>(v);
  }

  // if bits [B, N-1] are all ones
  template<int B, int N>
  inline bool ap_equal_zeros_from(ap_uint<N> value) {
    const int L = AC_MAX(N - B, 1);
    ap_uint<L> v = bit_slc<L, B, N>(value);
    return v == 0;
  }

  template<int B, int N>
  inline bool ap_equal_zeros_from(ap_int<N> value) {
    ap_uint<N> v = value;
    return ap_equal_zeros_from<B, N>(v);
  }

  template<int B, int N>
  inline bool ap_equal_ones_from(ap_uint<N> value) {
    const int L = AC_MAX(N - B, 1);
    ap_uint<L> v = bit_slc<L, B, N>(value);
    return (~v) == 0;
  }

  template<int B, int N>
  inline bool ap_equal_ones_from(ap_int<N> value) {
    ap_uint<N> v = value;
    return ap_equal_ones_from<B, N>(v);
  }

  // Build an ap_int from double d, value is d * 2 ^ N
  // Notice: in ref, it's d * 2 ^ (32 * N)
  template<int N, bool S>
  inline void ap_conv_from_fraction(double d, ap_int<N> & r, bool *qb, bool *rbits, bool *o, int *io) {
    bool b = d < 0;
    double d2 = b ? -d : d;
    double dfloor = mgc_floor(d2);
    *o = dfloor != 0.0;
    d2 = d2 - dfloor;
    const int shift_amount = N  + 1;
    const int container_length = (shift_amount + 31)/32 * 32 + 1;
    ap_int<N + 1> tb;
    ap_uint<container_length> k1;
    const Ulong u64_1 = 1;
    //for performance purpose, manually unroll the loop when shift_amount <= 64
    if(shift_amount <= 32){
      d2 *= u64_1 << shift_amount;
      k1 = (unsigned int) d2; //(unsigned int)floor(d2);
      tb = b ? ~k1 : k1;
      d2 -= k1;
    }
    else if(shift_amount <= 64){
      d2 *= u64_1 << 32;
      unsigned int temp = (unsigned int) d2; // (unsigned int)floor(d2);
      k1 = temp;
      d2 -= temp;
      const int shift_next = AC_MAX(shift_amount - 32, 0);
      d2 *= u64_1 << shift_next;
      temp = (unsigned int) d2;// (unsigned int)floor(d2);
      k1 <<= shift_next;
      k1 |= temp;
      d2 -= temp;
      tb = b? ~k1 : k1;
    }
    else {
      k1 = 0;
      int to_shift = shift_amount;
      unsigned int temp;
      while(to_shift >= 32){
        to_shift -= 32;
        d2 *= u64_1 << 32;
        temp = (unsigned int) d2; // (unsigned int)floor(d2);
        k1 <<= 32;
        k1 |= temp;
        d2 -= temp;
      }
      const int shift_next = AC_MAX(to_shift % 32, 0);
      d2 *= u64_1 << shift_next;
      temp = (unsigned int) d2;// (unsigned int)floor(d2);
      k1 <<= shift_next;
      k1 |= temp;
      d2 -= temp;
      tb = b? ~k1 : k1;
    }

    r = tb;
    d2 *= 2;
    bool k = (int(d2)) != 0; //math
    d2 -= k ? 1.0 : 0.0;
    *rbits = d2 != 0.0;
    *qb = (b && *rbits) ^ k;
    if(b && !*rbits && !*qb){
      r +=1;
    }
    *io = 0;
    bool cond1 = !ap_equal_zeros_from<N>(k1);
    if(!S){
      if(b) *io = -1;
      else if(cond1) *io = 1;
    } else{
      //             |    | N - 1 bits digi_bits
      //             |Sbit|
      // cond1: not 0|X   |.........: inner overflow
      // cond2      0|1   |000000000: not inner overflow, *io = -2
      // cond3:     0|1   |not all 0: inner overflow
      if(b){
        bool sign_bit = bit_slc<1, N-1>(k1);
        bool digi_bits_zero = ap_equal_zeros_to<N-1>(k1);
        if(cond1) *io = -1; // cond1
        else if(sign_bit){
          if(digi_bits_zero) *io = -2; // cond2
          else *io = -1; // cond3
        }
      } else{
        if(!ap_equal_zeros_from<N-1>(k1)) *io = 1;
      }
    }
    *o |= b ^ (( tb < 0) && S);
  }
  template<int N, bool S>
  inline void ap_conv_from_fraction(double d, ap_uint<N> & r, bool *qb, bool *rbits, bool *o, int *io) { 
    ap_int< N > r1;
    ap_conv_from_fraction<N, S>(d, r1, qb, rbits, o, io);
    r = r1;
  }

  inline Ulong mult_u_u(int a, int b) {
    return (Ulong) (unsigned) a * (Ulong) (unsigned) b;
  }
  inline Slong mult_u_s(int a, int b) {
    return (Ulong) (unsigned) a * (Slong) (signed) b;
  }
  inline Slong mult_s_u(int a, int b) {
    return (Slong) (signed) a * (Ulong) (unsigned) b;
  }
  inline Slong mult_s_s(int a, int b) {
    return (Slong) (signed) a * (Slong) (signed) b;
  }
  inline void accumulate(Ulong a, Ulong &l1, Slong &l2) {
    l1 += (Ulong) (unsigned) a;
    l2 += a >> 32;
  }
  inline void accumulate(Slong a, Ulong &l1, Slong &l2) {
    l1 += (Ulong) (unsigned) a;
    l2 += a >> 32;
  }

  template<int N>
  inline bool ap_uadd_carry(ap_uint<N> op, bool carry, ap_uint<N> & r) {
    r += carry;
    return carry && (r == 0);
  }

  template<int N>
  inline bool ap_uadd_carry(ap_int<N> op, bool carry, ap_int<N> & r) {
    ap_uint<N> ur = r;
    bool ret =  ap_uadd_carry( (ap_uint<N>)(op), carry, ur);
    r = ur;
    return ret;
  }

  //Helper function for multiplication on x86
  template <int N1, int N2>
  inline ap_uint<N1 + N2> bit_multiply(ap_uint<N1> v1, ap_uint<N2> v2) {
    ap_uint<N1 + N2 > x1 = v1;
    ap_uint<N2 > x2 = v2;

    ap_uint<N1 + N2> r = 0;
    while (x2 != 0) {
      if (x2 & 1) {
        r += x1;
      }
      x1 <<= 1;
      x2 >>= 1;
    }
    return r;
  }

  //Helper function for pow on x86
  template <int N, int P>
  inline ap_uint<N * P> ap_int_pow(ap_uint<N> value) {
    const int Nr = N * P;
    ap_uint<Nr> base = value;
    ap_uint<Nr> r = 1;
    int pow = P;
    while (pow > 0){
      if( pow % 2 == 0){
        pow /= 2;
        base = bit_multiply<Nr, Nr>(base, base);
      } else {
        pow -= 1;
        r = bit_multiply<Nr, Nr>(r, base);
        pow /= 2;
        base = bit_multiply<Nr, Nr>(base, base);
      }
    }
    return r;
  }

  //Helper function for large divisions on x86
  template <int N>
  inline ap_uint<N> bit_division(ap_uint<N> value, ap_uint<N> divisor, ap_uint<N> & remainder) {
    ap_uint<N> quotient = 1;
    ap_uint<N> tempdivisor = divisor;
    if (value == tempdivisor) {
      remainder = 0;
      return 1;
    } else if (value < tempdivisor) {
      remainder = value;
      return 0;
    }
    while ((tempdivisor << 1) <= value){
      tempdivisor = tempdivisor << 1;
      quotient = quotient << 1;
    }
    quotient = quotient + bit_division(value - tempdivisor, divisor, remainder);
    return quotient;
  }

  template <int N>
  inline ap_uint<N> bit_division(ap_uint<N> value, ap_uint<N> divisor) {
    ap_uint<N> r = 0;
    return bit_division<N>(value, divisor, r);
  }

  // template <int N>
  // inline std::string to_string(ap_uint<N> value, int base) {
  //   std::string buf = "";

  //   if (base < 2 || base > 16) return buf;

  //   enum { kMaxDigits = 35 };
  //   buf.reserve(kMaxDigits);

  //   const int N_bits = AC_MAX(N + 1, 5);
  //   ap_uint<N_bits> quotient = value;
  //   int mod;
  //   ap_uint<N_bits> b = base;
  //   do {
  //     ap_uint<N_bits> r = 0;
  //     quotient = bit_division<N_bits>(quotient, b, r);
  //     mod = (int)(r);
  //     buf += "0123456789abcdef"[ mod ];
  //   } while (quotient);

  //   std::reverse(buf.begin(), buf.end());
  //   return buf;
  // }

  // template <int N>
  // inline std::string to_string_u(ap_uint<N> value, int base) {
  //   return to_string(value,base);
  // }

  // template <int N>
  // inline std::string to_string(ap_int<N> value, int base) {
  //   ap_int<N + 1> v = value;
  //   if(value >= 0){
  //     ap_uint<N + 1> t = v;
  //     return to_string(t, base);
  //   } else {
  //     ap_uint<N + 1> t = -v;
  //     return "-" + to_string(t, base);
  //   }
  // }

  template<int W, bool S> struct select_type {};

  // The i++ flow type selections ...
  template<int W>
  struct select_type<W, true> {
    using type = int __attribute__((__ap_int(W)));
    // typedef ap_int<W> type;
  };

  template<int W>
  struct select_type<W, false> {
    using type = unsigned int __attribute__((__ap_int(W)));
    // typedef ap_uint<W> type;
  };

  //////////////////////////////////////////////////////////////////////////////
  //  Integer Vector class: iv
  //////////////////////////////////////////////////////////////////////////////
  template<int N, bool S>
  class iv {
  protected:
    typedef typename select_type<N, S>::type actype;
    actype value;
  public:
    template<int N2, bool S2> friend class iv;
    constexpr iv() {}
    template<int N2, bool S2>
    iv ( const iv<N2, S2> &b ) {
      value = b.value;
    }
    /* Note: char and short constructors are an extension to Calypto's
       implementation to address the i++ default behaviour of not promoting
       to integers. (these functions are not in Calypto's ac_int.h) */
    constexpr iv ( char t) : value(t) {}

    constexpr iv ( unsigned char t) : value(t) {}

    constexpr iv ( short t) : value(t) {}

    constexpr iv ( unsigned short t) : value(t) {}

    constexpr iv ( Slong t) : value(t) {}

    constexpr iv ( Ulong t) : value(t) {}

    constexpr iv ( int t) : value(t) {}

    constexpr iv ( unsigned int t) : value(t) {}

    constexpr iv ( long t) : value(t) {}

    constexpr iv ( unsigned long t) : value(t) {}

    constexpr iv ( double d ) : value((actype) (long long)d) {}

    constexpr iv ( float d ) : value((actype) (long long)d) {}

    // Explicit conversion functions to C built-in types -------------
    inline Slong to_int64() const {
      return (Slong) value;
    }
    inline Ulong to_uint64() const {
      return (Ulong) value;
    }
    inline double to_double() const {
      volatile Slong t = (Slong) value;
      return (double) (t);
    }

    // std::string to_string(ac_base_mode mode) const {
    //   return ac_private::to_string<N>(value, mode);
    // }

    //BEGIN: debug functions for X86 flow
    // template<int N2, bool S2>
    // void debug_within_range(const iv<N2, S2> &op2){
    // #ifdef HLS_X86
    // #if defined(DEBUG_AC_INT_WARNING) || defined(DEBUG_AC_INT_ERROR)
    //   enum {Nx = AC_MAX(N, N2 + 1) };
    //   ap_int<N2 + 1> v = op2.value;
    //   if(N2 + 1 <= N) return ;
    //   // S -> S, check bits [N2 + 1, .. , N-1]
    //   if(S){
    //     if(ap_equal_ones_from<N - 1, N2 + 1>(v)) return ;
    //     if(ap_equal_zeros_from<N - 1, N2 + 1>(v)) return ;
    //   }
    //   // S -> U, check bits [N2 + 1, .. , N]
    //   else{
    //     if(ap_equal_zeros_from<N, N2 + 1>(v)) return ;
    //   }
    //   #if defined (__linux__)
    //   std::cout << "warning: overflow, assign value " \
    //   << ac_private::to_string(v, 10) \
    //   << " to type ac_int<" << N << ", " << (S? "true" : "false") << ">" \
    //   << std::endl;
    //   #else
    //   printf("warning: overflow, assign value %s to type ac_int<%d, %s>\n",
    //     ac_private::to_string(v, 10).c_str(), N, S? "true" : "false");
    //   #endif

    //   #ifdef DEBUG_AC_INT_ERROR
    //   AC_ASSERT(0, "Assert due to overflow (DEBUG_AC_INT_ERROR)");
    //   #endif
    // #endif
    // #endif 
    // }

    // void debug_within_range(Ulong v){
    //   debug_within_range(iv<64, false>(v));
    // }

    // void debug_within_range(Slong v){
    //   debug_within_range(iv<64, true>(v));
    // }
    //END

    // o: outer overflow
    // io: inner overflow
    // qb: qb
    // rbits: r
    inline void conv_from_fraction(double d, bool *qb, bool *rbits, bool *o, int *io) {
      ap_conv_from_fraction<N, S>(d, value, qb, rbits, o, io);
    }

    // BEGIN: utility functions for x86
    #define MAX_BITS 512
    // template<int Nr>
    // inline void decompose_usigned(ap_uint<Nr> a, std::vector<Ulong> & vec ) const {
    //   while (a > 0){
    //     Ulong mask = 0XFFFFFFFF; //use a variable to avoid compiler warning
    //     int shift_amount = 32;
    //     if(Nr <= 32){
    //       // a is small
    //       vec.push_back(a);
    //       a = 0;
    //     } else{
    //       vec.push_back(a & mask);
    //       a >>= shift_amount;
    //     }
    //   }
    // }

    // template<int Nr, bool Sr>
    // inline void compose_unsigned(iv<Nr, Sr> &r, std::vector<Ulong> & vec) const {
    //   r = 0;
    //   int shift_value = 32;
    //   for(int i = vec.size() - 1; i >= 0; i--){
    //     ap_uint<Nr> t = vec[i];
    //     r.value <<= shift_value;
    //     r.value += t;
    //   }
    // }

    // template<int N2, bool S2, int Nr, bool Sr>
    // inline void mult_x86(const iv<N2, S2> &op2, iv<Nr, Sr> &r) const {

    //   ap_int<N2 + 1> ext_v1 = op2.value;
    //   ap_int<N + 1> ext_v2 = value;

    //   bool sign1 = ext_v1 >= 0;
    //   bool sign2 = ext_v2 >= 0;

    //   ap_uint<N2 + 2> a1 = sign1 ? ext_v1: -ext_v1;
    //   ap_uint<N + 2> a2 = sign2 ? ext_v2: -ext_v2;

    //   bool ret_sign = ~(sign1 ^ sign2);

    //   std::vector<Ulong> v1;
    //   std::vector<Ulong> v2;

    //   decompose_usigned<N2 + 2>(a1, v1);
    //   decompose_usigned<N + 2>(a2, v2);

    //   int shift_amount = 32;
    //   Ulong base = 1ULL << shift_amount;

    //   std::vector<Ulong> v3(v1.size() + v2.size());
    //   for(unsigned i = 0; i < v1.size(); i++){
    //     Ulong carry = 0;
    //     for(unsigned j = 0; j < v2.size(); j++){
    //       v3[i + j] += carry + v1[i] * v2[j];
    //       carry = v3[i + j] / base;
    //       v3[i + j] = v3[i + j] % base;
    //     }
    //     v3[i + v2.size()] += carry;
    //   }

    //   compose_unsigned(r, v3);
    //   if(!ret_sign){
    //     r.value = -r.value;
    //   }
    // }

    template<int Nr, bool Sr>
    inline void shift_l_x86(unsigned shift, iv<Nr, Sr> &r) const {
      int shift_amount = 64;
      while(shift >= shift_amount){
        r.value <<= shift_amount;
        shift -= shift_amount;
      }
      if(shift){
        r.value <<= shift;
      }
    }

    template<int Nr, bool Sr>
    inline void shift_r_x86(unsigned shift, iv<Nr, Sr> &r) const {
      int shift_amount = 64;
      iv<N, S> temp(*this);
      while(shift >= shift_amount){
        temp.value >>= shift_amount;
        shift -= shift_amount;
      }
      if(shift){
        temp.value >>= shift;
      }
      r.value = temp.value;
    }
    // END: utility functions for x86

    template<int N2, bool S2, int Nr, bool Sr>
    inline void mult(const iv<N2, S2> &op2, iv<Nr, Sr> &r) const {
      // ACINT_STATIC_ASSERT(N2 <= 512);
      // ACINT_STATIC_ASSERT(Nr <= 512);
      #ifdef HLS_X86
      mult_x86(op2, r);
      #else
      r.value = value;
      r.value *= op2.value;
      #endif
    }
    template<int N2, bool S2, int Nr, bool Sr>
    void add(const iv<N2, S2> &op2, iv<Nr, Sr> &r) const {
      r.value = value;
      r.value += op2.value;
    }
    template<int N2, bool S2, int Nr, bool Sr>
    void sub(const iv<N2, S2> &op2, iv<Nr, Sr> &r) const {
      r.value = value;
      r.value -= op2.value;
    }
    template<int N2, bool S2, int Nr, bool Sr>
    void div(const iv<N2, S2> &op2, iv<Nr, Sr> &r) const {
      // ACINT_STATIC_ASSERT(N2 <= 64);
      // ACINT_STATIC_ASSERT(Nr <= 64);
      typedef typename select_type<N2,S2>::type adivtype;
      typedef typename select_type<Nr,Sr>::type bdivtype;
      adivtype a = op2.value;
      bdivtype b = value;
      r.value = b/a;
    }
    template<int N2, bool S2, int Nr, bool Sr>
    void rem(const iv<N2, S2> &op2, iv<Nr, Sr> &r) const {
      r.value = value;
      r.value %= op2.value;
    }
    void increment() {
      value += 1;
    }
    void decrement() {
      value -= 1;
    }
    template<int Nr, bool Sr>
    void neg(iv<Nr, Sr> &r) const {
      r.value = value;
      r.value = -r.value;
    }

    // Shift Operators
    template<int Nr, bool Sr>
    void shift_l(unsigned op2, iv<Nr, Sr> &r) const {
      if (op2 >= Nr) {
        r.value = 0;
      } else {
        r.value = value;
        #ifdef HLS_X86
        shift_l_x86(op2, r);
        #else
        r.value <<= op2;
        #endif
      }
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
// Avoid a clang compiler warning below by temporarily suppressing
// a autological-compare warning to keep the compiler output
// tidy for the customer.
//
// warning: comparison of unsigned expression < 0 is always false [-Wtautological-compare]
//      } else if ( (op2 >= Nr) && Sr && (value < 0) ) {
//                                        ~~~~~ ^ ~
// Note that since this expression is evaluated at compile time, the compiler
// will throw a warning in the case the expression is always 0.
// So just suppress it.
//
    template<int Nr, bool Sr>
    void shift_l2(signed op2, iv<Nr, Sr> &r) const {
      signed shift = AC_ABS(op2); // abs(op2);
      if (shift >= Nr) {
        shift = Nr;
      }

      if (op2 > 0) {
        if (shift == Nr) {
          r.value = 0;
        } else {
          r.value = value;
          #ifdef HLS_X86
          shift_l_x86(shift, r);
          #else
          r.value <<= shift;
          #endif
        }
      } else {
        if (shift == Nr) {
          if (value < 0) {
            r.value = -1;
          } else {
            r.value = 0;
          }
        } else {
          #ifdef HLS_X86
          shift_r_x86(shift, r);
          #else
          r.value = value >> shift;
          #endif
        }
      }
    }
#pragma clang diagnostic pop

    // template<int Nr, int Sr, int B>
    // void const_shift_l(iv<Nr, Sr> &r) const {
    //   shift_l2<Nr, Sr>(B, r);
    // }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wtautological-compare"
    template<int Nr, bool Sr>
    void shift_r(unsigned op2, iv<Nr, Sr> &r) const {
      if ( (op2 >= Nr) && ((Sr && (value > 0)) || !Sr) ) {
        r.value = 0;
      } else if ( (op2 >= Nr) && Sr && (value < 0) ) {
        r.value = -1;
      } else {
        #ifdef HLS_X86
          shift_r_x86(op2, r);
        #else
          r.value = value >> op2;
        #endif
      }
    }

    template<int Nr, bool Sr>
    void shift_r2(signed op2, iv<Nr, Sr> &r) const {
      signed shift = AC_ABS(op2); // abs(op2);
      const int Ns = AC_MAX(Nr, N);
      if (shift >= Ns) {
          shift = Ns;
      }
      if (op2 > 0) {
        if (shift == Ns) {
          if (value < 0) {
            r.value = -1;
          } else {
            r.value = 0;
          }
        } else {
          #ifdef HLS_X86
            shift_r_x86(shift, r);
          #else
            r.value = value >> shift;
          #endif
        }
      } else {
        if (shift == Ns) {
          r.value = 0;
        } else {
          r.value = value;
          #ifdef HLS_X86
          shift_l_x86(shift, r);
          #else
          r.value <<= shift;
          #endif
        }
      }
    }
#pragma clang diagnostic pop

    template<int Nr, bool Sr, int B>
    void const_shift_r(iv<Nr, Sr> &r) const {
      iv<Nr, Sr> t;
      shift_r2<Nr, Sr>(B, t);
      r = t;
    }

    template<int Nr, bool Sr>
    void bitwise_complement(iv<Nr, Sr> &r) const {
      r.value = value;
      r.value = ~r.value;
    }
    template<int N2, bool S2, int Nr, bool Sr>
    void bitwise_and(const iv<N2, S2> &op2, iv<Nr, Sr> &r) const {
      r.value = value;
      r.value &= op2.value;
    }
    template<int N2, bool S2, int Nr, bool Sr>
    void bitwise_or(const iv<N2, S2> &op2, iv<Nr, Sr> &r) const {
      r.value = value;
      r.value |= op2.value;
    }
    template<int N2, bool S2, int Nr, bool Sr>
    void bitwise_xor(const iv<N2, S2> &op2, iv<Nr, Sr> &r) const {
      r.value = value;
      r.value ^= op2.value;
    }
    template<int N2, bool S2>
    bool equal(const iv<N2, S2> &op2) const {
      enum {Sx = AC_MAX(N, N2) };
      ap_int<Sx+1> a = (ap_int<Sx+1>)value;
      ap_int<Sx+1> b = (ap_int<Sx+1>)op2.value;
      return (a == b);
    }
    template<int N2, bool S2>
    bool greater_than(const iv<N2, S2> &op2) const {
      enum {Sx = AC_MAX(N, N2) };
      ap_int<Sx+1> a = (ap_int<Sx+1>)value;
      ap_int<Sx+1> b = (ap_int<Sx+1>)op2.value;
      return (a > b);
    }
    template<int N2, bool S2>
    bool less_than(const iv<N2, S2> &op2) const {
      enum {Sx = AC_MAX(N, N2) };
      ap_int<Sx+1> a = (ap_int<Sx+1>)value;
      ap_int<Sx+1> b = (ap_int<Sx+1>)op2.value;
      return (a < b);
    }
    bool equal_zero() const {
      actype zero = 0;
      return (value == zero);
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshift-count-overflow"
    template<int N2, bool S2>
    void set_slc(unsigned lsb, int WS, const iv<N2, S2> &op2) {
      // AC_ASSERT(N2<=N, "Bad usage: WS greater than length of slice");
      if(N2 == N){
        value = op2.value;
      }
      else if (N2 <= N) {
        iv<N,S> temp;
        temp.value = (actype) op2.value;
        #ifdef HLS_X86
        shift_l_x86(lsb, temp);
        #else
        temp.value <<= lsb;
        #endif
        // Compute AND mask
        iv<N,S> mask;
        mask.value = 1;
        #ifdef HLS_X86
        shift_l_x86(N2, mask);
        #else
        mask.value <<= N2;
        #endif
        mask.value -= 1;
        #ifdef HLS_X86
        shift_l_x86(lsb, mask);
        #else
        mask.value <<= lsb;
        #endif

        mask.value = ~mask.value;
        value &= mask.value;
        value |= temp.value;
      } else {
        value = 0;
      }
    }
#pragma clang diagnostic pop

    unsigned leading_bits(bool bit) const {
      return 0;
    }

    template <int Nr, bool Sr>
    void reverse(iv<Nr, Sr> &r) const {
      r.value = 0;
      for (int i = 0; i < N; i++) {
        r.value |= ((value >> i) & 1) << ((N - 1) -i);
      }
    }
  }; // class iv, signed

  /* TODO
  template<> inline Slong iv<1>::to_int64() const { return v[0]; }
  template<> inline Ulong iv<1>::to_uint64() const { return v[0]; }

  template<> inline Slong iv<2>::to_int64() const {
    return ((Ulong)v[1] << 32) | (Ulong) (unsigned) v[0];
  }
  template<> inline Ulong iv<2>::to_uint64() const {
    return ((Ulong)v[1] << 32) | (Ulong) (unsigned) v[0];
  }

  template<> template<> inline void iv<1>::set_slc(unsigned lsb, int WS, const iv<1> &op2) {
    v[0] ^= (v[0] ^ (op2.v[0] << lsb)) & (~(WS==32 ? 0 : ~0<<WS) << lsb);
  }
  template<> template<> inline void iv<2>::set_slc(unsigned lsb, int WS, const iv<1> &op2) {
    Ulong l = to_uint64();
    Ulong l2 = op2.to_uint64();
    l ^= (l ^ (l2 << lsb)) & (~((~(Ulong)0)<<WS) << lsb);  // WS <= 32
    *this = l;
  }
  template<> template<> inline void iv<2>::set_slc(unsigned lsb, int WS, const iv<2> &op2) {
    Ulong l = to_uint64();
    Ulong l2 = op2.to_uint64();
    l ^= (l ^ (l2 << lsb)) & (~(WS==64 ? (Ulong) 0 : ~(Ulong)0<<WS) << lsb);
    *this = l;
  }
  */

  // add automatic conversion to Slong/Ulong depending on S and C
  template<int N, bool S, bool C>
  class iv_conv : public iv<N, S> {
  protected:
    constexpr iv_conv() {}
    template<class T> constexpr iv_conv(const T& t) : iv<N, S>(t) {}
  };

  template<int N>
  class iv_conv<N,false,true> : public iv<N, false> {
  public:
    operator Ulong () const { return iv<N, false>::to_uint64(); }
  protected:
    constexpr iv_conv() {}
    template<class T> constexpr iv_conv(const T& t) : iv<N, false>(t) {}
  };

  template<int N>
  class iv_conv<N,true,true> : public iv<N, true> {
  public:
    operator Slong () const { return iv<N, true>::to_int64(); }
  protected:
    constexpr iv_conv() {}
    template<class T> constexpr iv_conv(const T& t) : iv<N, true>(t) {}
  };

  // Set default to promote to int as this is the case for almost all types
  //  create exceptions using specializations
  template<typename T>
  struct c_prom {
    typedef int promoted_type;
  };
  template<> struct c_prom<unsigned> {
    typedef unsigned promoted_type;
  };
  template<> struct c_prom<long> {
    typedef long promoted_type;
  };
  template<> struct c_prom<unsigned long> {
    typedef unsigned long promoted_type;
  };
  template<> struct c_prom<Slong> {
    typedef Slong promoted_type;
  };
  template<> struct c_prom<Ulong> {
    typedef Ulong promoted_type;
  };
  template<> struct c_prom<float> {
    typedef float promoted_type;
  };
  template<> struct c_prom<double> {
    typedef double promoted_type;
  };

  template<typename T, typename T2>
  struct c_arith {
     // will error out for pairs of T and T2 that are not defined through specialization
  };
  template<typename T> struct c_arith<T,T> {
    typedef T arith_conv;
  };

  #define C_ARITH(C_TYPE1, C_TYPE2) \
  template<> struct c_arith<C_TYPE1, C_TYPE2> { \
    typedef C_TYPE1 arith_conv; \
  }; \
  template<> struct c_arith<C_TYPE2, C_TYPE1> { \
    typedef C_TYPE1 arith_conv; \
  };

  C_ARITH(double, float)
  C_ARITH(double, int)
  C_ARITH(double, unsigned)
  C_ARITH(double, long)
  C_ARITH(double, unsigned long)
  C_ARITH(double, Slong)
  C_ARITH(double, Ulong)
  C_ARITH(float, int)
  C_ARITH(float, unsigned)
  C_ARITH(float, long)
  C_ARITH(float, unsigned long)
  C_ARITH(float, Slong)
  C_ARITH(float, Ulong)

  C_ARITH(Slong, int)
  C_ARITH(Slong, unsigned)
  C_ARITH(Ulong, int)
  C_ARITH(Ulong, unsigned)

  template<typename T>
  struct map {
    typedef T t;
  };
  template<typename T>
  struct c_type_params {
    // will error out for T for which this template struct is not specialized
  };

  template<typename T> inline const char *c_type_name() { return "unknown"; }
  template<> inline const char *c_type_name<bool>() { return "bool";}
  template<> inline const char *c_type_name<char>() { return "char";}
  template<> inline const char *c_type_name<signed char>() { return "signed char";}
  template<> inline const char *c_type_name<unsigned char>() { return "unsigned char";}
  template<> inline const char *c_type_name<signed short>() { return "signed short";}
  template<> inline const char *c_type_name<unsigned short>() { return "unsigned short";}
  template<> inline const char *c_type_name<int>() { return "int";}
  template<> inline const char *c_type_name<unsigned>() { return "unsigned";}
  template<> inline const char *c_type_name<signed long>() { return "signed long";}
  template<> inline const char *c_type_name<unsigned long>() { return "unsigned long";}
  template<> inline const char *c_type_name<signed long long>() { return "signed long long";}
  template<> inline const char *c_type_name<unsigned long long>() { return "unsigned long long";}
  template<> inline const char *c_type_name<float>() { return "float";}
  template<> inline const char *c_type_name<double>() { return "double";}

  template<typename T> struct c_type;

  template<typename T>
  struct rt_c_type_T {
    template<typename T2>
    struct op1 {
      typedef typename T::template rt_T< c_type<T2> >::mult mult;
      typedef typename T::template rt_T< c_type<T2> >::plus plus;
      typedef typename T::template rt_T< c_type<T2> >::minus2 minus;
      typedef typename T::template rt_T< c_type<T2> >::minus minus2;
      typedef typename T::template rt_T< c_type<T2> >::logic logic;
      typedef typename T::template rt_T< c_type<T2> >::div2 div;
      typedef typename T::template rt_T< c_type<T2> >::div div2;
    };
  };
  template<typename T>
  struct c_type {
    typedef typename c_prom<T>::promoted_type c_prom_T;
    struct rt_unary {
      typedef c_prom_T neg;
      typedef c_prom_T mag_sqr;
      typedef c_prom_T mag;
      template<unsigned N>
      struct set {
        typedef c_prom_T sum;
      };
    };
    template<typename T2>
    struct rt_T {
      typedef typename rt_c_type_T<T2>::template op1<T>::mult mult;
      typedef typename rt_c_type_T<T2>::template op1<T>::plus plus;
      typedef typename rt_c_type_T<T2>::template op1<T>::minus minus;
      typedef typename rt_c_type_T<T2>::template op1<T>::minus2 minus2;
      typedef typename rt_c_type_T<T2>::template op1<T>::logic logic;
      typedef typename rt_c_type_T<T2>::template op1<T>::div div;
      typedef typename rt_c_type_T<T2>::template op1<T>::div2 div2;
    };
    // inline static std::string type_name() {
    //   std::string r = c_type_name<T>();
    //   return r;
    // }

  };
  // with T == c_type
  template<typename T>
  struct rt_c_type_T< c_type<T> > {
    typedef typename c_prom<T>::promoted_type c_prom_T;
    template<typename T2>
    struct op1 {
      typedef typename c_prom<T2>::promoted_type c_prom_T2;
      typedef typename c_arith< c_prom_T, c_prom_T2 >::arith_conv mult;
      typedef typename c_arith< c_prom_T, c_prom_T2 >::arith_conv plus;
      typedef typename c_arith< c_prom_T, c_prom_T2 >::arith_conv minus;
      typedef typename c_arith< c_prom_T, c_prom_T2 >::arith_conv minus2;
      typedef typename c_arith< c_prom_T, c_prom_T2 >::arith_conv logic;
      typedef typename c_arith< c_prom_T, c_prom_T2 >::arith_conv div;
      typedef typename c_arith< c_prom_T, c_prom_T2 >::arith_conv div2;
    };
  };

  #define C_TYPE_MAP(C_TYPE) \
  template<> struct map<C_TYPE> { \
    typedef c_type<C_TYPE> t; \
  };

  #define C_TYPE_PARAMS(C_TYPE, WI, SI) \
  template<> struct c_type_params<C_TYPE> { \
    enum { W = WI, I = WI, E = 0, S = SI, floating_point = 0 }; \
  };

  #define C_TYPE_MAP_INT(C_TYPE, WI, SI) \
    C_TYPE_MAP(C_TYPE) \
    C_TYPE_PARAMS(C_TYPE, WI, SI)

  #define C_TYPE_MAP_FLOAT(C_TYPE, FP, WFP, IFP, EFP) \
  C_TYPE_MAP(C_TYPE) \
  template<> struct c_type_params<C_TYPE> { \
    enum { W = WFP, I = IFP, E = EFP, S = true, floating_point = FP }; \
  };

  C_TYPE_MAP_INT(bool, 1, false)
  C_TYPE_MAP_INT(char, 8, true)
  C_TYPE_MAP_INT(signed char, 8, true)
  C_TYPE_MAP_INT(unsigned char, 8, false)
  C_TYPE_MAP_INT(signed short, 16, true)
  C_TYPE_MAP_INT(unsigned short, 16, false)
  C_TYPE_MAP_INT(signed int, 32, true)
  C_TYPE_MAP_INT(unsigned int, 32, false)
  // C_TYPE_MAP_INT(signed long, ac_private::long_w, true)
  // C_TYPE_MAP_INT(unsigned long, ac_private::long_w, false)
  C_TYPE_MAP_INT(signed long long, 64, true)
  C_TYPE_MAP_INT(unsigned long long, 64, false)
  C_TYPE_MAP_FLOAT(float, 1, 25, 1, 8)
  C_TYPE_MAP_FLOAT(double, 2, 54, 1, 11)

  #undef C_TYPE_INT
  #undef C_TYPE_PARAMS
  #undef C_TYPE_FLOAT
  #undef C_TYPE_MAP

  // specializations for following struct declared/defined after definition of ac_int
  template<typename T>
  struct rt_ac_int_T {
    template<int W, bool S>
    struct op1 {
      typedef typename T::template rt_T< ac_int<W,S> >::mult mult;
      typedef typename T::template rt_T< ac_int<W,S> >::plus plus;
      typedef typename T::template rt_T< ac_int<W,S> >::minus2 minus;
      typedef typename T::template rt_T< ac_int<W,S> >::minus minus2;
      typedef typename T::template rt_T< ac_int<W,S> >::logic logic;
      typedef typename T::template rt_T< ac_int<W,S> >::div2 div;
      typedef typename T::template rt_T< ac_int<W,S> >::div div2;
    };
  };
}

namespace ac {
  // compiler time constant for log2 like functions
  template<unsigned X>
  struct nbits {
    enum { val = ac_private::s_N<16>::s_X<X>::nbits };
  };

  template<unsigned X>
  struct log2_floor {
    enum { val = nbits<X>::val - 1 };
  };

  // log2 of 0 is not defined: generate compiler error
  template<> struct log2_floor<0> {};

  template<unsigned X>
  struct log2_ceil {
    enum { lf = log2_floor<X>::val, val = (X == (1 << lf) ? lf : lf+1) };
  };

  // log2 of 0 is not defined: generate compiler error
  template<> struct log2_ceil<0> {};

  template<int LowerBound, int UpperBound>
  struct int_range {
    enum { l_s = LowerBound < 0, u_s = UpperBound < 0,
           signedness = l_s || u_s,
           l_nbits = nbits<AC_ABS(LowerBound+l_s)+l_s>::val,
           u_nbits = nbits<AC_ABS(UpperBound+u_s)+u_s>::val,
           nbits = AC_MAX(l_nbits, u_nbits + (!u_s && signedness))
         };
    typedef ac_int<nbits, signedness> type;
  };
}

enum ac_q_mode { AC_TRN, AC_RND, AC_TRN_ZERO, AC_RND_ZERO, AC_RND_INF, AC_RND_MIN_INF, AC_RND_CONV, AC_RND_CONV_ODD };
enum ac_o_mode { AC_WRAP, AC_SAT, AC_SAT_ZERO, AC_SAT_SYM };
template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2> class ac_fixed;

//////////////////////////////////////////////////////////////////////////////
//  Arbitrary-Length Integer: ac_int
//////////////////////////////////////////////////////////////////////////////

template<int W, bool S=true>
class ac_int : public ac_private::iv_conv<W, S, W<=64>
{
  typedef ac_private::iv_conv<W, S, W <= 64> ConvBase;
  typedef ac_private::iv<W, S>               Base;

  inline bool is_neg() const { return S && Base::value < 0; }

  enum ac_debug_op { 
    AC_DEBUG_ADD,
    AC_DEBUG_SUB, 
    AC_DEBUG_MUL,
    AC_DEBUG_DIV,
    AC_DEBUG_REM,
    AC_DEBUG_INCREMENT,
    AC_DEBUG_DECREMENT
  };

  // returns false if number is denormal
  template<int WE, bool SE>
  bool normalize_private(ac_int<WE,SE> &exp, bool reserved_min_exp=false) {
    int expt = exp;
    int lshift = leading_sign();
    bool fully_normalized = true;
    ac_int<WE, SE> min_exp = 0;
    min_exp.template set_val<AC_VAL_MIN>();
    int max_shift = exp - min_exp - reserved_min_exp;
    if(lshift > max_shift) {
      lshift = ac_int<WE,false>(max_shift);
      expt = min_exp + reserved_min_exp;
      fully_normalized = false;
    } else {
      expt -= lshift;
    }
    if(Base::equal_zero()) {
      expt = 0;
      fully_normalized = true;
    }
    exp = expt;
    Base r;
    Base::shift_l(lshift, r);
    Base::operator=(r);
    return fully_normalized;
  }

public:
  static const int width = W;
  static const int i_width = W;
  static const bool sign = S;
  static const ac_q_mode q_mode = AC_TRN;
  static const ac_o_mode o_mode = AC_WRAP;
  static const int e_width = 0;

  template<int W2, bool S2>
  struct rt {
    enum {
      mult_w = W+W2,
      mult_s = S||S2,
      plus_w = AC_MAX(W+(S2&&!S),W2+(S&&!S2))+1,
      plus_s = S||S2,
      minus_w = AC_MAX(W+(S2&&!S),W2+(S&&!S2))+1,
      minus_s = true,
      div_w = W+S2,
      div_s = S||S2,
      mod_w = AC_MIN(W,W2+(!S2&&S)),
      mod_s = S,
      logic_w = AC_MAX(W+(S2&&!S),W2+(S&&!S2)),
      logic_s = S||S2
    };
    typedef ac_int<mult_w, mult_s> mult;
    typedef ac_int<plus_w, plus_s> plus;
    typedef ac_int<minus_w, minus_s> minus;
    typedef ac_int<logic_w, logic_s> logic;
    typedef ac_int<div_w, div_s> div;
    typedef ac_int<mod_w, mod_s> mod;
    typedef ac_int<W, S> arg1;
  };

  template<typename T>
  struct rt_T {
    typedef typename ac_private::map<T>::t map_T;
    typedef typename ac_private::rt_ac_int_T<map_T>::template op1<W,S>::mult mult;
    typedef typename ac_private::rt_ac_int_T<map_T>::template op1<W,S>::plus plus;
    typedef typename ac_private::rt_ac_int_T<map_T>::template op1<W,S>::minus minus;
    typedef typename ac_private::rt_ac_int_T<map_T>::template op1<W,S>::minus2 minus2;
    typedef typename ac_private::rt_ac_int_T<map_T>::template op1<W,S>::logic logic;
    typedef typename ac_private::rt_ac_int_T<map_T>::template op1<W,S>::div div;
    typedef typename ac_private::rt_ac_int_T<map_T>::template op1<W,S>::div2 div2;
    typedef ac_int<W, S> arg1;
  };

  struct rt_unary {
    enum {
      neg_w = W+1,
      neg_s = true,
      mag_sqr_w = 2*W-S,
      mag_sqr_s = false,
      mag_w = W+S,
      mag_s = false,
      leading_sign_w = ac::log2_ceil<W+!S>::val,
      leading_sign_s = false
    };
    typedef ac_int<neg_w, neg_s> neg;
    typedef ac_int<mag_sqr_w, mag_sqr_s> mag_sqr;
    typedef ac_int<mag_w, mag_s> mag;
    typedef ac_int<leading_sign_w, leading_sign_s> leading_sign;
    template<unsigned N>
    struct set {
      enum { sum_w = W + ac::log2_ceil<N>::val, sum_s = S};
      typedef ac_int<sum_w, sum_s> sum;
    };
  };

  template<int W2, bool S2> friend class ac_int;
  template<int W2, int I2, bool S2, ac_q_mode Q2, ac_o_mode O2> friend class ac_fixed;
  constexpr ac_int() {
    // #ifndef SUPPRESS_AC_INT_WARNINGS
    // #ifdef HLS_X86
    //   if(!warned_undef) {
    //     printf("warning: using empty constructor for type %s", type_name().c_str());
    //     printf(". note: suppress by defining SUPPRESS_AC_INT_WARNINGS\n");
    //     warned_undef = true;
    //   }
    // #endif
    // #endif
  };
  template<int W2, bool S2>
  inline ac_int (const ac_int<W2,S2> &op) {
    // Base::debug_within_range(op);
    Base::operator =(op);
  }

  constexpr inline ac_int( bool b ) : ConvBase(b) { }
  constexpr inline ac_int( char b ) : ConvBase(b) {
    // Base::debug_within_range(Ulong(b));
  }
  constexpr inline ac_int( signed char b ) : ConvBase(b) {
    // Base::debug_within_range(Slong(b));
  }
  constexpr inline ac_int( unsigned char b ) : ConvBase(b) {
    // Base::debug_within_range(Ulong(b));
  }
  constexpr inline ac_int( signed short b ) : ConvBase(b) {
    // Base::debug_within_range(Slong(b));
  }
  constexpr inline ac_int( unsigned short b ) : ConvBase(b) {
    // Base::debug_within_range(Ulong(b));
  }
  constexpr inline ac_int( signed int b ) : ConvBase(b) {
    // Base::debug_within_range(Slong(b));
  }
  constexpr inline ac_int( unsigned int b ) : ConvBase(b) {
    // Base::debug_within_range(Ulong(b));
  }
  constexpr inline ac_int( signed long b ) : ConvBase(b) {
    // Base::debug_within_range(Slong(b));
  }
  constexpr inline ac_int( unsigned long b ) : ConvBase(b) {
    // Base::debug_within_range(Ulong(b));
  }
  constexpr inline ac_int( Slong b ) : ConvBase(b) {
    // Base::debug_within_range(b);
  }
  constexpr inline ac_int( Ulong b ) : ConvBase(b) {
    // Base::debug_within_range(b);
  }
  constexpr inline ac_int( double d ) : ConvBase(d) { }

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
#endif
  template<ac_special_val V>
  inline ac_int &set_val() {
    if(V == AC_VAL_DC) {
      ac_int r = 0;
      Base::operator =(r);
    }
    else if(V == AC_VAL_0 || V == AC_VAL_MIN || V == AC_VAL_QUANTUM) {
      Base::operator =(0);
      if(S && V == AC_VAL_MIN) {
        Base::value = 1;
        Base::value <<= W - 1;
      } else if(V == AC_VAL_QUANTUM)
        Base::value = 1;
    }
    else if(AC_VAL_MAX) {
      Base::value = 0;
      Base::value = ~Base::value;
      if(S){
        ap_uint<W> t = Base::value;
        t >>= 1;
        Base::value = t;
      }
    }
    return *this;
  }
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

  // Explicit conversion functions to C built-in types -------------
  inline int to_int() const { return (int) Base::value; }
  inline unsigned to_uint() const { return (unsigned) Base::value; }
  inline long to_long() const { return (long) Base::value; }
  inline unsigned long to_ulong() const { return (unsigned long) Base::value; }
  inline Slong to_int64() const { return Base::to_int64(); }
  inline Ulong to_uint64() const { return Base::to_uint64(); }
  inline double to_double() const { return Base::to_double(); }

  inline int length() const { return W; }

  // inline std::string to_string(ac_base_mode base_rep, bool sign_mag = false) const {
  //   return Base::to_string(base_rep);
  // }

  // inline static std::string type_name() {
  //   const char *tf[] = {",false>", ",true>"};
  //   std::string r = "ac_int<";
  //   r += ac_int<32,true>(W).to_string(AC_DEC);
  //   r += tf[S];
  //   return r;
  // }

  // Arithmetic : Binary ----------------------------------------------------
  template<int W2, bool S2>
  typename rt<W2,S2>::mult operator *( const ac_int<W2,S2> &op2) const {
    typename rt<W2,S2>::mult r = 0;
    Base::mult(op2, r);
    return r;
  }
  template<int W2, bool S2>
  typename rt<W2,S2>::plus operator +( const ac_int<W2,S2> &op2) const {
    typename rt<W2,S2>::plus r = 0;
    Base::add(op2, r);
    return r;
  }
  template<int W2, bool S2>
  typename rt<W2,S2>::minus operator -( const ac_int<W2,S2> &op2) const {
    typename rt<W2,S2>::minus r = 0;
    Base::sub(op2, r);
    return r;
  }
  template<int W2, bool S2>
  typename rt<W2,S2>::div operator /( const ac_int<W2,S2> &op2) const {
    typename rt<W2,S2>::div r = 0;
    Base::div(op2, r);
    return r;
  }
  template<int W2, bool S2>
  typename rt<W2,S2>::mod operator %( const ac_int<W2,S2> &op2) const {
    typename rt<W2,S2>::mod r = 0;
    Base::rem(op2, r);
    return r;
  }
  //BEGIN: X86 DEBUG
  template<int W2, bool S2>
  void check_overflow( const ac_int<W2, S2> &op2, ac_debug_op debug_op){
  #ifdef HLS_X86
  #if defined(DEBUG_AC_INT_WARNING) || defined(DEBUG_AC_INT_ERROR)
    ac_int<W, S> temp = 0;
    switch (debug_op){
      case AC_DEBUG_ADD:
        temp = (*this) + op2;
        break;
      case AC_DEBUG_SUB:
        temp = (*this) - op2;
        break;
      case AC_DEBUG_MUL:
        temp = (*this) * op2;
        break;
      case AC_DEBUG_DIV:
        temp = (*this) / op2;
        break;
      case AC_DEBUG_REM:
        temp = (*this) % op2;
        break;

      default:
        break;

    }
  #endif
  #endif
  }

  void check_overflow(ac_debug_op debug_op){
  #ifdef HLS_X86
  #if defined(DEBUG_AC_INT_WARNING) || defined(DEBUG_AC_INT_ERROR)
    ac_int<W, S> temp = 0;
    ac_int<2, true> op2 = 1;
    switch (debug_op){
      case AC_DEBUG_INCREMENT :
        temp = (*this) + op2;
        break;
      case AC_DEBUG_DECREMENT :
        temp = (*this) - op2;
        break;

      default:
        break;
    }
  #endif
  #endif
  }


  //END: X86 DEBUG


  // Arithmetic assign  ------------------------------------------------------
  template<int W2, bool S2>
  ac_int &operator *=( const ac_int<W2,S2> &op2) {
    check_overflow(op2, AC_DEBUG_MUL);
    Base r;
    Base::mult(op2, r);
    Base::operator=(r);
    return *this;
  }
  template<int W2, bool S2>
  ac_int &operator +=( const ac_int<W2,S2> &op2) {
    check_overflow(op2, AC_DEBUG_ADD);
    Base r;
    Base::add(op2, r);
    Base::operator=(r);
    return *this;
  }
  template<int W2, bool S2>
  ac_int &operator -=( const ac_int<W2,S2> &op2) {
    check_overflow(op2, AC_DEBUG_SUB);
    Base r;
    Base::sub(op2, r);
    Base::operator=(r);
    return *this;
  }
  template<int W2, bool S2>
  ac_int &operator /=( const ac_int<W2,S2> &op2) {
    check_overflow(op2, AC_DEBUG_DIV);
    Base r;
    Base::div(op2, r);
    Base::operator=(r);
    return *this;
  }
  template<int W2, bool S2>
  ac_int &operator %=( const ac_int<W2,S2> &op2) {
    check_overflow(op2, AC_DEBUG_REM);
    Base r;
    Base::mod(op2, r);
    Base::operator=(r);
    return *this;
  }
  // Arithmetic prefix increment, decrement ----------------------------------
  ac_int &operator ++() {
    check_overflow(AC_DEBUG_INCREMENT);
    Base::increment();
    return *this;
  }
  ac_int &operator --() {
    check_overflow(AC_DEBUG_DECREMENT);
    Base::decrement();
    return *this;
  }
  // Arithmetic postfix increment, decrement ---------------------------------
  const ac_int operator ++(int) {
    check_overflow(AC_DEBUG_INCREMENT);
    ac_int t = *this;
    Base::increment();
    return t;
  }
  const ac_int operator --(int) {
    check_overflow(AC_DEBUG_DECREMENT);
    ac_int t = *this;
    Base::decrement();
    return t;
  }
  // Arithmetic Unary --------------------------------------------------------
  ac_int operator +() {
    return *this;
  }
  typename rt_unary::neg operator -() const {
    typename rt_unary::neg r = 0;
    Base::neg(r);
    return r;
  }
  // ! ------------------------------------------------------------------------
  bool operator ! () const {
    return Base::equal_zero();
  }

  // Bitwise (arithmetic) unary: complement  -----------------------------
  ac_int<W+!S, true> operator ~() const {
    ac_int<W+!S, true> r = 0;
    Base::bitwise_complement(r);
    return r;
  }
  // Bitwise (non-arithmetic) bit_complement  -----------------------------
  ac_int<W, false> bit_complement() const {
    ac_int<W, false> r = 0;
    Base::bitwise_complement(r);
    return r;
  }
  // Bitwise (arithmetic): and, or, xor ----------------------------------
  template<int W2, bool S2>
  typename rt<W2,S2>::logic operator & ( const ac_int<W2,S2> &op2) const {
    typename rt<W2,S2>::logic r = 0;
    Base::bitwise_and(op2, r);
    return r;
  }
  template<int W2, bool S2>
  typename rt<W2,S2>::logic operator | ( const ac_int<W2,S2> &op2) const {
    typename rt<W2,S2>::logic r = 0;
    Base::bitwise_or(op2, r);
    return r;
  }
  template<int W2, bool S2>
  typename rt<W2,S2>::logic operator ^ ( const ac_int<W2,S2> &op2) const {
    typename rt<W2,S2>::logic r = 0;
    Base::bitwise_xor(op2, r);
    return r;
  }
  // Bitwise assign (not arithmetic): and, or, xor ----------------------------
  template<int W2, bool S2>
  ac_int &operator &= ( const ac_int<W2,S2> &op2 ) {
    Base r;
    Base::bitwise_and(op2, r);
    Base::operator=(r);
    return *this;
  }
  template<int W2, bool S2>
  ac_int &operator |= ( const ac_int<W2,S2> &op2 ) {
    Base r;
    Base::bitwise_or(op2, r);
    Base::operator=(r);
    return *this;
  }
  template<int W2, bool S2>
  ac_int &operator ^= ( const ac_int<W2,S2> &op2 ) {
    Base r;
    Base::bitwise_xor(op2, r);
    Base::operator=(r);
    return *this;
  }
  // Shift (result constrained by left operand) -------------------------------
  template<int W2>
  ac_int operator << ( const ac_int<W2,true> &op2 ) const {
    ac_int r = 0;
    Base::shift_l2(op2.to_int(), r);
    return r;
  }
  template<int W2>
  ac_int operator << ( const ac_int<W2,false> &op2 ) const {
    ac_int r = 0;
    Base::shift_l(op2.to_uint(), r);
    return r;
  }
  template<int W2>
  ac_int operator >> ( const ac_int<W2,true> &op2 ) const {
    ac_int r = 0;
    Base::shift_r2(op2.to_int(), r);
    return r;
  }
  template<int W2>
  ac_int operator >> ( const ac_int<W2,false> &op2 ) const {
    ac_int r = 0;
    Base::shift_r(op2.to_uint(), r);
    return r;
  }
  // Shift assign ------------------------------------------------------------
  template<int W2>
  ac_int &operator <<= ( const ac_int<W2,true> &op2 ) {
    Base r;
    Base::shift_l2(op2.to_int(), r);
    Base::operator=(r);
    return *this;
  }
  template<int W2>
  ac_int &operator <<= ( const ac_int<W2,false> &op2 ) {
    Base r;
    Base::shift_l(op2.to_uint(), r);
    Base::operator=(r);
    return *this;
  }
  template<int W2>
  ac_int &operator >>= ( const ac_int<W2,true> &op2 ) {
    Base r;
    Base::shift_r2(op2.to_int(), r);
    Base::operator=(r);
    return *this;
  }
  template<int W2>
  ac_int &operator >>= ( const ac_int<W2,false> &op2 ) {
    Base r;
    Base::shift_r(op2.to_uint(), r);
    Base::operator=(r);
    return *this;
  }
  // Relational ---------------------------------------------------------------
  template<int W2, bool S2>
  bool operator == ( const ac_int<W2,S2> &op2) const {
    return Base::equal(op2);
  }
  template<int W2, bool S2>
  bool operator != ( const ac_int<W2,S2> &op2) const {
    return !Base::equal(op2);
  }
  template<int W2, bool S2>
  bool operator < ( const ac_int<W2,S2> &op2) const {
    return Base::less_than(op2);
  }
  template<int W2, bool S2>
  bool operator >= ( const ac_int<W2,S2> &op2) const {
    return !Base::less_than(op2);
  }
  template<int W2, bool S2>
  bool operator > ( const ac_int<W2,S2> &op2) const {
    return Base::greater_than(op2);
  }
  template<int W2, bool S2>
  bool operator <= ( const ac_int<W2,S2> &op2) const {
    return !Base::greater_than(op2);
  }

  // Bit and Slice Select -----------------------------------------------------
  template<int WS, int WX, bool SX>
  inline ac_int<WS,S> slc(const ac_int<WX,SX> &index) const {
    ac_int<W, S> op = *this;
    ac_int<WS, S> r = 0;
    // AC_ASSERT(index >= 0, "Attempting to read slc with negative indeces");
    ac_int<WX-SX, false> uindex = index;
    Base::shift_r(uindex.to_uint(), op);
    r = op;
    return r;
  }

  template<int WS>
  inline ac_int<WS,S> slc(signed index) const {
    ac_int<W, S> op = *this;
    ac_int<WS,S> r = 0;
    // AC_ASSERT(index >= 0, "Attempting to read slc with negative indeces");
    unsigned uindex = index & ((unsigned)~0 >> 1);
    Base::shift_r(uindex, op);
    r = op;
    return r;
  }
  template<int WS>
  inline ac_int<WS,S> slc(unsigned uindex) const {
    ac_int<W,S> op = *this;
    Base::shift_r(uindex, op);
    ac_int<WS,S> r = op;
    return r;
  }

  template<int W2, bool S2, int WX, bool SX>
  inline ac_int &set_slc(const ac_int<WX,SX> lsb, const ac_int<W2,S2> &slc) {
    // AC_ASSERT(lsb.to_int() + W2 <= W && lsb.to_int() >= 0, "Out of bounds set_slc");
    if (lsb.to_int() + W2 <= W && lsb.to_int() >= 0) {
        ac_int<WX-SX, false> ulsb = lsb;
        Base::set_slc(ulsb.to_uint(), W2, (ac_int<W2,false>) slc);
    } else {
        Base r = 0;
        Base::operator=(r);
    }
    return *this;
  }
  template<int W2, bool S2>
  inline ac_int &set_slc(signed lsb, const ac_int<W2,S2> &slc) {
    // AC_ASSERT(lsb + W2 <= W && lsb >= 0, "Out of bounds set_slc");
    if (lsb + W2 <= W && lsb >= 0) {
        unsigned ulsb = lsb & ((unsigned)~0 >> 1);
        Base::set_slc(ulsb, W2, (ac_int<W2,false>) slc);
    } else {
        Base r = 0;
        Base::operator=(r);
    }
    return *this;
  }
  template<int W2, bool S2>
  inline ac_int &set_slc(unsigned ulsb, const ac_int<W2,S2> &slc) {
    // AC_ASSERT(ulsb + W2 <= W, "Out of bounds set_slc");
    if (ulsb + W2 <= W) {
        Base::set_slc(ulsb, W2, (ac_int<W2,false>) slc);
    } else {
        Base r = 0;
        Base::operator=(r);
    }
    return *this;
  }

  class ac_bitref {
    ac_int &d_bv;
    unsigned d_index;
  public:
    ac_bitref( ac_int *bv, unsigned index=0 ) : d_bv(*bv), d_index(index) {
    }
    operator bool () const {
      return (d_index < W) ? (d_bv.value>>(d_index) & 1) : 0;
    }

    template<int W2, bool S2>
    operator ac_int<W2,S2> () const { return operator bool (); }

    inline ac_bitref operator = ( int val ) {
      // lsb of int (val&1) is written to bit
      if(d_index < W) {
        ap_int<W+1> temp1 = d_bv.value;
        ap_int<W+1> temp2 = val;
        temp2 <<= d_index;
        temp1 ^= temp2;
        temp2 = 1;
        temp2 <<= d_index;
        temp1 &= temp2;
        d_bv.value ^= temp1;
      }
      return *this;
    }
    template<int W2, bool S2>
    inline ac_bitref operator = ( const ac_int<W2,S2> &val ) {
      return operator =(val.to_int());
    }
    inline ac_bitref operator = ( const ac_bitref &val ) {
      return operator =((int) (bool) val);
    }
  };

  ac_bitref operator [] ( unsigned int uindex) {
    // AC_ASSERT(uindex < W, "Attempting to read bit beyond MSB");
    ac_bitref bvh( this, uindex );
    return bvh;
  }
  ac_bitref operator [] ( int index) {
    // AC_ASSERT(index >= 0, "Attempting to read bit with negative index");
    // AC_ASSERT(index < W, "Attempting to read bit beyond MSB");
    unsigned uindex = index & ((unsigned)~0 >> 1);
    ac_bitref bvh( this, uindex );
    return bvh;
  }
  template<int W2, bool S2>
  ac_bitref operator [] ( const ac_int<W2,S2> &index) {
    // AC_ASSERT(index >= 0, "Attempting to read bit with negative index");
    // AC_ASSERT(index < W, "Attempting to read bit beyond MSB");
    ac_int<W2-S2,false> uindex = index;
    ac_bitref bvh( this, uindex.to_uint() );
    return bvh;
  }
  bool operator [] ( unsigned int uindex) const {
    // AC_ASSERT(uindex < W, "Attempting to read bit beyond MSB");
    return (uindex < W) ? (Base::v[uindex>>5]>>(uindex&31) & 1) : 0;
  }
  bool operator [] ( int index) const {
    // AC_ASSERT(index >= 0, "Attempting to read bit with negative index");
    // AC_ASSERT(index < W, "Attempting to read bit beyond MSB");
    unsigned uindex = index & ((unsigned)~0 >> 1);
    return (uindex < W) ? (Base::v[uindex>>5]>>(uindex&31) & 1) : 0;
  }
  template<int W2, bool S2>
  bool operator [] ( const ac_int<W2,S2> &index) const {
    // AC_ASSERT(index >= 0, "Attempting to read bit with negative index");
    // AC_ASSERT(index < W, "Attempting to read bit beyond MSB");
    ac_int<W2-S2,false> uindex = index;
    return (uindex < W) ? (Base::v[uindex>>5]>>(uindex.to_uint()&31) & 1) : 0;
  }
  typename rt_unary::leading_sign leading_sign() const {
    unsigned ls = 0;
    return ls;
  }
  typename rt_unary::leading_sign leading_sign(bool &all_sign) const {
    unsigned ls = 0;
    return ls;
  }
  // returns false if number is denormal
  template<int WE, bool SE>
  bool normalize(ac_int<WE,SE> &exp) {
    return false;
  }
  // returns false if number is denormal, minimum exponent is reserved (usually for encoding special values/errors)
  template<int WE, bool SE>
  bool normalize_RME(ac_int<WE,SE> &exp) {
    return false;
  }
  bool and_reduce() const {
    return false;
  }
  bool or_reduce() const {
    return !Base::equal_zero();
  }
  bool xor_reduce() const {
    return false;
  }
  ac_int reverse() const {
    ac_int r = 0;
    Base::reverse(r);
    return r;
  }

  inline void bit_fill_hex(const char *str) {
    // Zero Pads if str is too short, throws ms bits away if str is too long
    // Asserts if anything other than 0-9a-fA-F is encountered
    ac_int<W,S> res = 0;
    int i = 0;
    while(str[i]) {
      char c = str[i];
      ac_int<4, false> h = 0;
      if(c >= '0' && c <= '9')
        h = c - '0';
      else if(c >= 'A' && c <= 'F')
        h = c - 'A' + 10;
      else if(c >= 'a' && c <= 'f')
        h = c - 'a' + 10;
      else {
        // AC_ASSERT(!c, "Invalid hex digit");
        break;
      }
      ac_int<4, false> s = 4;
      res =  res << s;
      res |= h;
      i++;
    }
    *this = res;
  }

  template<int Na>
  inline void bit_fill(const int (&ivec)[Na], bool bigendian=true) {
    // bit_fill from integer vector
    //   if W > N*32, missing most significant bits are zeroed
    //   if W < N*32, additional bits in ivec are ignored (no overflow checking)
    // Example:
    //   ac_int<80,false> x;    int vec[] = { 0xffffa987, 0x6543210f, 0xedcba987 };
    //   x.bit_fill(vec);   // vec[0] fill bits 79-64
    const int M = AC_MIN((W + 31)/32,Na);
    ac_int<M*32, false> res = 0;
    // Do not unroll this loop, the HLS flow
    // auto unrolls for us.
    for(int i=0; i < M; i++){
      res.set_slc(i*32, ac_int<32, false>(ivec[bigendian ? M-1-i : i]));
    }
    *this = res;
  }
};

namespace ac {
  template<typename T, typename T2>
  struct rt_2T {
    typedef typename ac_private::map<T>::t map_T;
    typedef typename ac_private::map<T2>::t map_T2;
    typedef typename map_T::template rt_T< map_T2 >::mult mult;
    typedef typename map_T::template rt_T< map_T2 >::plus plus;
    typedef typename map_T::template rt_T< map_T2 >::minus minus;
    typedef typename map_T::template rt_T< map_T2 >::minus2 minus2;
    typedef typename map_T::template rt_T< map_T2 >::logic logic;
    typedef typename map_T::template rt_T< map_T2 >::div div;
    typedef typename map_T::template rt_T< map_T2 >::div2 div2;
  };
}

namespace ac {
  template<typename T>
  struct ac_int_represent {
    enum { t_w = ac_private::c_type_params<T>::W, t_s = ac_private::c_type_params<T>::S };
    typedef ac_int<t_w,t_s> type;
  };
  template<> struct ac_int_represent<float> {};
  template<> struct ac_int_represent<double> {};
  template<int W, bool S>
  struct ac_int_represent< ac_int<W,S> > {
    typedef ac_int<W,S> type;
  };
}

namespace ac_private {
  template<int W2, bool S2>
  struct rt_ac_int_T< ac_int<W2,S2> > {
    typedef ac_int<W2,S2> i2_t;
    template<int W, bool S>
    struct op1 {
      typedef ac_int<W,S> i_t;
      typedef typename i_t::template rt<W2,S2>::mult mult;
      typedef typename i_t::template rt<W2,S2>::plus plus;
      typedef typename i_t::template rt<W2,S2>::minus minus;
      typedef typename i2_t::template rt<W,S>::minus minus2;
      typedef typename i_t::template rt<W2,S2>::logic logic;
      typedef typename i_t::template rt<W2,S2>::div div;
      typedef typename i2_t::template rt<W,S>::div div2;
      typedef typename i_t::template rt<W2,S2>::mod mod;
      typedef typename i2_t::template rt<W,S>::mod mod2;
    };
  };

  template<typename T>
  struct rt_ac_int_T< c_type<T> > {
    typedef typename ac::ac_int_represent<T>::type i2_t;
    enum { W2 = i2_t::width, S2 = i2_t::sign };
    template<int W, bool S>
    struct op1 {
      typedef ac_int<W,S> i_t;
      typedef typename i_t::template rt<W2,S2>::mult mult;
      typedef typename i_t::template rt<W2,S2>::plus plus;
      typedef typename i_t::template rt<W2,S2>::minus minus;
      typedef typename i2_t::template rt<W,S>::minus minus2;
      typedef typename i_t::template rt<W2,S2>::logic logic;
      typedef typename i_t::template rt<W2,S2>::div div;
      typedef typename i2_t::template rt<W,S>::div div2;
      typedef typename i_t::template rt<W2,S2>::mod mod;
      typedef typename i2_t::template rt<W,S>::mod mod2;
    };
  };
}

// Stream --------------------------------------------------------------------
// #if defined (__linux__)
// template<int W, bool S>
// inline std::ostream& operator << (std::ostream &os, const ac_int<W,S> &x) {
// #ifdef HLS_X86
//   os << x.to_string(AC_DEC);
// #endif
//   return os;
// }
// #endif // linux

// Macros for Binary Operators with Integers --------------------------------------------

#define BIN_OP_WITH_INT(BIN_OP, C_TYPE, WI, SI, RTYPE)  \
  template<int W, bool S> \
  inline typename ac_int<WI,SI>::template rt<W,S>::RTYPE operator BIN_OP ( C_TYPE i_op, const ac_int<W,S> &op) {  \
    return ac_int<WI,SI>(i_op).operator BIN_OP (op);  \
  } \
  template<int W, bool S>   \
  inline typename ac_int<W,S>::template rt<WI,SI>::RTYPE operator BIN_OP ( const ac_int<W,S> &op, C_TYPE i_op) {  \
    return op.operator BIN_OP (ac_int<WI,SI>(i_op));  \
  }

#define REL_OP_WITH_INT(REL_OP, C_TYPE, W2, S2)  \
  template<int W, bool S>   \
  inline bool operator REL_OP ( const ac_int<W,S> &op, C_TYPE op2) {  \
    return op.operator REL_OP (ac_int<W2,S2>(op2));  \
  }  \
  template<int W, bool S> \
  inline bool operator REL_OP ( C_TYPE op2, const ac_int<W,S> &op) {  \
    return ac_int<W2,S2>(op2).operator REL_OP (op);  \
  }

#define ASSIGN_OP_WITH_INT(ASSIGN_OP, C_TYPE, W2, S2)  \
  template<int W, bool S>   \
  inline ac_int<W,S> &operator ASSIGN_OP ( ac_int<W,S> &op, C_TYPE op2) {  \
    return op.operator ASSIGN_OP (ac_int<W2,S2>(op2));  \
  }

#define OPS_WITH_INT(C_TYPE, WI, SI) \
  BIN_OP_WITH_INT(*, C_TYPE, WI, SI, mult) \
  BIN_OP_WITH_INT(+, C_TYPE, WI, SI, plus) \
  BIN_OP_WITH_INT(-, C_TYPE, WI, SI, minus) \
  BIN_OP_WITH_INT(/, C_TYPE, WI, SI, div) \
  BIN_OP_WITH_INT(%, C_TYPE, WI, SI, mod) \
  BIN_OP_WITH_INT(>>, C_TYPE, WI, SI, arg1) \
  BIN_OP_WITH_INT(<<, C_TYPE, WI, SI, arg1) \
  BIN_OP_WITH_INT(&, C_TYPE, WI, SI, logic) \
  BIN_OP_WITH_INT(|, C_TYPE, WI, SI, logic) \
  BIN_OP_WITH_INT(^, C_TYPE, WI, SI, logic) \
  \
  REL_OP_WITH_INT(==, C_TYPE, WI, SI) \
  REL_OP_WITH_INT(!=, C_TYPE, WI, SI) \
  REL_OP_WITH_INT(>, C_TYPE, WI, SI) \
  REL_OP_WITH_INT(>=, C_TYPE, WI, SI) \
  REL_OP_WITH_INT(<, C_TYPE, WI, SI) \
  REL_OP_WITH_INT(<=, C_TYPE, WI, SI) \
  \
  ASSIGN_OP_WITH_INT(+=, C_TYPE, WI, SI) \
  ASSIGN_OP_WITH_INT(-=, C_TYPE, WI, SI) \
  ASSIGN_OP_WITH_INT(*=, C_TYPE, WI, SI) \
  ASSIGN_OP_WITH_INT(/=, C_TYPE, WI, SI) \
  ASSIGN_OP_WITH_INT(%=, C_TYPE, WI, SI) \
  ASSIGN_OP_WITH_INT(>>=, C_TYPE, WI, SI) \
  ASSIGN_OP_WITH_INT(<<=, C_TYPE, WI, SI) \
  ASSIGN_OP_WITH_INT(&=, C_TYPE, WI, SI) \
  ASSIGN_OP_WITH_INT(|=, C_TYPE, WI, SI) \
  ASSIGN_OP_WITH_INT(^=, C_TYPE, WI, SI)

// ------------------------------------- End of Macros for Binary Operators with Integers

namespace ac {
  namespace ops_with_other_types {
    //  Mixed Operators with Integers  -----------------------------------------------
    OPS_WITH_INT(bool, 1, false)
    OPS_WITH_INT(char, 8, true)
    OPS_WITH_INT(signed char, 8, true)
    OPS_WITH_INT(unsigned char, 8, false)
    OPS_WITH_INT(short, 16, true)
    OPS_WITH_INT(unsigned short, 16, false)
    OPS_WITH_INT(int, 32, true)
    OPS_WITH_INT(unsigned int, 32, false)
    // OPS_WITH_INT(long, ac_private::long_w, true)
    // OPS_WITH_INT(unsigned long, ac_private::long_w, false)
    OPS_WITH_INT(Slong, 64, true)
    OPS_WITH_INT(Ulong, 64, false)
    // -----------------------------------------  End of Mixed Operators with Integers
  }  // ops_with_other_types namespace

  // Functions to fill bits

  template<typename T>
  inline T bit_fill_hex(const char *str) {
    T res;
    res.bit_fill_hex(str);
    return res;
  }

  // returns bit_fill for type
  //   example:
  //   ac_int<80,false> x = ac::bit_fill< ac_int<80,false> > ((int [3]) {0xffffa987, 0x6543210f, 0xedcba987 });
  template<typename T, int N>
  inline T bit_fill(const int (&ivec)[N], bool bigendian=true) {
    T res;
    res.bit_fill(ivec, bigendian);
    return res;
  }

}  // ac namespace

//  Mixed Operators with Pointers  -----------------------------------------------

// Addition of ac_int and  pointer
template<typename T, int W, bool S>
T *operator +(T *ptr, const ac_int<W,S> &op2) {
  return ptr + op2.to_int64();
}
template<typename T, int W, bool S>
T *operator +(const ac_int<W,S> &op2, T *ptr) {
  return ptr + op2.to_int64();
}
// Subtraction of ac_int from pointer
template<typename T, int W, bool S>
T *operator -(T *ptr, const ac_int<W,S> &op2) {
  return ptr - op2.to_int64();
}
// -----------------------------------------  End of Mixed Operators with Pointers

using namespace ac::ops_with_other_types;

namespace ac_intN {
  ///////////////////////////////////////////////////////////////////////////////
  //  Predefined for ease of use
  ///////////////////////////////////////////////////////////////////////////////
  typedef ac_int<1,          true>   int1;
  typedef ac_int<1,          false>  uint1;
  typedef ac_int<2,          true>   int2;
  typedef ac_int<2,          false>  uint2;
  typedef ac_int<3,          true>   int3;
  typedef ac_int<3,          false>  uint3;
  typedef ac_int<4,          true>   int4;
  typedef ac_int<4,          false>  uint4;
  typedef ac_int<5,          true>   int5;
  typedef ac_int<5,          false>  uint5;
  typedef ac_int<6,          true>   int6;
  typedef ac_int<6,          false>  uint6;
  typedef ac_int<7,          true>   int7;
  typedef ac_int<7,          false>  uint7;
  typedef ac_int<8,          true>   int8;
  typedef ac_int<8,          false>  uint8;
  typedef ac_int<9,          true>   int9;
  typedef ac_int<9,          false>  uint9;
  typedef ac_int<10,         true>   int10;
  typedef ac_int<10,         false>  uint10;
  typedef ac_int<11,         true>   int11;
  typedef ac_int<11,         false>  uint11;
  typedef ac_int<12,         true>   int12;
  typedef ac_int<12,         false>  uint12;
  typedef ac_int<13,         true>   int13;
  typedef ac_int<13,         false>  uint13;
  typedef ac_int<14,         true>   int14;
  typedef ac_int<14,         false>  uint14;
  typedef ac_int<15,         true>   int15;
  typedef ac_int<15,         false>  uint15;
  typedef ac_int<16,         true>   int16;
  typedef ac_int<16,         false>  uint16;
  typedef ac_int<17,         true>   int17;
  typedef ac_int<17,         false>  uint17;
  typedef ac_int<18,         true>   int18;
  typedef ac_int<18,         false>  uint18;
  typedef ac_int<19,         true>   int19;
  typedef ac_int<19,         false>  uint19;
  typedef ac_int<20,         true>   int20;
  typedef ac_int<20,         false>  uint20;
  typedef ac_int<21,         true>   int21;
  typedef ac_int<21,         false>  uint21;
  typedef ac_int<22,         true>   int22;
  typedef ac_int<22,         false>  uint22;
  typedef ac_int<23,         true>   int23;
  typedef ac_int<23,         false>  uint23;
  typedef ac_int<24,         true>   int24;
  typedef ac_int<24,         false>  uint24;
  typedef ac_int<25,         true>   int25;
  typedef ac_int<25,         false>  uint25;
  typedef ac_int<26,         true>   int26;
  typedef ac_int<26,         false>  uint26;
  typedef ac_int<27,         true>   int27;
  typedef ac_int<27,         false>  uint27;
  typedef ac_int<28,         true>   int28;
  typedef ac_int<28,         false>  uint28;
  typedef ac_int<29,         true>   int29;
  typedef ac_int<29,         false>  uint29;
  typedef ac_int<30,         true>   int30;
  typedef ac_int<30,         false>  uint30;
  typedef ac_int<31,         true>   int31;
  typedef ac_int<31,         false>  uint31;
  typedef ac_int<32,         true>   int32;
  typedef ac_int<32,         false>  uint32;
  typedef ac_int<33,         true>   int33;
  typedef ac_int<33,         false>  uint33;
  typedef ac_int<34,         true>   int34;
  typedef ac_int<34,         false>  uint34;
  typedef ac_int<35,         true>   int35;
  typedef ac_int<35,         false>  uint35;
  typedef ac_int<36,         true>   int36;
  typedef ac_int<36,         false>  uint36;
  typedef ac_int<37,         true>   int37;
  typedef ac_int<37,         false>  uint37;
  typedef ac_int<38,         true>   int38;
  typedef ac_int<38,         false>  uint38;
  typedef ac_int<39,         true>   int39;
  typedef ac_int<39,         false>  uint39;
  typedef ac_int<40,         true>   int40;
  typedef ac_int<40,         false>  uint40;
  typedef ac_int<41,         true>   int41;
  typedef ac_int<41,         false>  uint41;
  typedef ac_int<42,         true>   int42;
  typedef ac_int<42,         false>  uint42;
  typedef ac_int<43,         true>   int43;
  typedef ac_int<43,         false>  uint43;
  typedef ac_int<44,         true>   int44;
  typedef ac_int<44,         false>  uint44;
  typedef ac_int<45,         true>   int45;
  typedef ac_int<45,         false>  uint45;
  typedef ac_int<46,         true>   int46;
  typedef ac_int<46,         false>  uint46;
  typedef ac_int<47,         true>   int47;
  typedef ac_int<47,         false>  uint47;
  typedef ac_int<48,         true>   int48;
  typedef ac_int<48,         false>  uint48;
  typedef ac_int<49,         true>   int49;
  typedef ac_int<49,         false>  uint49;
  typedef ac_int<50,         true>   int50;
  typedef ac_int<50,         false>  uint50;
  typedef ac_int<51,         true>   int51;
  typedef ac_int<51,         false>  uint51;
  typedef ac_int<52,         true>   int52;
  typedef ac_int<52,         false>  uint52;
  typedef ac_int<53,         true>   int53;
  typedef ac_int<53,         false>  uint53;
  typedef ac_int<54,         true>   int54;
  typedef ac_int<54,         false>  uint54;
  typedef ac_int<55,         true>   int55;
  typedef ac_int<55,         false>  uint55;
  typedef ac_int<56,         true>   int56;
  typedef ac_int<56,         false>  uint56;
  typedef ac_int<57,         true>   int57;
  typedef ac_int<57,         false>  uint57;
  typedef ac_int<58,         true>   int58;
  typedef ac_int<58,         false>  uint58;
  typedef ac_int<59,         true>   int59;
  typedef ac_int<59,         false>  uint59;
  typedef ac_int<60,         true>   int60;
  typedef ac_int<60,         false>  uint60;
  typedef ac_int<61,         true>   int61;
  typedef ac_int<61,         false>  uint61;
  typedef ac_int<62,         true>   int62;
  typedef ac_int<62,         false>  uint62;
  typedef ac_int<63,         true>   int63;
  typedef ac_int<63,         false>  uint63;
}  // namespace ac_intN

#ifndef AC_NOT_USING_INTN
using namespace ac_intN;
#endif

///////////////////////////////////////////////////////////////////////////////

// Global templatized functions for easy initialization to special values
template<ac_special_val V, int W, bool S>
inline ac_int<W,S> value(ac_int<W,S>) {
  ac_int<W,S> r = 0;
  return r.template set_val<V>();
}
// forward declaration, otherwise GCC errors when calling init_array
template<ac_special_val V, int W, int I, bool S, ac_q_mode Q, ac_o_mode O>
inline ac_fixed<W,I,S,Q,O> value(ac_fixed<W,I,S,Q,O>);

#define SPECIAL_VAL_FOR_INTS_DC(C_TYPE, WI, SI) \
template<> inline C_TYPE value<AC_VAL_DC>(C_TYPE) { C_TYPE x=0; return x; }

// -- C int types -----------------------------------------------------------------
#define SPECIAL_VAL_FOR_INTS(C_TYPE, WI, SI) \
template<ac_special_val val> inline C_TYPE value(C_TYPE); \
template<> inline C_TYPE value<AC_VAL_0>(C_TYPE) { return (C_TYPE)0; } \
SPECIAL_VAL_FOR_INTS_DC(C_TYPE, WI, SI) \
template<> inline C_TYPE value<AC_VAL_QUANTUM>(C_TYPE) { return (C_TYPE)1; } \
template<> inline C_TYPE value<AC_VAL_MAX>(C_TYPE) { return (C_TYPE)(SI ? ~((C_TYPE) -1 << (WI-1)) : (C_TYPE) -1); } \
template<> inline C_TYPE value<AC_VAL_MIN>(C_TYPE) { return (C_TYPE)(SI ? (C_TYPE) 1 << (WI-1) : 0); }

SPECIAL_VAL_FOR_INTS(bool, 1, false)
SPECIAL_VAL_FOR_INTS(char, 8, true)
SPECIAL_VAL_FOR_INTS(signed char, 8, true)
SPECIAL_VAL_FOR_INTS(unsigned char, 8, false)
SPECIAL_VAL_FOR_INTS(short, 16, true)
SPECIAL_VAL_FOR_INTS(unsigned short, 16, false)
SPECIAL_VAL_FOR_INTS(int, 32, true)
SPECIAL_VAL_FOR_INTS(unsigned int, 32, false)
// SPECIAL_VAL_FOR_INTS(long, ac_private::long_w, true)
// SPECIAL_VAL_FOR_INTS(unsigned long, ac_private::long_w, false)
SPECIAL_VAL_FOR_INTS(Slong, 64, true)
SPECIAL_VAL_FOR_INTS(Ulong, 64, false)

#define INIT_ARRAY_SPECIAL_VAL_FOR_INTS(C_TYPE) \
  template<ac_special_val V> \
  inline bool init_array(C_TYPE *a, int n) { \
    C_TYPE t = value<V>(*a); \
    for(int i=0; i < n; i++) \
      a[i] = t; \
    return true; \
  }

namespace ac {
// PUBLIC FUNCTIONS
// function to initialize (or uninitialize) arrays
  template<ac_special_val V, int W, bool S>
  inline bool init_array(ac_int<W,S> *a, int n) {
    ac_int<W,S> t = value<V>(*a);
    for(int i=0; i < n; i++)
      a[i] = t;
    return true;
  }

  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(bool)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(char)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(signed char)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(unsigned char)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(signed short)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(unsigned short)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(signed int)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(unsigned int)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(signed long)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(unsigned long)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(signed long long)
  INIT_ARRAY_SPECIAL_VAL_FOR_INTS(unsigned long long)
}

#ifdef __AC_NAMESPACE
}
#endif

#endif // __ALTR_AC_INT_H

