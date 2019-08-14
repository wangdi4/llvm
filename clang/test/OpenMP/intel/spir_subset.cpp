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
    //HOST: atomicrmw add i32* %i{{.*}}monotonic
    //TARG-SPIR: call{{.*}}__atomic_load
    //TARG-SPIR: call{{.*}}__atomic_compare_exchange
    #pragma omp atomic
    i++;
    //ALL: [[T2:%[0-9]+]]{{.*}}region.entry(){{.*}}"DIR.OMP.BARRIER"
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
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  #pragma omp target
  #pragma omp teams
  #pragma omp distribute parallel for
  for (i=0;i<16;++i) {}

  #pragma omp sections
  {
    #pragma omp section
    {
    }
  }

  #pragma omp parallel sections
  {
    #pragma omp section
    {
    }
  }

  #pragma omp parallel
  {
    //expected-warning@+1 {{OpenMP directive 'critical' ignored for target}}
    #pragma omp critical
    {
    }
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
//TARG-SPIR: [[IBASE_CAST:%[a-z.0-9]+]] = addrspacecast i32* [[IBASE]] to i32 addrspace(4)*
   bar(Obj);
//TARG-SPIR: DIR.OMP.TARGET
//TARG-SPIR-SAME: "QUAL.OMP.PRIVATE"(i32 addrspace(4)* [[IBASE_CAST]])
//TARG-SPIR: DIR.OMP.END.TARGET
   #pragma omp target
       int ibase = 3;
}

void hp_func(int);

// ALL-LABEL: hp_bar
void hp_bar(int M, int N)
{
  int x = 0;

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE"
  #pragma omp target
  #pragma omp teams distribute
  for (int i = 0; i < N; ++i)
  {
    int myV = 0;
    //ALL: [[P0:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.PARALLEL"
    #pragma omp parallel
    {
      //ALL: [[M0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.MASTER"
      #pragma omp master
      hp_func(111);
      //ALL: region.exit(token [[M0]]) [ "DIR.OMP.END.MASTER"

      //ALL: [[S0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.SINGLE"
      #pragma omp single
      hp_func(222);
      //ALL: region.exit(token [[S0]]) [ "DIR.OMP.END.SINGLE"

      //ALL: [[F0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.LOOP"
      #pragma omp for
      for (int ii=0;ii<M;++ii) { }
      //ALL: region.exit(token [[F0]]) [ "DIR.OMP.END.LOOP"

      //ALL: [[F0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.LOOP"
      //ALL: [[S0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.SIMD"
      #pragma omp for simd
      for (int ii=0;ii<M;++ii) { }
      //ALL: region.exit(token [[S0]]) [ "DIR.OMP.END.SIMD"
      //ALL: region.exit(token [[F0]]) [ "DIR.OMP.END.LOOP"

      hp_func(333);

      //ALL: [[B0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.BARRIER"
      #pragma omp barrier
      //ALL: region.exit(token [[B0]]) [ "DIR.OMP.END.BARRIER"

      hp_func(444);

      //HOST: atomicrmw add i32* %myV{{.*}}monotonic
      //TARG-SPIR: call{{.*}}__atomic_load
      //TARG-SPIR: call{{.*}}__atomic_compare_exchange
      #pragma omp atomic
      myV += x;
    }
    //ALL: region.exit(token [[P0]]) [ "DIR.OMP.END.PARALLEL"

    hp_func(555);

    //ALL: [[P0:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.PARALLEL.LOOP"
    #pragma omp parallel for
    for (int ii=0;ii<M;++ii) { }
    //ALL: region.exit(token [[P0]]) [ "DIR.OMP.END.PARALLEL.LOOP"

    //ALL: [[P0:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.PARALLEL.LOOP"
    //ALL: [[S0:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.SIMD"
    #pragma omp parallel for simd
    for (int ii=0;ii<M;++ii) { }
    //ALL: region.exit(token [[S0]]) [ "DIR.OMP.END.SIMD"
    //ALL: region.exit(token [[P0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  }
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE.PARLOOP"
  #pragma omp target
  #pragma omp teams distribute parallel for
  for (int i = 0; i < N; ++i)
  {
    int myV = 0;

    //ALL: [[P0:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.PARALLEL"
    #pragma omp parallel
    {
      //ALL: [[S0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.SINGLE"
      #pragma omp single
      hp_func(222);
      //ALL: region.exit(token [[S0]]) [ "DIR.OMP.END.SINGLE"

      //ALL: [[F0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.LOOP"
      #pragma omp for
      for (int ii=0;ii<M;++ii) { }
      //ALL: region.exit(token [[F0]]) [ "DIR.OMP.END.LOOP"

      //ALL: [[F0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.LOOP"
      //ALL: [[S0:%[0-9]+]] = call token @llvm.directive.region.entry()
      //ALL-SAME:"DIR.OMP.SIMD"
      #pragma omp for simd
      for (int ii=0;ii<M;++ii) { }
      //ALL: region.exit(token [[S0]]) [ "DIR.OMP.END.SIMD"
      //ALL: region.exit(token [[F0]]) [ "DIR.OMP.END.LOOP"

      hp_func(333);

      //HOST: atomicrmw add i32* %myV{{.*}}monotonic
      //TARG-SPIR: call{{.*}}__atomic_load
      //TARG-SPIR: call{{.*}}__atomic_compare_exchange
      #pragma omp atomic
      myV += x;
    }
    //ALL: region.exit(token [[P0]]) [ "DIR.OMP.END.PARALLEL"

    hp_func(555);

    //ALL: [[L0:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.PARALLEL.LOOP"
    #pragma omp parallel for
    for (int ii=0;ii<M;++ii) { }
    //ALL: region.exit(token [[L0]]) [ "DIR.OMP.END.PARALLEL.LOOP"

    //ALL: [[L0:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.PARALLEL.LOOP"
    //ALL: [[S0:%[0-9]+]] = call token @llvm.directive.region.entry()
    //ALL-SAME:"DIR.OMP.SIMD"
    #pragma omp parallel for simd
    for (int ii=0;ii<M;++ii) { }
    //ALL: region.exit(token [[S0]]) [ "DIR.OMP.END.SIMD"
    //ALL: region.exit(token [[L0]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  }
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.DISTRIBUTE.PARLOOP"
  //ALL: [[S0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.SIMD"
  #pragma omp target
  #pragma omp teams distribute parallel for simd
  for (int i = 0; i < N; ++i)
  {
    hp_func(i);
  }
  //ALL: region.exit(token [[S0]]) [ "DIR.OMP.END.SIMD"
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISTRIBUTE.PARLOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.PARALLEL.LOOP"
  #pragma omp target
  #pragma omp teams
  #pragma omp parallel for
  for (int i=0;i<16;++i) {}
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T1:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TEAMS"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.PARALLEL.LOOP"
  #pragma omp target teams
  #pragma omp parallel for
  for (int i=0;i<16;++i) {}
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.PARALLEL.LOOP"
  //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TEAMS"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"

  //ALL: [[T0:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.TARGET"
  //ALL: [[T2:%[0-9]+]] = call token @llvm.directive.region.entry()
  //ALL-SAME:"DIR.OMP.PARALLEL"
  #pragma omp target parallel
  {
    hp_func(42);
  }
  //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.PARALLEL"
  //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
}
