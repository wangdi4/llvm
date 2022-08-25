// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s

// CHECK-LABEL: foo
void foo(int n) {
  _Complex double a, b[10], c[10], *d;
  _Complex double e[n];
  _Complex double f[n];
  int g[n];
  _Complex double z = 3.0;
  _Complex double *zp = &z;
  _Complex double &zr = z;
  _Complex double *(&zpr) = zp;
  _Complex double *p = &a;
  _Complex double &r = a;
  _Complex double (&h)[10] = b;
  _Complex double *(&rr) = p;
// CHECK: "DIR.OMP.PARALLEL"
// CHECK: "QUAL.OMP.REDUCTION.ADD:CMPLX"
#pragma omp parallel reduction(+: a)
  for(int i = 0 ; i <  10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:CMPLX"
#pragma omp parallel reduction(+: b)
  for(int i = 0 ; i <  10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.CMPLX"
#pragma omp parallel reduction(+: c[1:2])
  for(int i = 0 ; i <  10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.CMPLX"
#pragma omp parallel reduction(+: d[1:2])
  for(int i = 0 ; i <  10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:CMPLX"
#pragma omp parallel reduction(+: e)
  for(int i = 0 ; i <  10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.CMPLX"
#pragma omp parallel reduction(+: f[1:2])
  for(int i = 0 ; i <  10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD"
#pragma omp parallel reduction(+: g)
  for(int i = 0 ; i <  10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.CMPLX"
#pragma omp parallel reduction(+: zp[0:1])
  for(int i = 0 ; i <  10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:BYREF.CMPLX"
#pragma omp parallel reduction(+: zr)
  for(int i = 0 ; i <  10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.CMPLX"
#pragma omp parallel reduction(+: zpr[0:1])
  for(int i = 0 ; i <  10 ; i++) {}

// CHECK: "QUAL.OMP.REDUCTION.ADD:ARRSECT.CMPLX"
#pragma omp parallel reduction(+:p[1:2])
  for(int i = 0 ; i < 10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:BYREF.CMPLX"
#pragma omp parallel reduction(+:r)
  for(int i = 0 ; i < 10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:BYREF.CMPLX"
#pragma omp parallel reduction(+:h)
  for(int i = 0 ; i < 10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.CMPLX"
#pragma omp parallel reduction(+:h[1:2])
  for(int i = 0 ; i < 10 ; i++) {}
// CHECK: "QUAL.OMP.REDUCTION.ADD:BYREF.ARRSECT.CMPLX"
#pragma omp parallel reduction(+:rr[1:2])
  for(int i = 0 ; i < 10 ; i++) {}
}
// end INTEL_COLLAB
