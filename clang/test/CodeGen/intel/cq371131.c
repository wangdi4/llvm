// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility %s -emit-llvm -o - | FileCheck %s


struct Empty { };
struct EmptyTest {struct Empty s; int i;};
struct EmptyTest empty = { 3 };

// CHECK: [[EmptyTest:%.+]] = type { [[Empty:%.+]], i32 }
// CHECK: [[Empty]] = type {}
 
// CHECK: [[empty:@.+]] = global [[EmptyTest]] zeroinitializer
 
int f() {
  return empty.i;
// CHECK: [[O:%.+]] = load i32, i32* getelementptr inbounds ([[EmptyTest]], [[EmptyTest]]* @empty, i32 0, i32 1)
// CHECK: ret i32 [[O]]
}
