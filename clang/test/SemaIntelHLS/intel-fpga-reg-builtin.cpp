//RUN: %clang_cc1 -fhls -fsyntax-only -verify -pedantic %s
//RUN: %clang_cc1 -fsyntax-only -verify -pedantic %s -DWITHOUTFLAG

#ifndef WITHOUTFLAG
class A {
public:
  A(int a) {
    m_val = new int(a);
  };
  A(const A &a) { m_val = a.m_val; }
  ~A() { delete m_val; }
private:
    int *m_val;
};

struct st {
  int a;
  float b;
};

struct inner {
  void (*fp)(); // expected-note {{Field with illegal type declared here}}
};

struct outer {
  inner A;
};

void foo() {
  int a = 123;
  int b = __builtin_fpga_reg(a);
  int c = __builtin_fpga_reg(2.0f);
  int d = __builtin_fpga_reg( __builtin_fpga_reg( b+12 ));
  int e = __builtin_fpga_reg( __builtin_fpga_reg( a+b ));
  float f = 3.4f;
  int g = __builtin_fpga_reg((int)f);
  A h(5);
  A j = __builtin_fpga_reg(h);
  struct st i = {1, 5.0f};
  struct st ii = __builtin_fpga_reg(i);
  int *ap = &a;
  int *bp = __builtin_fpga_reg(ap);

  void (*fp1)();
  void (*fp2)() = __builtin_fpga_reg(fp1);
  //expected-error@-1{{Illegal argument of type 'void (*)()'  to __builtin_fpga_reg.}}
  struct outer iii;
  struct outer iv = __builtin_fpga_reg(iii);
  //expected-error@-1{{Illegal field in argument to __builtin_fpga_reg}}
  void *vp = __builtin_fpga_reg();
  // expected-error@-1{{too few arguments to function call, expected 1, have 0}}
  int tmp = __builtin_fpga_reg(1, 2);
  // expected-error@-1{{too many arguments to function call, expected 1, have 2}}
}
#else
void bar() {
  int a = 123;
  int b = __builtin_fpga_reg(a);
  // expected-error@-1{{'__builtin_fpga_reg' is only available in HLS}}
}
#endif
