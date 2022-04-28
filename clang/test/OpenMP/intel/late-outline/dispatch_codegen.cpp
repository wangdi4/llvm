//RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN:    | FileCheck %s --check-prefixes ALL,HOST

//RUN: %clang_cc1 -opaque-pointers -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fopenmp-typed-clauses -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN:    | FileCheck %s --check-prefixes ALL,HOST

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

//RUN: %clang_cc1 -opaque-pointers -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
//RUN:   -triple x86_64-unknown-linux-gnu -emit-pch %s -o %t

//RUN: %clang_cc1 -opaque-pointers -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
//RUN:   -triple x86_64-unknown-linux-gnu -include-pch %t -emit-llvm %s -o - \
//RUN:   | FileCheck %s --check-prefixes ALL,NEW-ALL,HOST,NEW-HOST

//RUN: %clang_cc1 -opaque-pointers -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
//RUN:   -fno-openmp-new-depend-ir -triple x86_64-unknown-linux-gnu \
//RUN:   -emit-pch %s -o %t

//RUN: %clang_cc1 -opaque-pointers -fopenmp -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
//RUN:   -triple x86_64-unknown-linux-gnu -include-pch %t -emit-llvm %s -o - \
//RUN:   -fno-openmp-new-depend-ir | \
//RUN:   FileCheck %s --check-prefixes ALL,OLD-ALL,HOST,OLD-HOST

//expected-no-diagnostics

#ifndef HEADER
#define HEADER
struct Obj {
  int arr[2];
Obj& foo_vari(const Obj&);
#pragma omp declare variant(foo_vari) \
  match(construct={dispatch}, device={arch(gen)})
  Obj& operator=(const Obj&);
};


void foo_gpu(float *A, int dnum) { }

//HOST: define{{.*}}foo_base{{.*}}#[[FOOBASE:[0-9]*]]
#pragma omp declare variant(foo_gpu) \
    match(construct={target variant dispatch}, device={arch(gen)})
void foo_base(float *A, int dnum) { }

// ALL-LABEL: caller2
void caller2(int n, float* x, int dnum)
{
  //ALL: [[DNUM:%dnum.*]] = alloca i32, align 4
  //ALL: [[CAP:%.capture.*]] = alloca i32, align 4
  //ALL: [[CAP1:%.capture.*]] = alloca i8, align 1
  //ALL: [[CAP2:%.capture.*]] = alloca i8, align 1
  //ALL: [[CAP3:%.capture.*]] = alloca i8, align 1
  //TARG: [[DNUM_CAST:%[a-z.0-9]+]] = addrspacecast ptr [[DNUM]] to ptr addrspace(4)
  //TARG: [[CAP_CAST:%.capture_.*]] = addrspacecast ptr [[CAP]] to ptr addrspace(4)

  //TARG: [[CAP_CAST1:%.capture_.*]] = addrspacecast ptr [[CAP1]] to ptr addrspace(4)
  //TARG: [[CAP_CAST2:%.capture_.*]] = addrspacecast ptr [[CAP2]] to ptr addrspace(4)
  //TARG: [[CAP_CAST3:%.capture_.*]] = addrspacecast ptr [[CAP3]] to ptr addrspace(4)
  #pragma omp target data map(tofrom:x[0:n]) \
                          use_device_ptr(x) device(dnum)
  {
    //ALL: [[T0:%[0-9]+]] = {{.*}}region.entry(){{.*}}"DIR.OMP.TARGET"()
    #pragma omp target
    {
      //HOST: [[L:%[0-9]+]] = load i32, ptr [[DNUM]]
      //HOST: [[L1:%[0-9]+]] = load i32, ptr [[CAP]]
      //TARG: [[L:%[0-9]+]] = load i32, ptr addrspace(4) [[DNUM_CAST]]
      //TARG: [[L1:%[0-9]+]] = load i32, ptr addrspace(4) [[CAP_CAST]]
      //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
      //ALL-SAME: "QUAL.OMP.DEVICE"(i32 [[L1]])
      //ALL-NOT: "QUAL.OMP.FRISTPRIVATE:TYPED"
      #pragma omp dispatch device(dnum) firstprivate(x)
      //ALL: call{{.*}}foo_base{{.*}}QUAL.OMP.DISPATCH.CALL
      foo_base(x, dnum);  // <-- may call foo_base or foo_gpu
      //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.DISPATCH"
    }
  }
  //ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  {
    int m;
    int sizea, sizeb, sizec;
    float *a, *b, *c;
    //ALL: [[T0:%[0-9]+]] = {{.*}}region.entry(){{.*}}"DIR.OMP.TARGET.DATA"()
    #pragma omp target data map(tofrom:c[0:sizec]) map(to:a[0:sizea]) \
                            map(to:b[0:sizeb])
    {
      //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
      #pragma omp dispatch
      //ALL: call{{.*}}foo_base
      foo_base(a, m);
      //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.DISPATCH"

      //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TASK
      //ALL-SAME: "QUAL.OMP.IMPLICIT"
      //HOST-SAME: "QUAL.OMP.SHARED:TYPED"(ptr %m
      //TARG-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %m.ascast
      //HOST-SAME: "QUAL.OMP.SHARED:TYPED"(ptr %a
      //TARG-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast
      //ALL: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
      //ALL-SAME: QUAL.OMP.NOWAIT
      #pragma omp dispatch nowait
      //ALL: call{{.*}}foo_base{{.*}}QUAL.OMP.DISPATCH.CALL
      foo_base(a, m);
      //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISPATCH"
      //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TASK"

      //NEW-ALL: [[DA:%[0-9]+]] = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0

      //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TASK
      //ALL-SAME: "QUAL.OMP.IMPLICIT"
      //NEW-HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %m
      //NEW-ALL-SAME: "QUAL.OMP.DEPARRAY"(i32 1, ptr [[DA]])
      //OLD-ALL-SAME: "QUAL.OMP.DEPEND.IN"
      //OLD-HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %m
      //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %m.ascast
      //HOST-SAME: "QUAL.OMP.SHARED:TYPED"(ptr %a
      //TARG-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.ascast
      //ALL: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
      //ALL-NOT: "QUAL.OMP.FIRSTPRIVATE:TYPED"
      //ALL-NOT: "QUAL.OMP.DEPEND.IN"
      #pragma omp dispatch depend(in: a) firstprivate(m)
      //ALL: call{{.*}}foo_base{{.*}}QUAL.OMP.DISPATCH.CALL
      foo_base(a, m);
      //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISPATCH"
      //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TASK"

      //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
      //HOST-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(ptr %a)
      //TARG-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(ptr addrspace(4) %a.ascast
      #pragma omp dispatch is_device_ptr(a)
      //ALL: call{{.*}}foo_base{{.*}}QUAL.OMP.DISPATCH.CALL
      foo_base(a, m);
      //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.DISPATCH"
    }
    //ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET.DATA"
  }
  {
     float *a;
     //ALL: [[T0:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET
     #pragma omp target
     for(int i = 0; i < 10; i++) {
       //ALL: [[FB:%frombool]] = zext i1 %cmp{{.*}} to i8
       //HOST: store i8 [[FB]], ptr [[CAP1]]
       //TARG: store i8 [[FB]], ptr addrspace(4) [[CAP_CAST1]]
       //HOST: [[L:%[0-9]+]] =  load i8, ptr [[CAP1]], align 1
       //TARG: [[L:%[0-9]+]] =  load i8, ptr addrspace(4) [[CAP_CAST1]], align 1
       //ALL: [[TB:%tobool]] = trunc i8 [[L]] to i1
       //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
       //ALL-SAME: "QUAL.OMP.NOVARIANTS"(i1 [[TB]])
       #pragma omp dispatch novariants(i < 5)
       //ALL: call{{.*}}foo_base{{.*}}QUAL.OMP.DISPATCH.CALL
       foo_base(a, dnum);
       //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.DISPATCH"
     }
     //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  }
  {
     float *a;
     //ALL: [[T0:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET
     #pragma omp target
     for(int i = 0; i < 10; i++) {
       //ALL: [[FB:%frombool[0-9]+]] = zext i1 %cmp{{.*}} to i8
       //HOST: store i8 [[FB]], ptr [[CAP2]]
       //TARG: store i8 [[FB]], ptr addrspace(4) [[CAP_CAST2]]
       //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TASK
       //ALL-SAME: "QUAL.OMP.IMPLICIT"()
       //HOST-SAME: "QUAL.OMP.SHARED:TYPED"(ptr %dnum.addr
       //TARG-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %dnum.addr.ascast
       //HOST-SAME: "QUAL.OMP.SHARED:TYPED"(ptr %a.map.ptr.tmp
       //TARG-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %a.map.ptr.tmp
       //HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[CAP2]]
       //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[CAP_CAST2]]
       //HOST: [[L:%[0-9]+]] =  load i8, ptr [[CAP2]], align 1
       //TARG: [[L:%[0-9]+]] =  load i8, ptr addrspace(4) [[CAP_CAST2]], align 1
       //ALL: [[TB:%tobool[0-9]+]] = trunc i8 [[L]] to i1
       //ALL: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
       //ALL-SAME: "QUAL.OMP.NOWAIT"
       //ALL-SAME: "QUAL.OMP.NOCONTEXT"(i1 [[TB]])
       #pragma omp dispatch nowait nocontext(i < 5)
       //ALL: call{{.*}}foo_base{{.*}}QUAL.OMP.DISPATCH.CALL
       foo_base(a, dnum);
       //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISPATCH"
       //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TASK"
       //ALL: [[FB1:%frombool[0-9]+]] = zext i1 %cmp{{.*}} to i8
       //HOST: store i8 [[FB1]], ptr [[CAP3]]
       //TARG: store i8 [[FB1]], ptr addrspace(4) [[CAP_CAST3]]
       //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TASK
       //ALL-SAME: "QUAL.OMP.IMPLICIT"()
       //HOST-SAME: "QUAL.OMP.SHARED:TYPED"(ptr %dnum.addr
       //TARG-SAME: "QUAL.OMP.SHARED:TYPED"(ptr addrspace(4) %dnum.addr.ascast
       //HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %a.map.ptr.tmp
       //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) %a.map.ptr.tmp
       //HOST-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr [[CAP3]]
       //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr addrspace(4) [[CAP_CAST3]]
       //HOST: [[L1:%[0-9]+]] =  load i8, ptr [[CAP3]], align 1
       //TARG: [[L1:%[0-9]+]] =  load i8, ptr addrspace(4) [[CAP_CAST3]], align 1
       //ALL: [[TB1:%tobool[0-9]+]] = trunc i8 [[L1]] to i1
       //ALL: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
       //ALL-SAME: "QUAL.OMP.NOWAIT"()
       //ALL-SAME: "QUAL.OMP.NOCONTEXT"(i1 [[TB1]])
       //HOST-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(ptr %a.map.ptr.tmp
       //TARG-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(ptr addrspace(4) %a.map.ptr.tmp
       #pragma omp dispatch nowait nocontext(i < 5) is_device_ptr(a)
       //ALL: call{{.*}}foo_base{{.*}}QUAL.OMP.DISPATCH.CALL
       foo_base(a, dnum);
       //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISPATCH"
       //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TASK"
     }
     //ALL: region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  }

  Obj o, o1;
  //ALL: [[T:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
  #pragma omp dispatch
  //ALL: call{{.*}}ZN3ObjaSERKS{{.*}}QUAL.OMP.DISPATCH.CAL
  o = o1;
  //ALL: region.exit(token [[T]]) [ "DIR.OMP.END.DISPATCH"
}

#endif // HEADER

//TARG: define{{.*}}foo_gpu
//TARG: define{{.*}}foo_base{{.*}}#[[FOOBASE:[0-9]*]]

//ALL: attributes #[[FOOBASE]] = {{.*}}"openmp-variant"=
//ALL-SAME:name:{{.*}}foo_gpu
//ALL-SAME:construct:target_variant_dispatch;arch:gen
