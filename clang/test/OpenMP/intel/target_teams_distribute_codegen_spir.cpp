//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t.out.1 %s

//RUN: %clang_cc1 -opaque-pointers -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t.out.1 %s

//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -opaque-pointers -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s

//RUN: %clang_cc1 -opaque-pointers -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -opaque-pointers -triple spir \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
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
  // CHECK: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // CHECK: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // CHECK: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // CHECK: [[I_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[I]] to ptr addrspace(4)
  // CHECK: [[J_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[J]] to ptr addrspace(4)
  // CHECK: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // CHECK: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)
  // CHECK: [[OMP_IV_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_IV]] to ptr addrspace(4)

  // CHECK: store i32 0, ptr addrspace(4) [[OMP_LB_CAST]],
  // CHECK: store i32 15, ptr addrspace(4) [[OMP_UB_CAST]],
  int i;
  int j = 20;
  // CHECK: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TARGET"()
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[J_CAST]]
  // CHECK: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.TEAMS"()
  // CHECK: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  // CHECK-SAME: "DIR.OMP.DISTRIBUTE"()
  // CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[I_CAST]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) [[OMP_IV_CAST]]
  // CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // CHECK-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // CHECK: [[L1:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_IV_CAST]], align 4
  // CHECK-NEXT: [[L2:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_UB_CAST]], align 4
  // CHECK-NEXT: icmp sle i32 [[L1]], [[L2]]
  // CHECK: [[L1:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_IV_CAST]], align 4
  // CHECK: store i32 {{.*}} ptr addrspace(4) [[I_CAST]], align 4
  // CHECK: [[L2:%[0-9]+]] = load i32, ptr addrspace(4) [[I_CAST]], align 4
  // CHECK: [[L3:%[0-9]+]] = load i32, ptr addrspace(4) [[J_CAST]], align 4
  // CHECK-NEXT: {{call|invoke}} spir_func void {{.*}}bar
  // CHECK-SAME: (i32 noundef 42, i32 noundef [[L2]], i32 noundef [[L3]])

#ifdef SPLIT
  #pragma omp target
  #pragma omp teams
  #pragma omp distribute
#else
  #pragma omp target teams distribute
#endif
  for(i=0;i<16;++i) {
    bar(42,i,j);
  }
  // CHECK: directive.region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE"
  // CHECK: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  // CHECK: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}
