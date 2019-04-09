//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s | FileCheck %s

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//RUN: %clang_cc1 -triple spir \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.ll %s

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//RUN: %clang_cc1 -triple spir \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s
//expected-no-diagnostics

void bar(int,int,...);

// The CodeGen for combined directives should be the same as the
// non-combined directives.

// CHECK-LABEL: foo2a
void foo2a() {
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  int i;
  int j = 20;
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_LB]]),
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_UB]]),
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J]]
  // CHECK: store i32 0, i32* [[OMP_LB]],
  // CHECK: store i32 15, i32* [[OMP_UB]],
  // CHECK: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]),
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[J]]
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = load i32, i32* [[OMP_UB]], align 4
  // CHECK-NEXT: icmp sle i32 [[L1]], [[L2]]
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // CHECK: store i32 {{.*}} i32* [[I]], align 4
  // CHECK: [[L2:%[0-9]+]] = load i32, i32* [[I]], align 4
  // CHECK: [[L3:%[0-9]+]] = load i32, i32* [[J]], align 4
  // CHECK-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // CHECK-SAME: (i32 42, i32 [[L2]], i32 [[L3]])

  #pragma omp target
  #pragma omp parallel for
  for(i=0;i<16;++i) {
    bar(42,i,j);
  }
  // CHECK: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// CHECK-LABEL: foo2b
void foo2b() {
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  int i;
  int j = 20;
  // CHECK: store i32 0, i32* [[OMP_LB]],
  // CHECK: store i32 15, i32* [[OMP_UB]],
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_UB]]),
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J]]
  // CHECK: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]),
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[J]]
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = load i32, i32* [[OMP_UB]], align 4
  // CHECK-NEXT: icmp sle i32 [[L1]], [[L2]]
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // CHECK: store i32 {{.*}} i32* [[I]], align 4
  // CHECK: [[L2:%[0-9]+]] = load i32, i32* [[I]], align 4
  // CHECK: [[L3:%[0-9]+]] = load i32, i32* [[J]], align 4
  // CHECK-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // CHECK-SAME: (i32 42, i32 [[L2]], i32 [[L3]])

  #pragma omp target parallel for
  for(i=0;i<16;++i) {
    bar(42,i,j);
  }
  // CHECK: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// CHECK-LABEL: foo3
void foo3() {
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  int i;
  int j = 20;
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_LB]]),
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_UB]]),
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J]]
  // CHECK: store i32 0, i32* [[OMP_LB]],
  // CHECK: store i32 15, i32* [[OMP_UB]],
  // CHECK: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]),
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[J]]
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = load i32, i32* [[OMP_UB]], align 4
  // CHECK-NEXT: icmp sle i32 [[L1]], [[L2]]
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // CHECK: store i32 {{.*}} i32* [[I]], align 4
  // CHECK: [[L2:%[0-9]+]] = load i32, i32* [[I]], align 4
  // CHECK: [[L3:%[0-9]+]] = load i32, i32* [[J]], align 4
  // CHECK-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // CHECK-SAME: (i32 42, i32 [[L2]], i32 [[L3]])

  // Check split with a block is not transformed.
  #pragma omp target
  {
    #pragma omp parallel for
    for(i=0;i<16;++i) {
      bar(42,i,j);
    }
  }
  // CHECK: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// CHECK-LABEL: foo4
void foo4(int n) {
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  int i;
  int j = 20;
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_LB]]),
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_UB]]),
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J]]
  // CHECK: store i32 0, i32* [[OMP_LB]],
  // CHECK: store i32 {{.*}}, i32* [[OMP_UB]],
  // CHECK: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]),
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[I]])
  // CHECK-SAME: "QUAL.OMP.SHARED"(i32* [[J]]
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = load i32, i32* [[OMP_UB]], align 4
  // CHECK-NEXT: icmp sle i32 [[L1]], [[L2]]
  // CHECK: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // CHECK: store i32 {{.*}} i32* [[I]], align 4
  // CHECK: [[L2:%[0-9]+]] = load i32, i32* [[I]], align 4
  // CHECK: [[L3:%[0-9]+]] = load i32, i32* [[J]], align 4
  // CHECK-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // CHECK-SAME: (i32 42, i32 [[L2]], i32 [[L3]])

  // Check split with a block that cannot be transformed.
  #pragma omp target
  {
    n++;
    #pragma omp parallel for
    for(i=0;i<16+n;++i) {
      bar(42,i,j);
    }
  }
  // CHECK: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// CHECK-LABEL: foo5
void foo5(double *qq, int nq) {

  // Checks hoisting of outer loop but not inner loop.

  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[K:%k.*]] = alloca i32,
  int i,k;
  // CHECK: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_IV:%.omp.iv.*]] = alloca i32,

  // CHECK: store i32 0, i32* [[OMP_LB]],
  // CHECK: store i32 1023, i32* [[OMP_UB]],

  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_UB]]),
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[K]]
  // CHECK: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TEAMS"()
  // CHECK: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  #pragma omp target teams distribute parallel for map(qq[:0])
  for(k=0; k<1024; k++)
  {
    // CHECK: [[T3:%[0-9]+]] = call token @llvm.directive.region.entry()
    // CHECK-SAME: "DIR.OMP.SIMD"()
    #pragma omp simd
    for(i=0; i<nq; i++)
      qq[k*nq + i] = 0.0;
  // CHECK: directive.region.exit(token [[T3]]) [ "DIR.OMP.END.SIMD"
  }
  // CHECK: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  // CHECK: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  // CHECK: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}
