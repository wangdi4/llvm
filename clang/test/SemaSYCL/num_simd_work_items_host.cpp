// RUN: %clang_cc1 -fsycl -fsycl-is-host -fsyntax-only -Wno-sycl-2017-compat -verify %s
// expected-no-diagnostics

<<<<<<< HEAD
[[intel::num_simd_work_items(2)]] void func_do_not_ignore() {}

struct FuncObj {
  [[intel::num_simd_work_items(42)]] void operator()() const {}
=======
[[intelfpga::num_simd_work_items(2)]] void func_do_not_ignore() {}

struct FuncObj {
  [[intelfpga::num_simd_work_items(42)]] void operator()() const {}
>>>>>>> b6200fef86407e4d479880f63282fd7b8ed83df1
};
