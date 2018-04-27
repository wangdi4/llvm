//RUN: %clang_cc1 -fhls -fsyntax-only -verify -pedantic %s
//RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s -DWITHOUTFLAG

class A {
public:
    A(int a) {
        m_val = new int(a);
    };
    A(const A& a) {m_val = a.m_val;}
    ~A() { delete m_val; }
private:
    int *m_val;
};

struct st {
  int a;
  float b;
};

void foo() {
#ifndef WITHOUTFLAG
  int a=123;
  int b = __builtin_fpga_reg(a);
  int c = __builtin_fpga_reg(2.0f);
  int d = __builtin_fpga_reg( __builtin_fpga_reg( b+12 ));
  int e = __builtin_fpga_reg( __builtin_fpga_reg( a+b ));
  float f = 3.4f;
  int g = __builtin_fpga_reg((int)f);
  A h(5);
  A j = __builtin_fpga_reg(h); // expected-error{{non-POD and array types cannot be passed to __builtin_fpga_reg}}
  struct st i = {1, 5.0f};
  struct st ii = __builtin_fpga_reg(i);
  int *ap = &a;
  int *bp = __builtin_fpga_reg(ap);

  void * vp = __builtin_fpga_reg();
  // expected-error@-1{{too few arguments to function call, expected 1, have 0}}
  int tmp = __builtin_fpga_reg(1, 2);
  // expected-error@-1{{too many arguments to function call, expected 1, have 2}}
#else
  int a=123;
  int b = __builtin_fpga_reg(a);
  // expected-error@-1{{'__builtin_fpga_reg' is only available in HLS}}
#endif
}
