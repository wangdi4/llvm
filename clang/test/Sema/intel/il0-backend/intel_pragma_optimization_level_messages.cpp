// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s 
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -pragma-optimization-level=Intel -verify -o - %s 
//***INTEL: pragma optimization_level

#pragma GCC optimization_level 1
struct AAAAAA{                  
  int a;
};
#pragma optimization_level -1 // expected-warning {{expected an integer constant}}
#pragma optimization_level 10 // expected-warning {{optimization level must be between 0 and 3}}
#pragma optimization_level 1+1 // expected-warning {{extra text after expected end of preprocessing directive}}
#pragma optimization_level reset // expected-warning {{expected an integer constant}}
#pragma optimization_level 1
struct AAAAAA1{                  // expected-warning {{optimization level applies only to function definition}}
  int a;
};

void aaaa() {
  ;
}
#pragma GCC optimization_level -1 // expected-warning {{expected reset or an integer constant}}
#pragma GCC optimization_level 10 // expected-warning {{optimization level must be between 0 and 3}}
#pragma GCC optimization_level reset
#pragma intel optimization_level -1 // expected-warning {{expected an integer constant}}
#pragma intel optimization_level 10 // expected-warning {{optimization level must be between 0 and 3}}
#pragma intel optimization_level reset // expected-warning {{expected an integer constant}}



#pragma optimization_level 0
#pragma optimization_level 3
#pragma intel optimization_level 1
#pragma intel optimization_level 2
#pragma GCC optimization_level 0
#pragma GCC optimization_level 3
#pragma GCC optimization_level 1
#pragma GCC optimization_level reset
int main() {
  int i = 13, j;
struct S {
  int a;
};

#pragma optimization_level 1 // expected-error {{this pragma must immediately precede a declaration}}
  return (0);
}

struct S {
#pragma GCC optimization_level 3 
  int a;
};

static int a;

#pragma optimization_level 1 // expected-error {{this pragma must immediately precede a declaration}}
