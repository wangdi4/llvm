// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s 
//***INTEL: pragma intel optimization_parameter target_arch=

#pragma intel optimization_parameter target_arch=ATOM
struct AAAAAA{                  
  int a;
};
#pragma intel optimization_parameter target_arch=AVX 
struct AAAAAA1{               
#pragma intel optimization_parameter target_arch=10 
  int a;
};

#pragma intel optimization_parameter 
#pragma intel optimization_parameter target_ar
#pragma intel optimization_parameter wfrrhff (10)+
void aaaa() {
  ;
}

#pragma intel optimization_parameter target_arch X86

int main() {
  int i = 13, j;
struct S {
  int a;
};

#pragma intel optimization_parameter target_arch=X86 // expected-error {{this pragma must immediately precede a declaration}}
  return (0);
}

struct S {
#pragma intel optimization_parameter target_arch=SSE2
  int a;
};

static int a;

#pragma intel optimization_parameter target_arch=ATOM// expected-error {{this pragma must immediately precede a declaration}}

