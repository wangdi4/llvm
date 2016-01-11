// RUN: %clang_cc1 -fsyntax-only -verify -fintel-compatibility -gnu-permissive -triple x86_64-unknown-linux-gnu %s
// expected-no-diagnostics

int f1(void* p) {
  return reinterpret_cast<int>(p);
}

template<class T1, class T2>
T1 f2(T2 p) {
  return reinterpret_cast<T1>(p);
}

short f2_test(void* p) {
  return f2<short>(p);
}
