// RUN: %clang_cc1 -fsycl-is-device -triple spir64-unknown-unknown-sycldevice -aux-triple x86_64-linux-pc -verify -fsyntax-only %s
// Eventually this should diagnose if we ever try to emit the builtin on
// the SYCL device, but that requires quite a lot of upstream work.
// expected-no-diagnostics

using VecTy = long __attribute__((vector_size(64)));

struct Converts {
  Converts(unsigned short);
  operator VecTy();
};

Converts func() {
  VecTy b;
  return __builtin_ia32_cmpd512_mask(func(), b, 0, 1);
}

