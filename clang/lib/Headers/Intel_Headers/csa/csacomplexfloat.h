/*===---- csacomplexfloat.hpp - CSA complex float class-------------------===
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *===-----------------------------------------------------------------------===
 */

#ifndef __CSACOMPLEXFLOAT_H
#define __CSACOMPLEXFLOAT_H

#pragma once

#include <stdint.h>
#include <csa/csaintrin.h>
#include <cmath>
#ifndef __CSA__
#include <iostream>
#endif
// Utility function to convert a float to a __m64f, leaving the top part undefined.
// This is a temporary work-around for a compiler issue.
static __m64f _mm64_castss_ps (float value)
{
  union
  {
    __m64f v;
    float f;
  };
  f = value;
  return v;
}

// Top level class to be used in programs
// with complex arithmetic
class ComplexFloat
{
public:

  // Convention is to initialise complex values with zero.
  constexpr ComplexFloat() : m_value() { }

  constexpr ComplexFloat(float r, float i) : m_value((__m64f){r, i}) { }

  constexpr ComplexFloat(float value) : m_value((__m64f){value, 0}) { }

  ComplexFloat(__m64f v) : m_value(v) { }

  /// Allow implicit conversion to the underlying data type, to make it easy to use
  /// ComplexFloat types with intrinsics.
  operator  __m64f() const { return m_value; }

  float real() const { return m_value[0]; }
  float imag() const { return m_value[1]; }

  friend float real(ComplexFloat cf) { return cf.real(); }
  friend float imag(ComplexFloat cf) { return cf.imag(); }

  friend float creal(ComplexFloat cf) { return cf.real(); }
  friend float cimag(ComplexFloat cf) { return cf.imag(); }

  void real(float r) { m_value[0] = r; }
  void imag(float i) { m_value[1] = i; }

  ComplexFloat getConj() const {
    // Flip the imaginary sign bit.
    union {
      __m64f cmplx;
      uint64_t i;
    };
    cmplx = m_value;
    i ^= 0x8000000000000000;
    return cmplx;
  }

  friend ComplexFloat conj(ComplexFloat value) { return value.getConj(); }

#ifdef __CSA__
  friend ComplexFloat round(ComplexFloat value)
  {
    // Calling roundf or std::round is an expensive function call. Call the asm directly.
    ComplexFloat cf;
    asm("roundf32 %0, %1" : "=d"(cf.m_value[0]) : "d"(value.m_value[0]));
    asm("roundf32 %0, %1" : "=d"(cf.m_value[1]) : "d"(value.m_value[1]));
    return cf;
  }
#else
friend ComplexFloat round(ComplexFloat value)
  {
    ComplexFloat cf;
    cf.m_value[0] = std::round(value.m_value[0]);
    cf.m_value[1] = std::round(value.m_value[1]);
    return cf;
  }
#endif

  // Given a + bi compute a * a + b * b;
  friend float norm(ComplexFloat value) {
    // Compute real^2 and imag^2, storing in the two elements.
    const auto sqr = _mm64_mul_ps(value.m_value, value.m_value,
                                  _MM_DISABLE_NONE, _MM_SWIZZLE_NONE,
                                  _MM_SWIZZLE_NONE);

    // Add together to get the norm.
    const auto norm =
       _mm64_add_ps(sqr, sqr, _MM_DISABLE_NONE,
                    _MM_SWIZZLE_INTERCHANGE, _MM_SWIZZLE_NONE);

    return norm[0];
  }

  // Given (a + bi) compute conj(a+bi)/(aa+bb)
#ifdef __CSA__
  friend ComplexFloat rcp(ComplexFloat value) {
    // Compute r * r and i * i
    const auto sqr = _mm64_mul_ps(value.m_value, value.m_value,
                                  _MM_DISABLE_NONE, _MM_SWIZZLE_NONE,
                                  _MM_SWIZZLE_NONE);

    // Add together to get the norm.
    const auto norm =
       _mm64_add_ps(sqr, sqr, _MM_DISABLE_NONE,
                    _MM_SWIZZLE_INTERCHANGE, _MM_SWIZZLE_NONE);

    // Use a little inline assembly to get at the appropriate rcp instruction.
    float rcpNorm;
    asm("rcp14f32 %0, %1" : "=d"(rcpNorm) : "c"(norm));

    ComplexFloat cj = value.getConj();

    // Multiply the conjugate by the rcp.
    return _mm64_mul_ps(cj.m_value, _mm64_pack_ps(rcpNorm, 0.0f),
                        _MM_DISABLE_NONE, _MM_SWIZZLE_NONE,
                        _MM_SWIZZLE_BCAST_LOW);
  }
#else
  friend ComplexFloat rcp(ComplexFloat value) {
    ComplexFloat cf;
    auto a = value.m_value[0];
    auto b = value.m_value[1];
    cf.m_value[0] = a/(a*a + b*b);
    cf.m_value[1] = -b/(a*a + b*b);
    return cf;
  }
#endif

  /// Perform a fused-multiply-add, equivalent to a * b + c.
  // :TODO: Ideally the compiler would have a peephole enabling it to do this itself, but until that
  // is added use an explicit method instead.
  friend ComplexFloat fma(ComplexFloat a, ComplexFloat b, ComplexFloat c)
  {
    // return a * b + c;
    const auto t = _mm64_fma_ps(a.m_value, b.m_value, c.m_value, _MM_DISABLE_NONE,
                                 _MM_SWIZZLE_BCAST_HIGH, _MM_SWIZZLE_INTERCHANGE);
    return _mm64_fmas_ps(a.m_value, b.m_value, t, _MM_DISABLE_NONE,
                         _MM_SWIZZLE_BCAST_LOW, _MM_SWIZZLE_NONE);
  }

  /// Perform a fused-multiply-, equivalent to c - a * b .
  // :TODO: Ideally the compiler would have a peephole enabling it to do this itself, but until that
  // is added use an explicit method instead.
  inline friend ComplexFloat fmrs(ComplexFloat a, ComplexFloat b, ComplexFloat c)
  {
    // return c - a * b ;
    // handles - a.imag * b.real(which is negative, hence fmsa) and  + a.imag * b.imag
    const auto t = _mm64_fmsa_ps(a.m_value, b.m_value, c.m_value, _MM_DISABLE_NONE,
                                 _MM_SWIZZLE_BCAST_HIGH, _MM_SWIZZLE_INTERCHANGE);
    return _mm64_fms_ps(a.m_value, b.m_value, t, _MM_DISABLE_NONE,
                         _MM_SWIZZLE_BCAST_LOW, _MM_SWIZZLE_NONE);
  }

  // Given (a + bi) * (c + di):
  // Compute b * (d, c) => bd, bc
  // Compute a * (c, d) => ac, ad
  // Add/sub together =>  ac - bd, ad + bc
  friend ComplexFloat operator*(ComplexFloat lhs, ComplexFloat rhs) {
    const auto t = _mm64_mul_ps(lhs.m_value, rhs.m_value, _MM_DISABLE_NONE,
                                _MM_SWIZZLE_BCAST_HIGH, _MM_SWIZZLE_INTERCHANGE);
    return _mm64_fmas_ps(lhs.m_value, rhs.m_value, t,
                         _MM_DISABLE_NONE, _MM_SWIZZLE_BCAST_LOW,
                         _MM_SWIZZLE_NONE);
  }
  ComplexFloat& operator *= (ComplexFloat rhs) { *this = *this * rhs; return *this; }

  // Compute (a + bi) * conj(c + di)
  friend ComplexFloat mulByConj(ComplexFloat lhs, ComplexFloat rhs) {
    const auto t = _mm64_mul_ps(lhs.m_value, rhs.m_value, _MM_DISABLE_NONE,
                                _MM_SWIZZLE_BCAST_LOW, _MM_SWIZZLE_NONE);
    return  _mm64_fmsa_ps(lhs.m_value, rhs.m_value, t,
                          _MM_DISABLE_NONE, _MM_SWIZZLE_BCAST_HIGH,
                          _MM_SWIZZLE_INTERCHANGE);
  }


  friend ComplexFloat operator*(ComplexFloat lhs, float rhs) {
    return _mm64_mul_ps(lhs.m_value, _mm64_pack_ps(rhs, 0.0f),
                        _MM_DISABLE_NONE, _MM_SWIZZLE_NONE,
                        _MM_SWIZZLE_BCAST_LOW);
  }
  friend ComplexFloat operator*(float lhs, ComplexFloat rhs) { return rhs * lhs; }
  ComplexFloat& operator *= (float rhs) { *this = *this * rhs; return *this; }


  friend ComplexFloat operator+(ComplexFloat lhs, ComplexFloat rhs) {
    return _mm64_add_ps(lhs.m_value, rhs.m_value, _MM_DISABLE_NONE,
                        _MM_SWIZZLE_NONE, _MM_SWIZZLE_NONE);
  }
  ComplexFloat& operator += (ComplexFloat rhs) { *this = *this + rhs; return *this; }

  friend ComplexFloat operator+(ComplexFloat lhs, float rhs) {
    return _mm64_add_ps(lhs.m_value, _mm64_pack_ps(rhs, 0.0f),
                        _MM_DISABLE_HIGH, _MM_SWIZZLE_NONE, _MM_SWIZZLE_NONE);
  }
  friend ComplexFloat operator+(float lhs, ComplexFloat rhs) { return rhs + lhs; }
  ComplexFloat& operator += (float rhs) { *this = *this + rhs; return *this; }


  friend ComplexFloat operator-(ComplexFloat lhs, ComplexFloat rhs) {
    return _mm64_sub_ps(lhs.m_value, rhs.m_value, _MM_DISABLE_NONE,
                        _MM_SWIZZLE_NONE, _MM_SWIZZLE_NONE);
  }
  ComplexFloat& operator -= (ComplexFloat rhs) { *this = *this - rhs; return *this; }

  friend bool operator == (ComplexFloat lhs, ComplexFloat rhs) { return (lhs.real() == rhs.real()) && (lhs.imag() == rhs.imag()); }
  friend bool operator != (ComplexFloat lhs, ComplexFloat rhs) { return !(lhs == rhs); }

  friend ComplexFloat operator-(ComplexFloat lhs, float rhs) {
    return _mm64_sub_ps(lhs.m_value, _mm64_pack_ps(rhs, 0.0f),
                        _MM_DISABLE_NONE, _MM_SWIZZLE_NONE,
                        _MM_SWIZZLE_BCAST_LOW);
  }
  ComplexFloat& operator -= (float rhs) { *this = *this - rhs; return *this; }
#ifndef __CSA__
  friend std::ostream& operator<<(std::ostream& os, const ComplexFloat& rhs) {

    os << rhs.real() << " + " << rhs.imag() << "j";

    return os;
  }
#endif

  friend ComplexFloat operator-(ComplexFloat value) {
    // Flip the sign bit.
    union {
      __m64f cmplx;
      uint64_t i;
    };
    cmplx = value.m_value;
    i ^= 0x8000000080000000;

    return cmplx;
  }

  friend float cabsf(ComplexFloat rhs) {
    return hypot(rhs.real(), rhs.imag());
  }

  __m64f m_value;

};
#endif /* __CSACOMPLEXFLOAT_H */
