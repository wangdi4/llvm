//==--- fallback-intel-dot-product.cpp - device agnostic implementation of dot-product --==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "device.h"

#ifdef __SYCL_DEVICE_ONLY__

extern "C" {

union Us{
  char s[4];
  int i;
};
union Uu{
  unsigned char s[4];
  int i;
};

DEVICE_EXTERN_C
int __builtin_IB_dp4a_ss(int c, int pa, int pb)
{
  Us a = *(reinterpret_cast<Us*>(&pa));
  Us b = *(reinterpret_cast<Us*>(&pb));
  return a.s[0] * b.s[0] +
      a.s[1] * b.s[1] +
      a.s[2] * b.s[2] +
      a.s[3] * b.s[3] +
      c;
}

DEVICE_EXTERN_C
int __builtin_IB_dp4a_uu(int c, int pa, int pb)
{
  Uu a = *(reinterpret_cast<Uu*>(&pa));
  Uu b = *(reinterpret_cast<Uu*>(&pb));
  return a.s[0] * b.s[0] +
    a.s[1] * b.s[1] +
    a.s[2] * b.s[2] +
    a.s[3] * b.s[3] +
    c;
}

DEVICE_EXTERN_C
int __builtin_IB_dp4a_su(int c, int pa, int pb)
{
  Us a = *(reinterpret_cast<Us*>(&pa));
  Uu b = *(reinterpret_cast<Uu*>(&pb));
  return a.s[0] * b.s[0] +
    a.s[1] * b.s[1] +
    a.s[2] * b.s[2] +
    a.s[3] * b.s[3] +
    c;
}

DEVICE_EXTERN_C
int __builtin_IB_dp4a_us(int c, int pa, int pb)
{
  Uu a = *(reinterpret_cast<Uu*>(&pa));
  Us b = *(reinterpret_cast<Us*>(&pb));
  return a.s[0] * b.s[0] +
    a.s[1] * b.s[1] +
    a.s[2] * b.s[2] +
    a.s[3] * b.s[3] +
    c;
}

}

#endif // __SYCL_DEVICE_ONLY_

