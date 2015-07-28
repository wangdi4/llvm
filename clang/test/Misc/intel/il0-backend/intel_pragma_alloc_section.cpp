// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma alloc_section test

void www();

struct S {
  int a;
} d;

class C {
  int a;
  public:
  int b;
} e;
#pragma alloc_section (e, "") // expected-warning {{invalid use of null string; pragma ignored}}
#pragma alloc_section (e, "long") // expected-warning {{variable 'e' has no "C" linkage specification}}
extern "C" int ddddddd;
extern "C" int dd1;
// CHECK: define private void @.DIRECTIVE.() #0 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ALLOC_SECTION", metadata !"short", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* @ddddddd)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
#pragma alloc_section (ddddddd, "short")

// CHECK: define i32 @main(
int main(int argc, char **argv)
{
  int i, lll;
  static int localS;
#pragma alloc_section (argc, i, "short") // expected-warning {{variable 'argc' has local storage}} expected-warning {{variable 'i' has local storage}}
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"ALLOC_SECTION", metadata !"long", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* @ddddddd, metadata !"LVALUE", metadata !"SIMPLE", metadata i32* @dd1)
#pragma alloc_section (ddddddd, dd1, localS, "long") // expected-warning {{variable 'localS' has no "C" linkage specification}} 
  
// CHECK: ret i32
  return (i);
}
// CHECK: }


