// RUN: %clang_cc1 -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: attribute avoid_false_share

void www();

// CHECK: define private void @.DIRECTIVE.() #0 {
// CHECK: call void @llvm.intel.pragma(metadata !1)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
__declspec(avoid_false_share) struct S {
  int a;
} d;

// CHECK: define private void @.DIRECTIVE.1() #0 {
// CHECK: call void @llvm.intel.pragma(metadata !2)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
__declspec(avoid_false_share("1")) class C {
  int a;
  public:
  __declspec(avoid_false_share) int b; // expected-warning {{attribute 'avoid_false_share' can be applied only to non-reference variables}}
  __declspec(avoid_false_share) static int g;
} e;

// CHECK: define private void @.DIRECTIVE.2() #0 {
// CHECK: call void @llvm.intel.pragma(metadata !3)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
static __declspec(avoid_false_share) int a;

// CHECK: define private void @.DIRECTIVE.3() #0 {
// CHECK: call void @llvm.intel.pragma(metadata !4)
// CHECK-NEXT: ret void
// CHECK-NEXT: }
extern "C" __declspec(avoid_false_share) int dd1;
__declspec(avoid_false_share) int fun() {return (0);} // expected-warning {{attribute 'avoid_false_share' can be applied only to non-reference variables}}

// CHECK: define i32 @main(
int main(int argc, char **argv)
{
// CHECK: call void @llvm.intel.pragma(metadata !{metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", i32* %i})
// CHECK: call void @llvm.intel.pragma(metadata !{metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", i32* %lll})
  __declspec(avoid_false_share) int i, lll;
// CHECK: call void @llvm.intel.pragma(metadata !5)
  static __declspec(avoid_false_share) int localS;
  return (i);
}
// CHECK: }

// CHECK: !1 = metadata !{metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", %struct.S* @d}
// CHECK-NEXT: !2 = metadata !{metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", %class.C* @e, metadata !"EXCEPTION_ID", metadata !"1"}
// CHECK-NEXT: !3 = metadata !{metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", i32* @_ZL1a}
// CHECK-NEXT: !4 = metadata !{metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", i32* @dd1}
// CHECK-NEXT: !5 = metadata !{metadata !"AVOID_FALSE_SHARE", metadata !"LVALUE", metadata !"SIMPLE", i32* @_ZZ4mainE6localS}

