// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp \
// RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fintel-openmp-region-late-collapsed-loops \
// RUN:   -triple x86_64-unknown-linux-gnu \
// RUN:   | FileCheck %s --check-prefixes CHECK,HOST

// RUN: %clang_cc1 -emit-llvm-bc -o %t_host.bc %s -std=c++14 -fopenmp \
// RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fintel-openmp-region-late-collapsed-loops \
// RUN:   -triple x86_64-unknown-linux-gnu

// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp \
// RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fintel-openmp-region-late-collapsed-loops \
// RUN:   -triple spir64 -fopenmp-is-device \
// RUN:   -fopenmp-host-ir-file-path %t_host.bc \
// RUN:   | FileCheck %s --check-prefixes CHECK,TARG

// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fexceptions -fopenmp \
// RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
// RUN:   -fintel-openmp-region-late-collapsed-loops \
// RUN:   -triple x86_64-unknown-linux-gnu \
// RUN:   | FileCheck %s --check-prefixes CHECK,HOST

// RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp \
// RUN:   -fopenmp-targets=spir64 -fintel-compatibility -fopenmp-late-outline \
// RUN:   -triple x86_64-unknown-linux-gnu \
// RUN:   -fintel-openmp-region-early-collapsed-loops \
// RUN:   | FileCheck %s --check-prefix OFF

void foo(float *, float *, int, int, int, int, int, int);

// CHECK-LABEL: test_one
void test_one(float *A0, float *A1, int Ni, int Nj, int Nk) {
  int i, j, k;
  int size=Ni*Nj*Nk;

  //HOST:[[A0:%A0.addr]] = alloca float[[AS:.*]]*, align 8
  //TARG:[[A0A:%A0.addr]] = alloca float[[AS:.*]]*, align 8
  //TARG-NEXT:[[A0:%.*.ascast]] = addrspacecast float

  //CHECK:[[A1:%A1.addr]] = alloca float
  //CHECK:[[NI:%Ni.*]] = alloca i32,
  //CHECK:[[NJ:%Nj.*]] = alloca i32,
  //CHECK:[[NK:%Nk.*]] = alloca i32,

  //HOST:[[I:%i.*]] = alloca i32,
  //TARG:[[IA:%i.*]] = alloca i32,
  //TARG-NEXT:[[I:%.*.ascast]] = addrspacecast i32* [[IA]]

  //HOST:[[J:%j.*]] = alloca i32,
  //TARG:[[JA:%j.*]] = alloca i32,
  //TARG-NEXT:[[J:%.*.ascast]] = addrspacecast i32* [[JA]]

  //CHECK:[[K:%k.*]] = alloca i32,
  //CHECK:[[SIZE:%size.*]] = alloca i32,

  //HOST:[[IV_I:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG:[[IV_IA:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG-NEXT:[[IV_I:%.*ascast]] = addrspacecast i64* [[IV_IA]]

  //HOST:[[IV_J:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG:[[IV_JA:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG-NEXT:[[IV_J:%.*ascast]] = addrspacecast i64* [[IV_JA]]

  //HOST:[[LB_I:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG:[[LB_IA:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG-NEXT:[[LB_I:%.*.ascast]] = addrspacecast i64* [[LB_IA]]

  //HOST:[[UB_I:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG:[[UB_IA:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG-NEXT:[[UB_I:%.*.ascast]] = addrspacecast i64* [[UB_IA]]

  //HOST:[[LB_J:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG:[[LB_JA:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG-NEXT:[[LB_J:%.*.ascast]] = addrspacecast i64* [[LB_JA]]

  //HOST:[[UB_J:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG:[[UB_JA:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG-NEXT:[[UB_J:%.*.ascast]] = addrspacecast i64* [[UB_JA]]

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK: "DIR.OMP.TEAMS"()
  // Setup LBs and UBs
  //CHECK: store i64 0, {{.*}}[[LB_I]]
  //CHECK: store {{.*}}[[UB_I]]
  //CHECK: store i64 0, {{.*}}[[LB_J]]
  //CHECK: store {{.*}}[[UB_J]]
  //CHECK: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i64[[AS]]* [[IV_I]], i64[[AS]]* [[IV_J]])
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i64[[AS]]* [[UB_I]], i64[[AS]]* [[UB_J]])
  // Init the first iteration variable
  //CHECK: [[L22:%[0-9]*]] = load i64, i64[[AS]]* [[LB_I]]
  //CHECK: store i64 [[L22]], i64[[AS]]* [[IV_I]]
  // Check first condition
  //CHECK: [[L23:%[0-9]*]] = load i64, i64[[AS]]* [[IV_I]], align 8
  //CHECK: [[L24:%[0-9]*]] = load i64, i64[[AS]]* [[UB_I]], align 8
  //CHECK: icmp sle i64 [[L23]], [[L24]]
  // Init the second iteration variable
  //CHECK: [[L25:%[0-9]*]] = load i64, i64[[AS]]* [[LB_J]]
  //CHECK: store i64 [[L25]], i64[[AS]]* [[IV_J]]
  // Check second condition
  //CHECK: [[L26:%[0-9]*]] = load i64, i64[[AS]]* [[IV_J]], align 8
  //CHECK: [[L27:%[0-9]*]] = load i64, i64[[AS]]* [[UB_J]], align 8
  //CHECK: icmp sle i64 [[L26]], [[L27]]
  // Loop Body
  // Set original loop variables
  //CHECK: load i64, i64[[AS]]* [[IV_I]]
  //CHECK: store i32 {{.*}}[[I]]
  //CHECK: load i64, i64[[AS]]* [[IV_J]]
  //CHECK: store i32 {{.*}}[[J]]

  //CHECK: "DIR.OMP.SIMD"()
  //CHECK: {{call|invoke}}{{.*}}foo
  //CHECK: "DIR.OMP.END.SIMD"()

  // Inner Inc
  //CHECK: [[L47:%[0-9]*]] = load i64, i64[[AS]]* [[IV_J]]
  //CHECK: [[ADD54:%add[0-9]*]] = add nsw i64 [[L47]], 1
  //CHECK: store i64 [[ADD54]], i64[[AS]]* [[IV_J]]
  // Outer Inc
  //CHECK: [[L48:%[0-9]*]] = load i64, i64[[AS]]* [[IV_I]]
  //CHECK: [[ADD56:%add[0-9]*]] = add nsw i64 [[L48]], 1
  //CHECK: store i64 [[ADD56]], i64[[AS]]* [[IV_I]]
  //CHECK: "DIR.OMP.END.DISTRIBUTE.PARLOOP"()
  //CHECK: "DIR.OMP.END.TEAMS"()
  //CHECK: "DIR.OMP.END.TARGET"()

  #pragma omp target map(alloc:A0[0:size], A1[0:size])
  #pragma omp teams distribute parallel for collapse(2)
  for (i=0;i<Ni;i++) {
    for (j=0;j<Nj;j++) {
      #pragma omp simd
      for (k=0;k<Nk;k++) {
        foo(A0,A1,i,j,k,Ni,Nj,Nk);
      }
    }
  }
}

// CHECK-LABEL: test_two
void test_two(float *A0, float *A1, int Ni, int Nj, int Nk) {
  int i, j, k;
  int size=Ni*Nj*Nk;

  //HOST:[[A0:%A0.addr]] = alloca float[[AS:.*]]*, align 8
  //TARG:[[A0A:%A0.addr]] = alloca float[[AS:.*]]*, align 8
  //TARG-NEXT:[[A0:%.*.ascast]] = addrspacecast float

  //CHECK:[[A1:%A1.addr]] = alloca float
  //CHECK:[[NI:%Ni.*]] = alloca i32,
  //CHECK:[[NJ:%Nj.*]] = alloca i32,
  //CHECK:[[NK:%Nk.*]] = alloca i32,

  //HOST:[[I:%i.*]] = alloca i32,
  //TARG:[[IA:%i.*]] = alloca i32,
  //TARG-NEXT:[[I:%.*.ascast]] = addrspacecast i32* [[IA]]

  //HOST:[[J:%j.*]] = alloca i32,
  //TARG:[[JA:%j.*]] = alloca i32,
  //TARG-NEXT:[[J:%.*.ascast]] = addrspacecast i32* [[JA]]

  //HOST:[[K:%k.*]] = alloca i32,
  //TARG:[[KA:%k.*]] = alloca i32,
  //TARG-NEXT:[[K:%.*.ascast]] = addrspacecast i32* [[KA]]

  //CHECK:[[SIZE:%size.*]] = alloca i32,

  //HOST:[[IV_I:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG:[[IV_IA:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG-NEXT:[[IV_I:%.*ascast]] = addrspacecast i64* [[IV_IA]]

  //HOST:[[IV_J:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG:[[IV_JA:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG-NEXT:[[IV_J:%.*ascast]] = addrspacecast i64* [[IV_JA]]

  //HOST:[[IV_K:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG:[[IV_KA:%.omp.uncollapsed.iv.*]] = alloca i64,
  //TARG-NEXT:[[IV_K:%.*ascast]] = addrspacecast i64* [[IV_KA]]

  //HOST:[[LB_I:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG:[[LB_IA:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG-NEXT:[[LB_I:%.*.ascast]] = addrspacecast i64* [[LB_IA]]

  //HOST:[[UB_I:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG:[[UB_IA:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG-NEXT:[[UB_I:%.*.ascast]] = addrspacecast i64* [[UB_IA]]

  //HOST:[[LB_J:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG:[[LB_JA:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG-NEXT:[[LB_J:%.*.ascast]] = addrspacecast i64* [[LB_JA]]

  //HOST:[[UB_J:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG:[[UB_JA:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG-NEXT:[[UB_J:%.*.ascast]] = addrspacecast i64* [[UB_JA]]

  //HOST:[[LB_K:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG:[[LB_KA:%.omp.uncollapsed.lb.*]] = alloca i64,
  //TARG-NEXT:[[LB_K:%.*.ascast]] = addrspacecast i64* [[LB_KA]]

  //HOST:[[UB_K:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG:[[UB_KA:%.omp.uncollapsed.ub.*]] = alloca i64,
  //TARG-NEXT:[[UB_K:%.*.ascast]] = addrspacecast i64* [[UB_KA]]

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK: "DIR.OMP.TEAMS"()
  // Setup LBs and UBs
  //CHECK: store i64 0, {{.*}}[[LB_I]]
  //CHECK: store {{.*}}[[UB_I]]
  //CHECK: store i64 0, {{.*}}[[LB_J]]
  //CHECK: store {{.*}}[[UB_J]]
  //CHECK: store i64 0, {{.*}}[[LB_K]]
  //CHECK: store {{.*}}[[UB_K]]
  //CHECK: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i64[[AS]]* [[IV_I]], i64[[AS]]* [[IV_J]], i64[[AS]]* [[IV_K]])
  //CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i64[[AS]]* [[UB_I]], i64[[AS]]* [[UB_J]], i64[[AS]]* [[UB_K]])
  //CHECK: "DIR.OMP.SIMD"()
  // Init the first iteration variable
  //CHECK: [[L22:%[0-9]*]] = load i64, i64[[AS]]* [[LB_I]]
  //CHECK: store i64 [[L22]], i64[[AS]]* [[IV_I]]
  // Check first condition
  //CHECK: [[L23:%[0-9]*]] = load i64, i64[[AS]]* [[IV_I]], align 8
  //CHECK: [[L24:%[0-9]*]] = load i64, i64[[AS]]* [[UB_I]], align 8
  //CHECK: icmp sle i64 [[L23]], [[L24]]
  // Init the second iteration variable
  //CHECK: [[L25:%[0-9]*]] = load i64, i64[[AS]]* [[LB_J]]
  //CHECK: store i64 [[L25]], i64[[AS]]* [[IV_J]]
  // Check second condition
  //CHECK: [[L26:%[0-9]*]] = load i64, i64[[AS]]* [[IV_J]], align 8
  //CHECK: [[L27:%[0-9]*]] = load i64, i64[[AS]]* [[UB_J]], align 8
  //CHECK: icmp sle i64 [[L26]], [[L27]]
  // Init the third iteration variable
  //CHECK: [[L3A:%[0-9]*]] = load i64, i64[[AS]]* [[LB_K]]
  //CHECK: store i64 [[L3A]], i64[[AS]]* [[IV_K]]
  // Check third condition
  //CHECK: [[L3B:%[0-9]*]] = load i64, i64[[AS]]* [[IV_K]], align 8
  //CHECK: [[L3C:%[0-9]*]] = load i64, i64[[AS]]* [[UB_K]], align 8
  //CHECK: icmp sle i64 [[L3B]], [[L3C]]


  // Loop Body
  // Set original loop variables
  //CHECK: load i64, i64[[AS]]* [[IV_I]]
  //CHECK: store i32 {{.*}}[[I]]
  //CHECK: load i64, i64[[AS]]* [[IV_J]]
  //CHECK: store i32 {{.*}}[[J]]
  //CHECK: load i64, i64[[AS]]* [[IV_K]]
  //CHECK: store i32 {{.*}}[[K]]
  //CHECK: {{call|invoke}}{{.*}}foo

  // Inner Inc
  //CHECK: [[L47:%[0-9]*]] = load i64, i64[[AS]]* [[IV_K]]
  //CHECK: [[ADD54:%add[0-9]*]] = add nsw i64 [[L47]], 1
  //CHECK: store i64 [[ADD54]], i64[[AS]]* [[IV_K]]
  // Next Inc
  //CHECK: [[L48:%[0-9]*]] = load i64, i64[[AS]]* [[IV_J]]
  //CHECK: [[ADD56:%add[0-9]*]] = add nsw i64 [[L48]], 1
  //CHECK: store i64 [[ADD56]], i64[[AS]]* [[IV_J]]
  // Outer Inc
  //CHECK: [[LI1:%[0-9]*]] = load i64, i64[[AS]]* [[IV_I]]
  //CHECK: [[ADDL:%add[0-9]*]] = add nsw i64 [[LI1]], 1
  //CHECK: store i64 [[ADDL]], i64[[AS]]* [[IV_I]]
  //CHECK: "DIR.OMP.END.SIMD"()
  //CHECK: "DIR.OMP.END.DISTRIBUTE.PARLOOP"()
  //CHECK: "DIR.OMP.END.TEAMS"()
  //CHECK: "DIR.OMP.END.TARGET"()

  #pragma omp target map(alloc:A0[0:size], A1[0:size])
  #pragma omp teams distribute parallel for simd collapse(3)
  for (i=0;i<Ni;i++) {
    for (j=0;j<Nj;j++) {
      for (k=0;k<Nk;k++) {
        foo(A0,A1,i,j,k,Ni,Nj,Nk);
      }
    }
  }
}

// OFF-LABEL: test_three
void test_three(float *A0, float *A1, int Ni, int Nj, int Nk) {
  int i, j, k;
  int size=Ni*Nj*Nk;

  //No uncollapsed names expected
  //OFF-NOT: .omp.uncollapsed.
  //OFF:[[IV:%.omp.iv.*]] = alloca i64,
  //OFF-NOT: .omp.uncollapsed.

  #pragma omp target map(alloc:A0[0:size], A1[0:size])
  #pragma omp teams distribute parallel for simd collapse(3)
  for (i=0;i<Ni;i++) {
    for (j=0;j<Nj;j++) {
      for (k=0;k<Nk;k++) {
        foo(A0,A1,i,j,k,Ni,Nj,Nk);
      }
    }
  }
}
