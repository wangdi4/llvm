// RUN: %clang_cc1 -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: attribute avoid_false_share

void www();

// CHECK: define private void @.DIRECTIVE.() #0 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", metadata %struct.S* @d)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
__declspec(avoid_false_share) struct S {
  int a;
} d;

// CHECK: define private void @.DIRECTIVE..1() #0 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", metadata %class.C* @e, metadata !"EXCEPTION_ID", metadata !"1")
// CHECK-NEXT: ret void
// CHECK-NEXT: }
__declspec(avoid_false_share("1")) class C {
  int a;
  public:
  __declspec(avoid_false_share) int b; // expected-warning {{attribute 'avoid_false_share' can be applied only to non-reference variables}}
  __declspec(avoid_false_share) static int g;
} e;

// CHECK: define private void @.DIRECTIVE..2() #0 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* @_ZL1a)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
static __declspec(avoid_false_share) int a;

// CHECK: define private void @.DIRECTIVE..3() #0 {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* @dd1)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
extern "C" __declspec(avoid_false_share) int dd1;
__declspec(avoid_false_share) int fun() {return (0);} // expected-warning {{attribute 'avoid_false_share' can be applied only to non-reference variables}}

// CHECK: define i32 @main(
int main(int argc, char **argv)
{
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %i)
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %lll)
  __declspec(avoid_false_share) int i, lll;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* @_ZZ4mainE6localS)
  static __declspec(avoid_false_share) int localS;
  return (i);
}
// CHECK: }


