// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -disable-llvm-passes \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct S {
  int foo(int);
  short s;
};

#pragma omp declare simd linear(fp : 8)
float add_1(float *fp, int *ip, long long* llp, double *dp, short *sp, char *cp,
            long l, double d, int &ir, double &dr, double arr[10],
            void (*fncp)(void), void *vp,
            int (S::*pmf)(int), short (S::*pm)) {
//CHECK: call void @llvm.intel.directive.elementsize(ptr %fp, i64 4)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %ip, i64 4)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %llp, i64 8)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %dp, i64 8)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %sp, i64 2)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %cp, i64 1)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %ir, i64 4)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %dr, i64 8)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %arr, i64 8)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %fncp, i64 0)
//CHECK: call void @llvm.intel.directive.elementsize(ptr %vp, i64 0)
//CHECK-NOT: call void @llvm.intel.directive.elementsize(ptr %pmf
//CHECK-NOT: call void @llvm.intel.directive.elementsize(ptr %pm
  return 1.0+*fp;
}

float other(float *fff) {
  //CHECK-NOT: call void @llvm.intel.directive.elementsize(ptr %fff, i64 4)
  return *fff;
}
