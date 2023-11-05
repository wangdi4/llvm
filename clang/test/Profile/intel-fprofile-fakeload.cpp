// RUN: %clang_cc1 %s -fintel-compatibility -O2 -o - -emit-llvm -fprofile-ml-use 
// RUN: %clang_cc1 %s -fintel-compatibility -O2 -o - -emit-llvm -mllvm -enable-tbaa-prop=false
// CHECK: br i1 %tobool{{.*}}if.then
// CHECK-NOT: select i1
// CHECK-NOT: call{{.*}}fakeload

// The pass pipeline was not removing "fakeload" intrinsics for the option
// sets above. These were causing the null checks on "p" below to be removed.
// The fakeload attributes such as "nonnull" are control-flow-sensitive and
// will cause problems if they are left in the IR, and get moved to other
// blocks.
struct obj {
  int x;
  int &foo() { return x; }
};

__attribute__((noinline)) int inc(obj *p) {
  int x =4;
  if (p) {
    x = p->foo();
  }
  int y = 5;
  if (p) {
    y = p->foo();
  }
 
  return x+y;
}

int main() {
  obj *p = nullptr;
  return inc(p);
}
