//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefix=HOST --check-prefix=ALL

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
//RUN:  | FileCheck %s --check-prefix=TARG-SPIR --check-prefix=ALL

//RUN: %clang_cc1 -triple spir \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefix=TARG-SPIR --check-prefix=ALL

#pragma omp declare target
int zzvar = 23;
#pragma omp end declare target

#pragma omp declare simd linear(d : 8)
void add_1(float *d) {}

void bar(int);

// ALL-LABEL: foo1
void foo1()
{
  int i;
  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target
  {
    bar(zzvar);
  }

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.PARALLEL.LOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target
  #pragma omp parallel for
  for(int i=0;i<10;++i) {
  }

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE"
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target teams distribute
  for (i=0;i<16;++i) {}

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE"
  //ALL: [[T3:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.SIMD"
  //ALL: region.exit(token [[T3]]) [ "DIR.OMP.END.SIMD"
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target teams distribute simd
  for (i=0;i<16;++i) {}

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.PARALLEL.LOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target parallel for
  for (i=0;i<16;++i) {}

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.PARALLEL.LOOP"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.SIMD"
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.SIMD"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target parallel for simd
  for (i=0;i<16;++i) {}

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target teams distribute parallel for
  for (i=0;i<16;++i) {}

  // Split case
  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target
  #pragma omp teams distribute parallel for
  for (i=0;i<16;++i) {}

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE.PARLOOP"
  //ALL: [[T3:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.SIMD"
  //ALL: region.exit(token [[T3]]) [ "DIR.OMP.END.SIMD"
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target teams distribute parallel for simd
  for (i=0;i<16;++i) {}

  // Split case
  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE.PARLOOP"
  //ALL: [[T3:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.SIMD"
  //ALL: region.exit(token [[T3]]) [ "DIR.OMP.END.SIMD"
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target
  #pragma omp teams distribute parallel for simd
  for (i=0;i<16;++i) {}

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  #pragma omp target
  {
    int i = 0;
    //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.ATOMIC"
    //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.ATOMIC"
    #pragma omp atomic
    i++;
    //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.BARRIER"
    //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.BARRIER"
    #pragma omp barrier

    //ALL: [[T3:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.MASTER"
    //ALL: region.exit(token [[T3]]) [ "DIR.OMP.END.MASTER"
    #pragma omp master
    {
    }

    //ALL: [[T4:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.SIMD"
    //ALL: region.exit(token [[T4]]) [ "DIR.OMP.END.SIMD"
    #pragma omp simd
    for (i=0;i<16;++i) {}
  }
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.SIMD"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.SIMD"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target
  #pragma omp simd
  for (i=0;i<16;++i) {}

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //HOST: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //HOST-SAME:"DIR.OMP.TEAMS"
  //HOST: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //HOST-SAME:"DIR.OMP.DISTRIBUTE.PARLOOP"
  //HOST: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  //HOST: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  /* Host compile allows, spir target allows target, ignores others, warns. */
  //expected-warning@+3 {{OpenMP directive 'teams' ignored for target}}
  //expected-warning@+3 {{OpenMP directive 'distribute parallel for' ignored for target}}
  #pragma omp target
  #pragma omp teams
  #pragma omp distribute parallel for
  for (i=0;i<16;++i) {}

  // Test warnings for some unsupported directives.

  //expected-warning@+2 {{OpenMP directive 'sections' ignored for target}}
  //expected-warning@+3 {{OpenMP directive 'section' ignored for}}
  #pragma omp sections
  {
    #pragma omp section
    {
    }
  }

  //expected-warning@+1 {{OpenMP directive 'parallel' ignored for target}}
  #pragma omp parallel
  {
    //expected-warning@+1 {{OpenMP directive 'critical' ignored for target}}
    #pragma omp critical
    {
    }
    //expected-warning@+1 {{OpenMP directive 'single' ignored for target}}
    #pragma omp single
    {
    }
    //expected-warning@+1 {{OpenMP directive 'task' ignored for target}}
    #pragma omp task
    {
    }
  }
}

// ALL-LABEL: foo2
void foo2(int s, int *a)
{
  int i;
  int size = s;

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET.DATA"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TARGET"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET.DATA"
  #pragma omp target data map(a[0:size])
  #pragma omp target teams distribute parallel for shared(size)
  for (i = 0; i < size; ++i) {
  }

  int dt;
  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET.ENTER.DATA"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET.ENTER.DATA"
  #pragma omp target enter data map(to:dt)

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET.UPDATE
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET.UPDATE"
  #pragma omp target update from(dt)

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET.EXIT.DATA"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET.EXIT.DATA"
  #pragma omp target exit data map(from:dt)
}

struct ios_base {
  typedef void (*event_callback) (ios_base& __b);
  struct _Callback_list {
    event_callback _M_fn;
  };
  struct _Callback_list* Lst;
};

ios_base *Obj;
void bar(...);
void execute_offload () {
//TARG-SPIR: [[IBASE:%ibase.*]] = alloca i32,
   bar(Obj);
//TARG-SPIR: DIR.OMP.TARGET
//TARG-SPIR-SAME: "QUAL.OMP.PRIVATE"(i32* [[IBASE]])
//TARG-SPIR: DIR.OMP.END.TARGET
   #pragma omp target
       int ibase = 3;
}
