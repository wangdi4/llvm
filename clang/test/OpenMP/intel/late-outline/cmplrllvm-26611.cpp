// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

typedef struct {
  int a;
  double *b;
} C1;
#pragma omp declare mapper(C1 s) map(to : s.a) map(from : s.b [0:2])

typedef struct {
  int a;
  double *b;
  C1 c;
} C;


#pragma omp declare mapper(C s) map(to : s.a, s.c) map(from : s.b [0:2])

typedef struct {
  int e;
  C f;
  int h;
} D;

int main() {
  constexpr int N = 10;
  D s;
  s.e = 111;
  s.f.a = 222;
  s.f.c.a = 777;
  double x[2];
  double x1[2];
  x[1] = 20;
  s.f.b = &x[0];
  s.f.c.b = &x1[0];
  s.h = N;

  D *sp = &s;
  D **spp = &sp;

  // CHECK: store ptr %sp, ptr %spp
  // CHECK: [[L:%[0-9]+]] = load ptr, ptr %spp
  // CHECK: [[L1:%[0-9]+]] = load ptr, ptr %spp
  // CHECK: [[TV1:%[0-9]+]] = call token{{.*}}region.entry{{.*}}DIR.OMP.TARGET
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr [[L1]], ptr
  // CHECK-SAME: "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %arrayidx
  // CHECK: region.exit(token [[TV1]]) [ "DIR.OMP.END.TARGET"() ]
  #pragma omp target map(tofrom : spp[0][0]) //firstprivate(p)
  {
    spp[0][0].e = 333;
    spp[0][0].f.a = 444;
    spp[0][0].f.c.a = 555;
    spp[0][0].f.b[1] = 40;
  }
}
