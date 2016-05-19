// CQ#381613
// RUN: %clang_cc1 -fintel-compatibility -verify %s

extern "C" int printf (const char *__restrict __format, ...);

struct C3 {
  int m0;
  C3():m0(7) {};
  ~C3() {
    m0 = (int)3;
    printf(" ~C3()\n");
  }
};

struct C2 {
  C3 m0;
  C2() { printf("C2()"); }
};

int main() {
  C2 o2;
  printf("o2.m0 = %x\n", o2.m0); // expected-warning {{cannot pass non-POD object of type 'C3' to variadic function; expected type from format string was 'unsigned int'}}
}
