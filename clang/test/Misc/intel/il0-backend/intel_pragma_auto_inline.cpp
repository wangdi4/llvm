// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma auto_inline

#pragma auto_inline(on)
#pragma auto_inline("asq" off) // expected-warning {{'on' or 'off' expected}}
// CHECK: define i32 @main()
int main() {
  int i = 13, j;
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"AUTO_INLINE")
// CHECK: ret i32 0
#pragma auto_inline on
  return (0);
}

struct S {
#pragma auto_inline
  int a;
  int get();
};
// CHECK: define i32 {{.*}}get{{.*}}
int S::get() {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOAUTO_INLINE")
  return (a);
}
// CHECK: }

static int a;

#pragma auto_inline off)
#pragma auto_inline (on
#pragma auto_inline (on)
#pragma auto_inline (on  off // expected-warning {{extra text after expected end of preprocessing directive}}
// CHECK: define i32 @{{.*}}www{{.*}}
int www() {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"AUTO_INLINE")
   return a;
}
// CHECK: }

#pragma auto_inline (off)
// CHECK: define i32 @{{.*}}aaa{{.*}}
int aaa(int s) {
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"NOAUTO_INLINE")
  return s;
}
// CHECK: }

// CHECK: define i32 @{{.*}}bbb{{.*}}
int bbb(int s) {
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma(metadata !
  return s;
}
// CHECK: }

