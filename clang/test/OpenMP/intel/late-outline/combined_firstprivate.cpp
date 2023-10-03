// INTEL_COLLAB

// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
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

struct S {
  float scalar;
        // CHECK: define {{.*}}run
	void run (unsigned N)
	{
          //CHECK: %scalar{{.*}} = getelementptr inbounds %struct.S, ptr %this1, i32 0, i32 0
          //CHECK-NEXT: store ptr %scalar{{.*}}, ptr %scalar, align 8
          //CHECK: [[SCALAR:%scalar.*]] = getelementptr inbounds %struct.S, ptr %this1, i32 0, i32 0
          //CHECK: DIR.OMP.TARGET{{.*}}FIRSTPRIVATE:TYPED"(ptr [[SCALAR]]
          //CHECK: DIR.OMP.TEAMS{{.*}}SHARED:TYPED"(ptr [[SCALAR]]
          //CHECK: DIR.OMP.DISTRIBUTE.PARLOOP{{.*}}FIRSTPRIVATE:TYPED"(ptr [[SCALAR]]
	  #pragma omp target teams distribute parallel for simd firstprivate(scalar)
	  for (unsigned index = 0; index < N; ++index)
		float f = scalar;
	}

};

void foo() {
  S n;
  n.run(32);
}
// end INTEL_COLLAB
