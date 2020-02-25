// RUN: %clang_cc1 -fsycl-is-device -fsyntax-only -verify -pedantic %s

// Test for Intel FPGA loop attributes applied not to a loop
void foo() {
  // expected-error@+1 {{intelfpga loop attributes must be applied to for, while, or do statements}}
  [[intelfpga::ivdep]] int a[10];
  // expected-error@+1 {{intelfpga loop attributes must be applied to for, while, or do statements}}
  [[intelfpga::ivdep(2)]] int b[10];
// INTEL_CUSTOMIZATION
  // expected-error@+1 {{intelfpga loop attributes must be applied to for, while, or do statements}}
  [[intelfpga::ivdep_exp]] int a_exp[10];
  // expected-error@+1 {{intelfpga loop attributes must be applied to for, while, or do statements}}
  [[intelfpga::ivdep_exp(2)]] int b_exp[10];
// end INTEL_CUSTOMIZATION
  // expected-error@+1 {{intelfpga loop attributes must be applied to for, while, or do statements}}
  [[intelfpga::ii(2)]] int c[10];
  // expected-error@+1 {{intelfpga loop attributes must be applied to for, while, or do statements}}
  [[intelfpga::max_concurrency(2)]] int d[10];

  int arr[10];
  // expected-error@+1 {{intelfpga loop attributes must be applied to for, while, or do statements}}
  [[intelfpga::ivdep_exp(arr)]] int e[10]; // INTEL
  // expected-error@+1 {{intelfpga loop attributes must be applied to for, while, or do statements}}
  [[intelfpga::ivdep_exp(arr, 2)]] int f[10]; // INTEL
}

// Test for incorrect number of arguments for Intel FPGA loop attributes
void boo() {
  int a[10];
  int b[10];
// INTEL_CUSTOMIZATION
  // expected-warning@+1 {{'ivdep' attribute takes no more than 1 argument - attribute ignored}}
  [[intelfpga::ivdep(2,2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// end INTEL_CUSTOMIZATION
  // expected-error@+1 {{duplicate argument to 'ivdep'. attribute requires one or both of a safelen and array}}
  [[intelfpga::ivdep_exp(2,2)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-warning@+1 {{'ii' attribute takes at least 1 argument - attribute ignored}}
  [[intelfpga::ii]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-warning@+1 {{'ii' attribute takes no more than 1 argument - attribute ignored}}
  [[intelfpga::ii(2,2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-warning@+1 {{'max_concurrency' attribute takes at least 1 argument - attribute ignored}}
  [[intelfpga::max_concurrency]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-warning@+1 {{'max_concurrency' attribute takes no more than 1 argument - attribute ignored}}
  [[intelfpga::max_concurrency(2,2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-error@+1 {{duplicate argument to 'ivdep'. attribute requires one or both of a safelen and array}}
  [[intelfpga::ivdep_exp(2, 3)]] for (int i = 0; i != 10; ++i) // INTEL
      a[i] = 0;
  // expected-error@+1 {{duplicate argument to 'ivdep'. attribute requires one or both of a safelen and array}}
  [[intelfpga::ivdep_exp(a, b)]] for (int i = 0; i != 10; ++i) // INTEL
      a[i] = 0;
  // expected-error@+1 {{unknown argument to 'ivdep'. Expected integer or array variable}}
  [[intelfpga::ivdep_exp(2, 3.0)]] for (int i = 0; i != 10; ++i) // INTEL
      a[i] = 0;
}

// Test for incorrect argument value for Intel FPGA loop attributes
void goo() {
  int a[10];
  // no diagnostics are expected
  [[intelfpga::max_concurrency(0)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// INTEL_CUSTOMIZATION
  // expected-error@+1 {{'ivdep' attribute requires a positive integral compile time constant expression}}
  [[intelfpga::ivdep(0)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// end INTEL_CUSTOMIZATION
  // expected-error@+1 {{'ivdep' attribute requires a positive integral compile time constant expression}}
  [[intelfpga::ivdep_exp(0)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-error@+1 {{'ii' attribute requires a positive integral compile time constant expression}}
  [[intelfpga::ii(0)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-error@+1 {{'max_concurrency' attribute requires a non-negative integral compile time constant expression}}
  [[intelfpga::max_concurrency(-1)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-error@+1 {{unknown argument to 'ivdep'. Expected integer or array variable}}
  [[intelfpga::ivdep_exp("test123")]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-error@+1 {{'ii' attribute requires an integer constant}}
  [[intelfpga::ii("test123")]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // expected-error@+1 {{'max_concurrency' attribute requires an integer constant}}
  [[intelfpga::max_concurrency("test123")]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// INTEL_CUSTOMIZATION
  // expected-error@+1 {{'ivdep' attribute requires an integer constant}}
  [[intelfpga::ivdep("test123")]] for (int i = 0; i != 10; ++i)
      a[i] = 0;
// end INTEL_CUSTOMIZATION
  // expected-error@+1 {{unknown argument to 'ivdep'. Expected integer or array variable}}
  [[intelfpga::ivdep_exp("test123")]] for (int i = 0; i != 10; ++i) // INTEL
      a[i] = 0;
  // no diagnostics are expected
  [[intelfpga::ivdep_exp(a, 2)]] for (int i = 0; i != 10; ++i) // INTEL
      a[i] = 0;
  // no diagnostics are expected
  [[intelfpga::ivdep_exp(2, a)]] for (int i = 0; i != 10; ++i) // INTEL
      a[i] = 0;
}

// Test for Intel FPGA loop attributes duplication
void zoo() {
  int a[10];
  // no diagnostics are expected
  [[intelfpga::ivdep_exp]] // INTEL
  [[intelfpga::max_concurrency(2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  [[intelfpga::ivdep_exp]] // INTEL
  // expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  // expected-note@-2 {{previous attribute is here}}
  [[intelfpga::ivdep_exp]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  [[intelfpga::ivdep_exp]] // INTEL
  // expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 2}}
  // expected-note@-2 {{previous attribute is here}}
    [[intelfpga::ivdep_exp(2)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  [[intelfpga::ivdep_exp(2)]] // INTEL
  // expected-warning@-1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 4 >= safelen 2}}
  // expected-note@+1 {{previous attribute is here}}
  [[intelfpga::ivdep_exp(4)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  [[intelfpga::max_concurrency(2)]]
  // expected-error@-1 {{duplicate Intel FPGA loop attribute 'max_concurrency'}}
  [[intelfpga::max_concurrency(2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  [[intelfpga::ii(2)]]
  // expected-error@-1 {{duplicate Intel FPGA loop attribute 'ii'}}
  [[intelfpga::ii(2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  [[intelfpga::ii(2)]]
  // expected-error@-1 {{duplicate Intel FPGA loop attribute 'ii'}}
  [[intelfpga::max_concurrency(2)]]
  [[intelfpga::ii(2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

// INTEL_CUSTOMIZATION
  [[intelfpga::ivdep]]
  // expected-error@-1 {{duplicate Intel FPGA loop attribute 'ivdep'}}
  [[intelfpga::ivdep]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// end INTEL_CUSTOMIZATION
  [[intelfpga::ivdep_exp]] // INTEL
  // expected-warning@+2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  // expected-note@-2 {{previous attribute is here}}
  [[intelfpga::ivdep_exp]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// INTEL_CUSTOMIZATION
  [[intelfpga::ivdep]]
  // expected-error@-1 {{duplicate Intel FPGA loop attribute 'ivdep'}}
  [[intelfpga::ivdep(2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// end INTEL_CUSTOMIZATION
  [[intelfpga::ivdep_exp(2)]] // INTEL
  // expected-warning@-1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 2}}
  // expected-note@+1 {{previous attribute is here}}
  [[intelfpga::ivdep_exp]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// INTEL_CUSTOMIZATION
  [[intelfpga::ivdep(2)]]
  // expected-error@-1 {{duplicate Intel FPGA loop attribute 'ivdep'}}
  [[intelfpga::ivdep(4)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// end INTEL_CUSTOMIZATION
  [[intelfpga::ivdep_exp(a, 2)]] // INTEL
  // expected-warning@-1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 2}}
  // expected-note@+1 {{previous attribute is here}}
  [[intelfpga::ivdep_exp(a)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  [[intelfpga::ivdep_exp(2)]] // INTEL
  // expected-warning@-1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 4 >= safelen 2}}
  // expected-note@+1 {{previous attribute is here}}
  [[intelfpga::ivdep_exp(4)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  // no diagnostics are expected
  [[intelfpga::ivdep_exp(a)]] // INTEL
  [[intelfpga::ivdep_exp(2)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  [[intelfpga::ivdep_exp(a, 2)]] // INTEL
  // expected-warning@-1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen 2}}
  // expected-note@+1 {{previous attribute is here}}
  [[intelfpga::ivdep_exp]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  // Ensure we only diagnose conflict with the 'worst', not all.
  // expected-warning@+1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 5 >= safelen 3}}
  [[intelfpga::ivdep_exp(3)]] // INTEL
  // expected-warning@+1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 5 >= safelen 4}}
  [[intelfpga::ivdep_exp(4)]] // INTEL
  // expected-note@+1 2 {{previous attribute is here}}
  [[intelfpga::ivdep_exp(5)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  [[intelfpga::ivdep_exp(a, 2)]] // INTEL
  // expected-warning@-1 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 3 >= safelen 2}}
  // expected-note@+1 {{previous attribute is here}}
  [[intelfpga::ivdep_exp(a, 3)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

template<int A, int B, int C>
void ivdep_dependent() {
  int a[10];
// INTEL_CUSTOMIZATION
  // test this again to ensure we skip properly during instantiation.
  [[intelfpga::ivdep(3)]]
  // expected-error@-1 {{duplicate Intel FPGA loop attribute 'ivdep'}}
  [[intelfpga::ivdep(5)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// end INTEL_CUSTOMIZATION
  [[intelfpga::ivdep_exp(3)]] // INTEL
  // expected-warning@-1 2{{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 5 >= safelen 3}}
  // expected-note@+1 2{{previous attribute is here}}
  [[intelfpga::ivdep_exp(5)]] // INTEL
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

// INTEL_CUSTOMIZATION
  // expected-error@+1 {{'ivdep' attribute requires a positive integral compile time constant expression}}
  [[intelfpga::ivdep(C)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// end INTEL_CUSTOMIZATION

  [[intelfpga::ivdep_exp(C)]] // INTEL
  // expected-error@-1 {{'ivdep' attribute requires a positive integral compile time constant expression}}
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

// INTEL_CUSTOMIZATION
  // expected-error@+1 {{duplicate Intel FPGA loop attribute 'ivdep'}}
  [[intelfpga::ivdep(A)]]
  [[intelfpga::ivdep(B)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
// end INTEL_CUSTOMIZATION

  // expected-warning@+3 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 4 >= safelen 2}}
  // expected-note@+1 {{previous attribute is here}}
  [[intelfpga::ivdep_exp(A)]] // INTEL
  [[intelfpga::ivdep_exp(B)]] // INTEL
  // expected-warning@-2 {{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen 4 >= safelen 2}}
  // expected-note@-2 {{previous attribute is here}}
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  (void)[]() {
  // expected-warning@+3 2{{ignoring redundant Intel FPGA loop attribute 'ivdep': safelen INF >= safelen INF}}
  // expected-note@+1 2{{previous attribute is here}}
  [[intelfpga::ivdep_exp]] // INTEL
  [[intelfpga::ivdep_exp]] // INTEL
    while(true);
  };
}

template <int A, int B, int C>
void ii_dependent() {
  int a[10];
  // expected-error@+1 {{'ii' attribute requires a positive integral compile time constant expression}}
  [[intelfpga::ii(C)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  // expected-error@+1 {{duplicate Intel FPGA loop attribute 'ii'}}
  [[intelfpga::ii(A)]]
  [[intelfpga::ii(B)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

template <int A, int B, int C>
void max_concurrency_dependent() {
  int a[10];
  // expected-error@+1 {{'max_concurrency' attribute requires a non-negative integral compile time constant expression}}
  [[intelfpga::max_concurrency(C)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;

  // expected-error@+1 {{duplicate Intel FPGA loop attribute 'max_concurrency'}}
  [[intelfpga::max_concurrency(A)]]
  [[intelfpga::max_concurrency(B)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

int main() {
  kernel_single_task<class kernel_function>([]() {
    foo();
    boo();
    goo();
    zoo();
    ivdep_dependent<4, 2, 1>();
    //expected-note@-1 +{{in instantiation of function template specialization}}
    ivdep_dependent<2, 4, -1>();
    //expected-note@-1 +{{in instantiation of function template specialization}}
    ii_dependent<2, 4, -1>();
    //expected-note@-1 +{{in instantiation of function template specialization}}
    max_concurrency_dependent<1, 4, -2>();
    //expected-note@-1 +{{in instantiation of function template specialization}}
  });
  return 0;
}
