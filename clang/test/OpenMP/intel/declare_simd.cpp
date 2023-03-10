// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -disable-llvm-passes \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

void add_1(float *d);

#pragma omp declare simd linear(d : 8)
#pragma omp declare simd inbranch simdlen(32)
#pragma omp declare simd notinbranch
void add_1(float *d);

#pragma omp declare simd linear(d : 8)
#pragma omp declare simd inbranch simdlen(32)
#pragma omp declare simd notinbranch
void add_1(float *d){}

void add_1(float *d);

void add_2(float *d);

#pragma omp declare simd linear(d : 8)
#pragma omp declare simd inbranch simdlen(32)
#pragma omp declare simd notinbranch
void add_2(float *d);

#pragma omp declare simd linear(d : 8)
#pragma omp declare simd inbranch simdlen(16)
#pragma omp declare simd notinbranch
void add_2(float *d){}

void add_2(float *d);

// CHECK-NOT: _ZGVbN2vv__Z5add_1Pf
// CHECK-NOT: _ZGVcN4vv__Z5add_1Pf
// CHECK-NOT: _ZGVdN4vv__Z5add_1Pf
// CHECK-NOT: _ZGVeN8vv__Z5add_1Pf
// CHECK-NOT: _ZGVbM32vv__Z5add_1Pf
// CHECK-NOT: _ZGVcM32vv__Z5add_1Pf
// CHECK-NOT: _ZGVdM32vv__Z5add_1Pf
// CHECK-NOT: _ZGVeM32vv__Z5add_1Pf
// CHECK-NOT: _ZGVbN4l32v__Z5add_1Pf
// CHECK-NOT: _ZGVcN8l32v__Z5add_1Pf
// CHECK-NOT: _ZGVdN8l32v__Z5add_1Pf
// CHECK-NOT: _ZGVeN16l32v__Z5add_1Pf
// CHECK-NOT: _ZGVbM4l32v__Z5add_1Pf
// CHECK-NOT: _ZGVcM8l32v__Z5add_1Pf
// CHECK-NOT: _ZGVdM8l32v__Z5add_1Pf
// CHECK-NOT: _ZGVeM16l32v__Z5add_1Pf

// CHECK-DAG: _ZGVbN2v__Z5add_1Pf
// CHECK-DAG: _ZGVcN4v__Z5add_1Pf
// CHECK-DAG: _ZGVdN4v__Z5add_1Pf
// CHECK-DAG: _ZGVeN8v__Z5add_1Pf
// CHECK-DAG: _ZGVbM32v__Z5add_1Pf
// CHECK-DAG: _ZGVcM32v__Z5add_1Pf
// CHECK-DAG: _ZGVdM32v__Z5add_1Pf
// CHECK-DAG: _ZGVeM32v__Z5add_1Pf
// CHECK-DAG: _ZGVbN4l32__Z5add_1Pf
// CHECK-DAG: _ZGVcN8l32__Z5add_1Pf
// CHECK-DAG: _ZGVdN8l32__Z5add_1Pf
// CHECK-DAG: _ZGVeN16l32__Z5add_1Pf
// CHECK-DAG: _ZGVbM4l32__Z5add_1Pf
// CHECK-DAG: _ZGVcM8l32__Z5add_1Pf
// CHECK-DAG: _ZGVdM8l32__Z5add_1Pf
// CHECK-DAG: _ZGVeM16l32__Z5add_1Pf

// CHECK-DAG: _ZGVbN2v__Z5add_2Pf
// CHECK-DAG: _ZGVcN4v__Z5add_2Pf
// CHECK-DAG: _ZGVdN4v__Z5add_2Pf
// CHECK-DAG: _ZGVeN8v__Z5add_2Pf
// CHECK-DAG: _ZGVbM16v__Z5add_2Pf
// CHECK-DAG: _ZGVcM16v__Z5add_2Pf
// CHECK-DAG: _ZGVdM16v__Z5add_2Pf
// CHECK-DAG: _ZGVeM16v__Z5add_2Pf
// CHECK-DAG: _ZGVbN4l32__Z5add_2Pf
// CHECK-DAG: _ZGVcN8l32__Z5add_2Pf
// CHECK-DAG: _ZGVdN8l32__Z5add_2Pf
// CHECK-DAG: _ZGVeN16l32__Z5add_2Pf
// CHECK-DAG: _ZGVbM4l32__Z5add_2Pf
// CHECK-DAG: _ZGVcM8l32__Z5add_2Pf
// CHECK-DAG: _ZGVdM8l32__Z5add_2Pf
// CHECK-DAG: _ZGVeM16l32__Z5add_2Pf
// CHECK-DAG: _ZGVbM32v__Z5add_2Pf
// CHECK-DAG: _ZGVcM32v__Z5add_2Pf
// CHECK-DAG: _ZGVdM32v__Z5add_2Pf
// CHECK-DAG: _ZGVeM32v__Z5add_2Pf
