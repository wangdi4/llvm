//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN: | FileCheck %s --check-prefixes ALL,HOST

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
//RUN:  | FileCheck %s --check-prefixes ALL,TARG

//RUN: %clang_cc1 -triple spir \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefixes ALL,TARG

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
//RUN:  | FileCheck %s --check-prefixes ALL,TARG

//RUN: %clang_cc1 -triple spir \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefixes ALL,TARG
//expected-no-diagnostics

void bar(int,int,...);

// The CodeGen for combined directives should be the same as the
// non-combined directives.

// ALL-LABEL: foo2a
void foo2a() {
  // ALL: [[I:%i.*]] = alloca i32,
  // TARG: [[I_CAST:%[0-9]+]] = addrspacecast i32* [[I]] to i32 addrspace(4)*
  // ALL: [[J:%j.*]] = alloca i32,
  // TARG: [[J_CAST:%[0-9]+]] = addrspacecast i32* [[J]] to i32 addrspace(4)*
  // ALL: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // TARG: [[OMP_IV_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_IV]] to i32 addrspace(4)*
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_LB]] to i32 addrspace(4)*
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_UB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_UB]] to i32 addrspace(4)*
  int i;
  int j = 20;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_LB]]),
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_UB]]),
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[OMP_UB_CAST]]),
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[J_CAST]]
  // HOST: store i32 0, i32* [[OMP_LB]],
  // TARG: store i32 0, i32 addrspace(4)* [[OMP_LB_CAST]],
  // HOST: store i32 15, i32* [[OMP_UB]],
  // TARG: store i32 15, i32 addrspace(4)* [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // HOST-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]),
  // TARG-SAME: "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* [[OMP_IV_CAST]]),
  // HOST-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]),
  // TARG-SAME: "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* [[OMP_UB_CAST]]),
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[I]])
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[I_CAST]])
  // HOST-SAME: "QUAL.OMP.SHARED"(i32* [[J]]
  // TARG-SAME: "QUAL.OMP.SHARED"(i32 addrspace(4)* [[J_CAST]]
  // HOST: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // HOST-NEXT: [[L2:%[0-9]+]] = load i32, i32* [[OMP_UB]], align 4
  // TARG-NEXT: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_UB_CAST]], align 4
  // ALL-NEXT: icmp sle i32 [[L1]], [[L2]]
  // HOST: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // HOST: store i32 {{.*}} i32* [[I]], align 4
  // TARG: store i32 {{.*}} i32 addrspace(4)* [[I_CAST]], align 4
  // HOST: [[L2:%[0-9]+]] = load i32, i32* [[I]], align 4
  // TARG: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[I_CAST]], align 4
  // HOST: [[L3:%[0-9]+]] = load i32, i32* [[J]], align 4
  // TARG: [[L3:%[0-9]+]] = load i32, i32 addrspace(4)* [[J_CAST]], align 4
  // ALL-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // ALL-SAME: (i32 42, i32 [[L2]], i32 [[L3]])

  #pragma omp target
  #pragma omp parallel for
  for(i=0;i<16;++i) {
    bar(42,i,j);
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// ALL-LABEL: foo2b
void foo2b() {
  // ALL: [[I:%i.*]] = alloca i32,
  // ALL: [[J:%j.*]] = alloca i32,
  // TARG: [[J_CAST:%[0-9]+]] = addrspacecast i32* [[J]] to i32 addrspace(4)*
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_LB]] to i32 addrspace(4)*
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_UB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_UB]] to i32 addrspace(4)*
  // ALL: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // TARG: [[OMP_IV_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_IV]] to i32 addrspace(4)*
  int i;
  int j = 20;
  // HOST: store i32 0, i32* [[OMP_LB]],
  // TARG: store i32 0, i32 addrspace(4)* [[OMP_LB_CAST]],
  // HOST: store i32 15, i32* [[OMP_UB]],
  // TARG: store i32 15, i32 addrspace(4)* [[OMP_UB_CAST]],
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_UB]]),
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_UB_CAST]]),
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[J_CAST]]),
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // HOST-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]),
  // TARG-SAME: "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* [[OMP_IV_CAST]]),
  // HOST-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]),
  // TARG-SAME: "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* [[OMP_UB_CAST]]),
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[I]])
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[I_CAST]])
  // HOST-SAME: "QUAL.OMP.SHARED"(i32* [[J]]
  // TARG-SAME: "QUAL.OMP.SHARED"(i32 addrspace(4)* [[J_CAST]]
  // HOST: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // HOST-NEXT: [[L2:%[0-9]+]] = load i32, i32* [[OMP_UB]], align 4
  // TARG-NEXT: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_UB_CAST]], align 4
  // ALL-NEXT: icmp sle i32 [[L1]], [[L2]]
  // HOST: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // HOST: store i32 {{.*}} i32* [[I]], align 4
  // TARG: store i32 {{.*}} i32 addrspace(4)* [[I_CAST]], align 4
  // HOST: [[L2:%[0-9]+]] = load i32, i32* [[I]], align 4
  // TARG: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[I_CAST]], align 4
  // HOST: [[L3:%[0-9]+]] = load i32, i32* [[J]], align 4
  // TARG: [[L3:%[0-9]+]] = load i32, i32 addrspace(4)* [[J_CAST]], align 4
  // ALL-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // ALL-SAME: (i32 42, i32 [[L2]], i32 [[L3]])

  #pragma omp target parallel for
  for(i=0;i<16;++i) {
    bar(42,i,j);
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// ALL-LABEL: foo3
void foo3() {
  // ALL: [[I:%i.*]] = alloca i32,
  // TARG: [[I_CAST:%[0-9]+]] = addrspacecast i32* [[I]] to i32 addrspace(4)*
  // ALL: [[J:%j.*]] = alloca i32,
  // TARG: [[J_CAST:%[0-9]+]] = addrspacecast i32* [[J]] to i32 addrspace(4)*
  // ALL: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // TARG: [[OMP_IV_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_IV]] to i32 addrspace(4)*
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_LB]] to i32 addrspace(4)*
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_UB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_UB]] to i32 addrspace(4)*
  int i;
  int j = 20;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_LB]]),
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_UB]]),
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[OMP_UB_CAST]]),
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[J_CAST]]
  // HOST: store i32 0, i32* [[OMP_LB]],
  // TARG: store i32 0, i32 addrspace(4)* [[OMP_LB_CAST]],
  // HOST: store i32 15, i32* [[OMP_UB]],
  // TARG: store i32 15, i32 addrspace(4)* [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // HOST-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]),
  // TARG-SAME: "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* [[OMP_IV_CAST]]),
  // HOST-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]),
  // TARG-SAME: "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* [[OMP_UB_CAST]]),
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[I]])
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[I_CAST]])
  // HOST-SAME: "QUAL.OMP.SHARED"(i32* [[J]]
  // TARG-SAME: "QUAL.OMP.SHARED"(i32 addrspace(4)* [[J_CAST]]
  // HOST: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // HOST-NEXT: [[L2:%[0-9]+]] = load i32, i32* [[OMP_UB]], align 4
  // TARG-NEXT: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_UB_CAST]], align 4
  // ALL-NEXT: icmp sle i32 [[L1]], [[L2]]
  // HOST: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // HOST: store i32 {{.*}} i32* [[I]], align 4
  // TARG: store i32 {{.*}} i32 addrspace(4)* [[I_CAST]], align 4
  // HOST: [[L2:%[0-9]+]] = load i32, i32* [[I]], align 4
  // TARG: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[I_CAST]], align 4
  // HOST: [[L3:%[0-9]+]] = load i32, i32* [[J]], align 4
  // TARG: [[L3:%[0-9]+]] = load i32, i32 addrspace(4)* [[J_CAST]], align 4
  // ALL-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // ALL-SAME: (i32 42, i32 [[L2]], i32 [[L3]])

  // Check split with a block is not transformed.
  #pragma omp target
  {
    #pragma omp parallel for
    for(i=0;i<16;++i) {
      bar(42,i,j);
    }
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// ALL-LABEL: foo4
void foo4(int n) {
  // ALL: [[I:%i.*]] = alloca i32,
  // TARG: [[I_CAST:%[0-9]+]] = addrspacecast i32* [[I]] to i32 addrspace(4)*
  // ALL: [[J:%j.*]] = alloca i32,
  // TARG: [[J_CAST:%[0-9]+]] = addrspacecast i32* [[J]] to i32 addrspace(4)*
  // ALL: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // TARG: [[OMP_IV_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_IV]] to i32 addrspace(4)*
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_LB]] to i32 addrspace(4)*
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_UB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_UB]] to i32 addrspace(4)*
  int i;
  int j = 20;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_LB]]),
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[OMP_UB]]),
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[OMP_UB_CAST]]),
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[J]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[J_CAST]]
  // HOST: store i32 0, i32* [[OMP_LB]],
  // TARG: store i32 0, i32 addrspace(4)* [[OMP_LB_CAST]],
  // HOST: store i32 {{.*}}, i32* [[OMP_UB]],
  // TARG: store i32 {{.*}}, i32 addrspace(4)* [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // HOST-SAME: "QUAL.OMP.NORMALIZED.IV"(i32* [[OMP_IV]]),
  // TARG-SAME: "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* [[OMP_IV_CAST]]),
  // HOST-SAME: "QUAL.OMP.NORMALIZED.UB"(i32* [[OMP_UB]]),
  // TARG-SAME: "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* [[OMP_UB_CAST]]),
  // HOST-SAME: "QUAL.OMP.PRIVATE"(i32* [[I]])
  // TARG-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[I_CAST]])
  // HOST-SAME: "QUAL.OMP.SHARED"(i32* [[J]]
  // TARG-SAME: "QUAL.OMP.SHARED"(i32 addrspace(4)* [[J_CAST]]
  // HOST: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // HOST-NEXT: [[L2:%[0-9]+]] = load i32, i32* [[OMP_UB]], align 4
  // TARG-NEXT: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_UB_CAST]], align 4
  // ALL-NEXT: icmp sle i32 [[L1]], [[L2]]
  // HOST: [[L1:%[0-9]+]] = load i32, i32* [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // HOST: store i32 {{.*}} i32* [[I]], align 4
  // TARG: store i32 {{.*}} i32 addrspace(4)* [[I_CAST]], align 4
  // HOST: [[L2:%[0-9]+]] = load i32, i32* [[I]], align 4
  // TARG: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[I_CAST]], align 4
  // HOST: [[L3:%[0-9]+]] = load i32, i32* [[J]], align 4
  // TARG: [[L3:%[0-9]+]] = load i32, i32 addrspace(4)* [[J_CAST]], align 4
  // ALL-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // ALL-SAME: (i32 42, i32 [[L2]], i32 [[L3]])

  // Check split with a block that cannot be transformed.
  #pragma omp target
  {
    n++;
    #pragma omp parallel for
    for(i=0;i<16+n;++i) {
      bar(42,i,j);
    }
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// ALL-LABEL: foo5
void foo5(double *qq, int nq) {

  // Checks hoisting of outer loop but not inner loop.

  // ALL: [[I:%i.*]] = alloca i32,
  // ALL: [[K:%k.*]] = alloca i32,
  // TARG: [[K_CAST:%[0-9]+]] = addrspacecast i32* [[K]] to i32 addrspace(4)*
  int i,k;
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_LB]] to i32 addrspace(4)*
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_UB_CAST:%[0-9]+]] = addrspacecast i32* [[OMP_UB]] to i32 addrspace(4)*
  // ALL: [[OMP_IV:%.omp.iv.*]] = alloca i32,

  // HOST: store i32 0, i32* [[OMP_LB]],
  // HOST: store i32 1023, i32* [[OMP_UB]],

  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_LB]]),
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[OMP_UB]]),
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_UB_CAST]]),
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[K]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[K_CAST]]
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TEAMS"()
  // ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  #pragma omp target teams distribute parallel for map(qq[:0])
  for(k=0; k<1024; k++)
  {
    // ALL: [[T3:%[0-9]+]] = call token @llvm.directive.region.entry()
    // ALL-SAME: "DIR.OMP.SIMD"()
    #pragma omp simd
    for(i=0; i<nq; i++)
      qq[k*nq + i] = 0.0;
  // ALL: directive.region.exit(token [[T3]]) [ "DIR.OMP.END.SIMD"
  }
  // ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  // ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  // ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}
