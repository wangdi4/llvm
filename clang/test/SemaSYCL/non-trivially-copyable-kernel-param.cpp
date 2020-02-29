// RUN: %clang_cc1 -fsycl-is-device -verify -fsyntax-only %s

// This test checks if compiler reports compilation error on an attempt to pass
// a struct with non-trivially copyable type as SYCL kernel parameter.

struct A { int i; };

struct B {
  int i;
  B (int _i) : i(_i) {}
  B (const B& x) : i(x.i) {}
};

<<<<<<< HEAD
// if INTEL_CUSTOMIZATION
=======
>>>>>>> e0d64f538e3d2743f8dbd278f736aec57213f568
struct C : A {
  const A C2;
  C() : A{0}, C2{2}{}
};

struct D {
  int i;
  ~D();
};
<<<<<<< HEAD
// endif // INTEL_CUSTOMIZATION
=======
>>>>>>> e0d64f538e3d2743f8dbd278f736aec57213f568

template <typename Name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

void test() {
  A IamGood;
  IamGood.i = 0;
  B IamBad(1);
<<<<<<< HEAD
  C IamAlsoGood;  // INTEL
  D IamAlsoBad{0}; // INTEL
  kernel_single_task<class kernel_capture_refs>([=] {
    int a = IamGood.i;
// if INTEL_CUSTOMIZATION
=======
  C IamAlsoGood;
  D IamAlsoBad{0};
  kernel_single_task<class kernel_capture_refs>([=] {
    int a = IamGood.i;
>>>>>>> e0d64f538e3d2743f8dbd278f736aec57213f568
    // expected-error@+1 {{kernel parameter has non-trivially copy constructible class/struct type}}
    int b = IamBad.i;
    int c = IamAlsoGood.i;
    // expected-error@+1 {{kernel parameter has non-trivially destructible class/struct type}}
    int d = IamAlsoBad.i;
<<<<<<< HEAD
// endif // INTEL_CUSTOMIZATION
=======
>>>>>>> e0d64f538e3d2743f8dbd278f736aec57213f568
  });
}
