// RUN: %clang_cc1 -IntelCompat -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma check_stack

#pragma check_stack(on)
#pragma check_stack("asq" off) // expected-warning {{'on' or 'off' expected}}
// CHECK: define i32 @main()
int main() {
  int i = 13, j;
// CHECK: call void @llvm.intel.pragma(metadata !1)
#pragma check_stack on
#pragma check_stack +
#pragma check_stack(+)
#pragma check_stack -
#pragma check_stack(-)
// CHECK: ret i32 0
  return (0);
}
// CHECK: }

struct S {
#pragma check_stack // expected-warning {{'on' or 'off' expected}}
  int a;
};

static int a;

#pragma check_stack off)
#pragma check_stack (on
#pragma check_stack (on)
#pragma check_stack (on  off // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK: define i32 @{{.*}}www{{.*}}
int www() {
// CHECK: call void @llvm.intel.pragma(metadata !2)
   return a;
}
// CHECK: }

#pragma check_stack (off)
static void aaa() {
  return;
}
// CHECK: define void @{{.*}}bbb{{.*}}
void bbb() {
// CHECK-NOT: call void @llvm.intel.pragma(metadata 
  aaa();
  return;
}
// CHECK: }
// CHECK: define internal void @{{.*}}aaa{{.*}}
// CHECK: call void @llvm.intel.pragma(metadata !1)
// CHECK: }

// CHECK: !1 = metadata !{metadata !"NOCHECK_STACK"}
// CHECK-NEXT: !2 = metadata !{metadata !"CHECK_STACK"}
