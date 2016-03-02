// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

union U
{
  struct _s
  {
    int a, b, c;
  } __data;
  char __size[40];
} pthread_mutex_t;

struct S {
  U  u;
  void foo();
};

// CHECK: [[U:%.+]] = type { [[US:%.+]], [28 x i8] }
// CHECK: [[US]] = type { i32, i32, i32 }
// CHECK: [[S:%.+]] = type { [[U]] }

// CHECK: [[C:@.+]] = private constant [[U]] { [[US]] { i32 1, i32 2, i32 3 }, [28 x i8] undef }

void S::foo()
{
  u = { { 1, 2, 3 } };
// CHECK:  [[TA:%.+]] = alloca [[S]]*
// CHECK:  store [[S]]* %this, [[S]]** [[TA]]
// CHECK:  [[T1:%.+]] = load [[S]]*, [[S]]** [[TA]]
// CHECK:  [[T2:%.+]] = getelementptr inbounds [[S]], [[S]]* [[T1]], i32 0, i32 0
// CHECK:  [[tem0:%.+]] = bitcast [[U]]* [[T2]] to i8*
// CHECK:  call void {{@.+}}(i8* [[tem0]], i8* bitcast ([[U]]* [[C]] to i8*), i64 40, i32 4, i1 false)
}
