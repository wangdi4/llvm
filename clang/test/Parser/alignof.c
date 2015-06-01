// RUN: %clang_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

#define ALIGN1(x) __ALIGNOF__(x)

__declspec(align(16)) struct B {
  __int64 i;
  __int64 j;
};

struct __declspec(align(16)) C {
  __int64 i;
  __int64 j;
};

typedef __declspec(align(16)) struct _E {
  __int64 i;
  __int64 j;
} E;

typedef struct __declspec(align(16)) _F {
  __int64 i;
  __int64 j;
} F;

int main() {

  int retval = 0;

#ifndef __GNUC__
  /* gcc ignores alignment on struct */
  if (ALIGN1(struct B) != 16) {
    retval++;
  }
#endif /* __GNUC__ */
  if (ALIGN1(struct C) != 16) {
    retval++;
  }
  if (ALIGN1(E) != 16) {
    retval++;
  }
  if (ALIGN1(F) != 16) {
    retval++;
  }

  return retval;
}
