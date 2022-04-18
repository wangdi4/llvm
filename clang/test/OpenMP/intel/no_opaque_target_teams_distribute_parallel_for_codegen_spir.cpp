//RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -no-opaque-pointers -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host_split.bc %s

//RUN: %clang_cc1 -no-opaque-pointers -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host_split.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes -DSPLIT2 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host_split2.bc %s

//RUN: %clang_cc1 -no-opaque-pointers -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT2 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host_split2.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//RUN: %clang_cc1 -no-opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes -DSPLIT3 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host_split2.bc %s

//RUN: %clang_cc1 -no-opaque-pointers -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT3 \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host_split2.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//expected-no-diagnostics

void bar(int,int,...);

// The CodeGen for combined directives should be the same as the
// non-combined directives.

// CHECK-LABEL: foo2
void foo2() {
  // CHECK: [[I:%i.*]] = alloca i32,
  // CHECK: [[J:%j.*]] = alloca i32,
  // CHECK-DAG: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // CHECK-DAG: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // CHECK-DAG: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[I_CAST:%[a-z.0-9]+]] = addrspacecast i32* [[I]] to i32 addrspace(4)*
  // CHECK: [[J_CAST:%[a-z.0-9]+]] = addrspacecast i32* [[J]] to i32 addrspace(4)*
  // CHECK-DAG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast i32* [[OMP_LB]] to i32 addrspace(4)*
  // CHECK-DAG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast i32* [[OMP_UB]] to i32 addrspace(4)*
  // CHECK-DAG: [[OMP_IV_CAST:%[a-z.0-9]+]] = addrspacecast i32* [[OMP_IV]] to i32 addrspace(4)*

  int i;
  int j = 20;
  // CHECK: store i32 0, i32 addrspace(4)* [[OMP_LB_CAST]],
  // CHECK: store i32 15, i32 addrspace(4)* [[OMP_UB_CAST]],
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[J_CAST]]
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_UB_CAST]]
  // CHECK: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TEAMS"()
  // CHECK: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.DISTRIBUTE.PARLOOP"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[I_CAST]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV"(i32 addrspace(4)* [[OMP_IV_CAST]]),
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* [[OMP_LB_CAST]]),
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB"(i32 addrspace(4)* [[OMP_UB_CAST]])
  // CHECK: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_UB_CAST]], align 4
  // CHECK-NEXT: icmp sle i32 [[L1]], [[L2]]
  // CHECK: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[OMP_IV_CAST]], align 4
  // CHECK: store i32 {{.*}} i32 addrspace(4)* [[I_CAST]], align 4
  // CHECK: [[L2:%[0-9]+]] = load i32, i32 addrspace(4)* [[I_CAST]], align 4
  // CHECK: [[L3:%[0-9]+]] = load i32, i32 addrspace(4)* [[J_CAST]], align 4
  // CHECK-NEXT: {{call|invoke}} spir_func void {{.*}}bar
  // CHECK-SAME: (i32 noundef 42, i32 noundef [[L2]], i32 noundef [[L3]])

#ifdef SPLIT
  #pragma omp target
  #pragma omp teams
  #pragma omp distribute parallel for
#elif defined(SPLIT2)
  #pragma omp target teams
  #pragma omp distribute parallel for
#elif defined(SPLIT3)
  #pragma omp target
  {
  #pragma omp teams distribute parallel for
#else
  #pragma omp target teams distribute parallel for
#endif
  for(i=0;i<16;++i) {
    bar(42,i,j);
  }
#ifdef SPLIT3
  }
#endif // SPLIT3

  // CHECK: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  // CHECK: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  // CHECK: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}
