// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:   -fcxx-exceptions -fexceptions \
// RUN:   -triple x86_64-unknown-linux-gnu %s | FileCheck %s

int foo()
{
  int error;
  try {
    int l=0;
    #pragma omp parallel for private(l)
    for (l=0;l<10; ++l) {
      try {
        if (l >= 10) throw 1;
// CHECK: to label %[[UB:.*unreachable[0-9]*]] unwind label
      } catch (...) { l = 30;}
// CHECK: to label %unreachable unwind label
      throw 1;
   }

   if (error)
// CHECK: to label %[[UB2:.*unreachable[0-9]*]] unwind label
     throw 1;
 } catch (...) { }
//CHECK: [[UB]]:
 int l=0;
 #pragma omp parallel for private(l)
 for (int l=0;l<10; ++l) {
   try {
// CHECK: to label %[[UB1:.*unreachable[0-9]*]] unwind label
    if (l >= 10) throw 1;
  } catch (...) {l = 10; }
 }
 return 0;
}
//CHECK:  [[UB1]]:
//CHECK:  [[UB2]]:
