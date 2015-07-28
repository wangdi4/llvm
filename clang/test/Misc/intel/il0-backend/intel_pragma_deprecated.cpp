// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//expected-no-diagnostics
//***INTEL: pragma deprecated test

void www();

struct S {
  int a;
} d;

class C {
  int a;
  public:
  int b;
} e;
#pragma deprecated (e, "") 
#pragma deprecated (e, "long") 
extern "C" int ddddddd;
extern "C" int dd1;
#pragma deprecated (ddddddd, d.a)

// CHECK: define i32 @main(
int main(int argc, char **argv)
{
  int i, lll;
  static int localS;
#pragma deprecated (argc, i, "short") 
#pragma deprecated (ddddddd, dd1, localS, "long")
  
// CHECK: ret i32
  return (i);
}
// CHECK: }

// CHECK: = !{!"{{C|c}}lang
// CHECK-NOT: = !{
