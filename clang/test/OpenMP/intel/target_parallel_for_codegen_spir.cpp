//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN: | FileCheck %s --check-prefixes ALL,HOST

//RUN: %clang_cc1 -opaque-pointers -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN: | FileCheck %s --check-prefixes ALL,HOST

//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -opaque-pointers -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefixes ALL,TARG

//RUN: %clang_cc1 -opaque-pointers -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -opaque-pointers -triple spir \
//RUN:  -emit-llvm -disable-llvm-passes -DSPLIT \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefixes ALL,TARG

//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.ll %s

//RUN: %clang_cc1 -opaque-pointers -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.ll %s

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
//RUN:  | FileCheck %s --check-prefixes ALL,TARG

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
//RUN:  | FileCheck %s --check-prefixes ALL,TARG
//expected-no-diagnostics

void bar(int,int,...);

// The CodeGen for combined directives should be the same as the
// non-combined directives.

// ALL-LABEL: foo2
void foo2() {
  // ALL: [[I:%i.*]] = alloca i32,
  // ALL: [[J:%j.*]] = alloca i32,
  // ALL-DAG: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // ALL-DAG: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // ALL-DAG: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[I_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[I]] to ptr addrspace(4)
  // TARG: [[J_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[J]] to ptr addrspace(4)
  // TARG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)
  // TARG: [[OMP_IV_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_IV]] to ptr addrspace(4)
  int i;
  int j = 20;

  // HOST: store i32 0, ptr [[OMP_LB]],
  // TARG: store i32 0, ptr addrspace(4) [[OMP_LB_CAST]],
  // HOST: store i32 15, ptr [[OMP_UB]],
  // TARG: store i32 15, ptr addrspace(4) [[OMP_UB_CAST]],

  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[J]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[J_CAST]]
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[I_CAST]]
  // HOST-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[J]]
  // TARG-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) [[J_CAST]]
  // HOST-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV]]
  // TARG-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) [[OMP_IV_CAST]]
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: [[L1:%[0-9]+]] = load i32, ptr [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_IV_CAST]], align 4
  // HOST-NEXT: [[L2:%[0-9]+]] = load i32, ptr [[OMP_UB]], align 4
  // TARG-NEXT: [[L2:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_UB_CAST]], align 4
  // ALL-NEXT: icmp sle i32 [[L1]], [[L2]]
  // HOST: [[L1:%[0-9]+]] = load i32, ptr [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_IV_CAST]], align 4
  // HOST: store i32 {{.*}} ptr [[I]], align 4
  // TARG: store i32 {{.*}} ptr addrspace(4) [[I_CAST]], align 4
  // HOST: [[L2:%[0-9]+]] = load i32, ptr [[I]], align 4
  // TARG: [[L2:%[0-9]+]] = load i32, ptr addrspace(4) [[I_CAST]], align 4
  // HOST: [[L3:%[0-9]+]] = load i32, ptr [[J]], align 4
  // TARG: [[L3:%[0-9]+]] = load i32, ptr addrspace(4) [[J_CAST]], align 4
  // ALL-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // ALL-SAME: (i32 noundef 42, i32 noundef [[L2]], i32 noundef [[L3]])

#ifdef SPLIT
  #pragma omp target
  #pragma omp parallel for
#else
  #pragma omp target parallel for
#endif
  for(i=0;i<16;++i) {
    bar(42,i,j);
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// Hoist bounds from non-combined directives only in certain cases.

// Don't hoist if bounds are used in map clause
// ALL-LABEL: foo2_map
void foo2_map() {
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)

  int i;
  int j = 20;
  int sixteen = 16;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: store i32 0, ptr [[OMP_LB]],
  // TARG: store i32 0, ptr addrspace(4) [[OMP_LB_CAST]],
  // HOST: store i32 {{.*}}, ptr [[OMP_UB]],
  // TARG: store i32 {{.*}}, ptr addrspace(4) [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // ALL: {{call|invoke}}{{.*}}void {{.*}}bar

  #pragma omp target map(sixteen)
  #pragma omp parallel for
  for(i=0;i<sixteen;++i) {
    bar(42,i,j);
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// Don't hoist if bounds are used in is_device_ptr clause
// ALL-LABEL: foo2_is_device_ptr
void foo2_is_device_ptr() {
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)

  int i;
  int j = 20;
  int sixteen = 16;
  int *p_sixteen = &sixteen;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: store i32 0, ptr [[OMP_LB]],
  // TARG: store i32 0, ptr addrspace(4) [[OMP_LB_CAST]],
  // HOST: store i32 {{.*}}, ptr [[OMP_UB]],
  // TARG: store i32 {{.*}}, ptr addrspace(4) [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // ALL: {{call|invoke}}{{.*}}void {{.*}}bar

  #pragma omp target is_device_ptr(p_sixteen)
  #pragma omp parallel for
  for(i=0;i<*p_sixteen;++i) {
    bar(42,i,j);
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// Don't hoist if defaultmap clause is present
// ALL-LABEL: foo2_defaultmap
void foo2_defaultmap() {
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)

  int i;
  int j = 20;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: store i32 0, ptr [[OMP_LB]],
  // TARG: store i32 0, ptr addrspace(4) [[OMP_LB_CAST]],
  // HOST: store i32 {{.*}}, ptr [[OMP_UB]],
  // TARG: store i32 {{.*}}, ptr addrspace(4) [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // ALL: {{call|invoke}}{{.*}}void {{.*}}bar

  #pragma omp target defaultmap(tofrom:scalar)
  #pragma omp parallel for
  for(i=0;i<16;++i) {
    bar(42,i,j);
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// Don't hoist if global with same restrictions as autos.
#pragma omp declare target
int global_sixteen = 16;
#pragma omp end declare target
// ALL-LABEL: foo2_global
void foo2_global() {
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)

  int i;
  int j = 20;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: store i32 0, ptr [[OMP_LB]],
  // TARG: store i32 0, ptr addrspace(4) [[OMP_LB_CAST]],
  // HOST: store i32 {{.*}}, ptr [[OMP_UB]],
  // TARG: store i32 {{.*}}, ptr addrspace(4) [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // ALL: {{call|invoke}}{{.*}}void {{.*}}bar

  #pragma omp target map(tofrom:global_sixteen)
  #pragma omp parallel for
  for(i=0;i<global_sixteen;++i) {
    bar(42,i,j);
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// Also don't hoist combined loops if bounds are used in map clause
// ALL-LABEL: foo2_map_combined
void foo2_map_combined() {
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)

  int i;
  int j = 20;
  int sixteen = 16;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: store i32 0, ptr [[OMP_LB]],
  // TARG: store i32 0, ptr addrspace(4) [[OMP_LB_CAST]],
  // HOST: store i32 {{.*}}, ptr [[OMP_UB]],
  // TARG: store i32 {{.*}}, ptr addrspace(4) [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // ALL: {{call|invoke}}{{.*}}void {{.*}}bar

  #pragma omp target parallel for map(sixteen)
  for(i=0;i<sixteen;++i) {
    bar(42,i,j);
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// Hoisting ok for globals that are firstprivate (implicit/explicit).
#pragma omp declare target
int global_sixteen2 = 16;
#pragma omp end declare target
// ALL-LABEL: foo2_global2
void foo2_global2() {
  // ALL: [[OMP_LB1:%.omp.lb.*]] = alloca i32,
  // ALL: [[OMP_UB1:%.omp.ub.*]] = alloca i32,
  // TARG: [[OMP_LB1_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB1_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)

  int ii;
  int jj = 20;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_LB1]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB1_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB1]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB1_CAST]]
  // HOST: store i32 0, ptr [[OMP_LB1]],
  // TARG: store i32 0, ptr addrspace(4) [[OMP_LB1_CAST]],
  // HOST: store i32 {{.*}}, ptr [[OMP_UB1]],
  // TARG: store i32 {{.*}}, ptr addrspace(4) [[OMP_UB1_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // ALL: {{call|invoke}}{{.*}}void {{.*}}bar

  #pragma omp target
  #pragma omp parallel for
  for(ii=0;ii<global_sixteen2;++ii) {
    bar(42,ii,jj);
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// ALL-LABEL: foo3
void foo3() {
  // ALL: [[I:%i.*]] = alloca i32,
  // ALL: [[J:%j.*]] = alloca i32,
  // ALL: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[I_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[I]] to ptr addrspace(4)
  // TARG: [[J_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[J]] to ptr addrspace(4)
  // TARG: [[OMP_IV_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_IV]] to ptr addrspace(4)
  // TARG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)
  int i;
  int j = 20;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[J]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[J_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: store i32 0, ptr [[OMP_LB]],
  // TARG: store i32 0, ptr addrspace(4) [[OMP_LB_CAST]],
  // HOST: store i32 15, ptr [[OMP_UB]],
  // TARG: store i32 15, ptr addrspace(4) [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[I_CAST]]
  // HOST-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[J]]
  // TARG-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) [[J_CAST]]
  // HOST-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV]]
  // TARG-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) [[OMP_IV_CAST]]
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: [[L1:%[0-9]+]] = load i32, ptr [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_IV_CAST]], align 4
  // HOST-NEXT: [[L2:%[0-9]+]] = load i32, ptr [[OMP_UB]], align 4
  // TARG-NEXT: [[L2:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_UB_CAST]], align 4
  // ALL-NEXT: icmp sle i32 [[L1]], [[L2]]
  // HOST: [[L1:%[0-9]+]] = load i32, ptr [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_IV_CAST]], align 4
  // HOST: store i32 {{.*}} ptr [[I]], align 4
  // TARG: store i32 {{.*}} ptr addrspace(4) [[I_CAST]], align 4
  // HOST: [[L2:%[0-9]+]] = load i32, ptr [[I]], align 4
  // TARG: [[L2:%[0-9]+]] = load i32, ptr addrspace(4) [[I_CAST]], align 4
  // HOST: [[L3:%[0-9]+]] = load i32, ptr [[J]], align 4
  // TARG: [[L3:%[0-9]+]] = load i32, ptr addrspace(4) [[J_CAST]], align 4
  // ALL-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // ALL-SAME: (i32 noundef 42, i32 noundef [[L2]], i32 noundef [[L3]])

  // Check split with a block is not transformed.
  #pragma omp target
  {
    #pragma omp parallel for
    for(i=0;i<16;++i) {
      bar(42,i,j);
    }
    int q = 1;
  }
  // ALL: directive.region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  // ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}

// ALL-LABEL: foo4
void foo4(int n) {
  // ALL: [[I:%i.*]] = alloca i32,
  // ALL: [[J:%j.*]] = alloca i32,
  // ALL: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // TARG: [[I_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[I]] to ptr addrspace(4)
  // TARG: [[J_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[J]] to ptr addrspace(4)
  // TARG: [[OMP_IV_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_IV]] to ptr addrspace(4)
  // TARG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)
  int i;
  int j = 20;
  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[J]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[J_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: store i32 0, ptr [[OMP_LB]],
  // TARG: store i32 0, ptr addrspace(4) [[OMP_LB_CAST]],
  // HOST: store i32 {{.*}}, ptr [[OMP_UB]],
  // TARG: store i32 {{.*}}, ptr addrspace(4) [[OMP_UB_CAST]],
  // ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.PARALLEL.LOOP"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[I]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[I_CAST]]
  // HOST-SAME: "QUAL.OMP.SHARED:TYPED"(ptr [[J]]
  // TARG-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) [[J_CAST]]
  // HOST-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr [[OMP_IV]]
  // TARG-SAME: "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr addrspace(4) [[OMP_IV_CAST]]
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
  // HOST: [[L1:%[0-9]+]] = load i32, ptr [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_IV_CAST]], align 4
  // HOST-NEXT: [[L2:%[0-9]+]] = load i32, ptr [[OMP_UB]], align 4
  // TARG-NEXT: [[L2:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_UB_CAST]], align 4
  // ALL-NEXT: icmp sle i32 [[L1]], [[L2]]
  // HOST: [[L1:%[0-9]+]] = load i32, ptr [[OMP_IV]], align 4
  // TARG: [[L1:%[0-9]+]] = load i32, ptr addrspace(4) [[OMP_IV_CAST]], align 4
  // HOST: store i32 {{.*}} ptr [[I]], align 4
  // TARG: store i32 {{.*}} ptr addrspace(4) [[I_CAST]], align 4
  // HOST: [[L2:%[0-9]+]] = load i32, ptr [[I]], align 4
  // TARG: [[L2:%[0-9]+]] = load i32, ptr addrspace(4) [[I_CAST]], align 4
  // HOST: [[L3:%[0-9]+]] = load i32, ptr [[J]], align 4
  // TARG: [[L3:%[0-9]+]] = load i32, ptr addrspace(4) [[J_CAST]], align 4
  // ALL-NEXT: {{call|invoke}}{{.*}}void {{.*}}bar
  // ALL-SAME: (i32 noundef 42, i32 noundef [[L2]], i32 noundef [[L3]])

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
  int i,k;
  // ALL: [[OMP_LB:%.omp.lb.*]] = alloca i32,
  // ALL: [[OMP_UB:%.omp.ub.*]] = alloca i32,
  // ALL: [[OMP_IV:%.omp.iv.*]] = alloca i32,
  // TARG: [[K_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[K]] to ptr addrspace(4)
  // TARG: [[OMP_LB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_LB]] to ptr addrspace(4)
  // TARG: [[OMP_UB_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[OMP_UB]] to ptr addrspace(4)

  // HOST: store i32 0, ptr [[OMP_LB]],
  // HOST: store i32 1023, ptr [[OMP_UB]],

  // ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  // ALL-SAME: "DIR.OMP.TARGET"()
  // HOST-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr [[K]]
  // TARG-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[K_CAST]]
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OMP_LB]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[OMP_LB_CAST]]
  // HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[OMP_UB]]
  // TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[OMP_UB_CAST]]
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
