//==----------- dot_product.hpp ------- SYCL dot-product --------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// DP4A extension

#pragma once

namespace cl {
namespace sycl {
namespace intel {

#ifdef __SYCL_DEVICE_ONLY__

typedef int8_t my_char4 __attribute__((ext_vector_type(4)));
typedef uint8_t my_uchar4 __attribute__((ext_vector_type(4)));

#if 0 // Enable when OpenCL implements these functions
int __attribute__((overloadable))
intel_dot_acc(my_char4 a, my_char4 b, int c);

int __attribute__((overloadable))
intel_dot_acc(my_char4 a, my_uchar4 b, int c);

int __attribute__((overloadable))
intel_dot_acc(my_uchar4 a, my_char4 b, int c);

int __attribute__((overloadable))
intel_dot_acc(my_uchar4 a, my_uchar4 b, int c);
#endif

extern "C" {
    SYCL_EXTERNAL int __builtin_IB_dp4a_ss(int c, int a, int b) __attribute__((const));
    SYCL_EXTERNAL int __builtin_IB_dp4a_uu(int c, int a, int b) __attribute__((const));
    SYCL_EXTERNAL int __builtin_IB_dp4a_su(int c, int a, int b) __attribute__((const));
    SYCL_EXTERNAL int __builtin_IB_dp4a_us(int c, int a, int b) __attribute__((const));
}

int dot_acc(int a, int b, int c) {
    //return intel_dot_acc(my_char4(a), my_char4(b), c);
    return __builtin_IB_dp4a_ss(c, a, b);
}

int dot_acc(int a, unsigned int b, int c) {
  //return intel_dot_acc(my_char4(a), my_uchar4(b), c);
  return __builtin_IB_dp4a_su(c, a, b);
}

int dot_acc(unsigned int a, int b, int c) {
  //return intel_dot_acc(my_uchar4(a), my_char4(b), c);
  return __builtin_IB_dp4a_us(c, a, b);
}

int dot_acc(unsigned int a, unsigned int b, int c) {
  //return intel_dot_acc(my_uchar4(a), my_uchar4(b), c);
  return __builtin_IB_dp4a_uu(c, a, b);
}

int32_t dot_acc(vec<int8_t, 4> a, vec<int8_t, 4> b, int32_t c) {
  //return intel_dot_acc(my_char4(a), my_char4(b), c);
  return __builtin_IB_dp4a_ss(c, *(reinterpret_cast<int*>(&a)), *(reinterpret_cast<int*>(&b)));
}

int32_t dot_acc(vec<int8_t, 4> a, vec<uint8_t, 4> b, int32_t c) {
  //return intel_dot_acc(my_char4(a), my_uchar4(b), c);
  return __builtin_IB_dp4a_su(c, *(reinterpret_cast<int*>(&a)), *(reinterpret_cast<int*>(&b)));
}

int32_t dot_acc(vec<uint8_t, 4> a, vec<int8_t, 4> b, int32_t c) {
  //return intel_dot_acc(my_uchar4(a), my_char4(b), c);
  return __builtin_IB_dp4a_us(c, *(reinterpret_cast<int*>(&a)), *(reinterpret_cast<int*>(&b)));
}

int32_t dot_acc(vec<uint8_t, 4> a, vec<uint8_t, 4> b, int32_t c) {
  //return intel_dot_acc(my_uchar4(a), my_uchar4(b), c);
  return __builtin_IB_dp4a_uu(c, *(reinterpret_cast<int*>(&a)), *(reinterpret_cast<int*>(&b)));
}

#else // SYCL_DEVICE_ONLY

int dot_acc(int pa, int pb, int c)
{
  vec<int8_t, 4> a = *(reinterpret_cast<vec<int8_t, 4>*>(&pa));
  vec<int8_t, 4> b = *(reinterpret_cast<vec<int8_t, 4>*>(&pb));
  return a.s0() * b.s0() +
    a.s1() * b.s1() +
    a.s2() * b.s2() +
    a.s3() * b.s3() +
    c;
}

int dot_acc(unsigned int pa, unsigned int pb, int c)
{
  vec<uint8_t, 4> a = *(reinterpret_cast<vec<uint8_t, 4>*>(&pa));
  vec<uint8_t, 4> b = *(reinterpret_cast<vec<uint8_t, 4>*>(&pb));
  return a.s0() * b.s0() +
    a.s1() * b.s1() +
    a.s2() * b.s2() +
    a.s3() * b.s3() +
    c;
}

int dot_acc(int pa, unsigned int pb, int c)
{
  vec<int8_t, 4>  a = *(reinterpret_cast<vec<int8_t, 4>*>(&pa));
  vec<uint8_t, 4> b = *(reinterpret_cast<vec<uint8_t, 4>*>(&pb));
  return a.s0() * b.s0() +
    a.s1() * b.s1() +
    a.s2() * b.s2() +
    a.s3() * b.s3() +
    c;
}

int dot_acc(unsigned int pa, int pb, int c)
{
  vec<uint8_t, 4> a = *(reinterpret_cast<vec<uint8_t, 4>*>(&pa));
  vec<int8_t, 4>  b = *(reinterpret_cast<vec<int8_t, 4>*>(&pb));
  return a.s0() * b.s0() +
    a.s1() * b.s1() +
    a.s2() * b.s2() +
    a.s3() * b.s3() +
    c;
}

int dot_acc(vec<int8_t, 4> a, vec<int8_t, 4> b, int32_t c) {
  return a.s0() * b.s0() +
    a.s1() * b.s1() +
    a.s2() * b.s2() +
    a.s3() * b.s3() +
    c;
}

int dot_acc(vec<uint8_t, 4> a, vec<uint8_t, 4> b, int32_t c) {
  return a.s0() * b.s0() +
    a.s1() * b.s1() +
    a.s2() * b.s2() +
    a.s3() * b.s3() +
    c;
}

int dot_acc(vec<uint8_t, 4> a, vec<int8_t, 4> b, int32_t c) {
  return a.s0() * b.s0() +
    a.s1() * b.s1() +
    a.s2() * b.s2() +
    a.s3() * b.s3() +
    c;
}

int dot_acc(vec<int8_t, 4> a, vec<uint8_t, 4> b, int32_t c) {
  return a.s0() * b.s0() +
    a.s1() * b.s1() +
    a.s2() * b.s2() +
    a.s3() * b.s3() +
    c;
}

#endif // SYCL_DEVICE_ONLY

} // namespace intel
} // namespace sycl
} // namespace cl
