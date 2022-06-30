// INTEL_COLLAB

// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-targets=spir64 -emit-llvm -o - %s | FileCheck %s
//
// expected-no-diagnostics

void k()
{
  int ts = 0;
  //CHECK: [[A:%a.*]] = alloca [8192 x i32],
  int a[8192];
  int R = 64;
  //CHECK: DIR.OMP.TARGET
  //CHECK: TEAMS{{.*}}"QUAL.OMP.SHARED:TYPED"(ptr [[A]]
  //CHECK: DISTRIBUTE{{.*}}"QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[A]]
  //CHECK: DIR.OMP.END.DISTRIBUTE
  //CHECK: DIR.OMP.END.TEAMS
  //CHECK: DIR.OMP.END.TARGET
  #pragma omp target teams distribute parallel for map(tofrom:ts) firstprivate(a) reduction(+:ts)
  for (int r=0; r < R; r++)
  {
      for(int i=0; i < 8192; i++)
        a[i] = 0;
      a[0] = 1;
      for(int i=0; i < 8192; i++)
        ts += a[i];
  }
  //CHECK: DIR.OMP.TARGET
  //CHECK-NOT: TEAMS{{.*}}"QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[A]]{{.*}}{{$}}
  //CHECK: DISTRIBUTE{{.*}}"QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[A]]
  //CHECK: DIR.OMP.END.DISTRIBUTE
  //CHECK: DIR.OMP.END.TEAMS
  //CHECK: DIR.OMP.END.TARGET
  #pragma omp target teams distribute parallel for map(tofrom:ts) firstprivate(a) reduction(+:ts)
  for (int r=0; r < R; r++)
  {
      for(int i=0; i < 8192; i++)
        a[i] = 0;
      a[0] = 1;
      for(int i=0; i < 8192; i++)
        ts += a[i];
  }
}

// end INTEL_COLLAB
