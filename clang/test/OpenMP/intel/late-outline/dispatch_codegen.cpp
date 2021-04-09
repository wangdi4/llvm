//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN:    | FileCheck %s --check-prefixes ALL,HOST

//RUN: %clang_cc1 -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN:    | FileCheck %s --check-prefixes ALL,HOST

//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefixes ALL,TARG

//RUN: %clang_cc1 -triple i386-unknown-linux-gnu \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -verify -Wsource-uses-openmp -o - %s \
//RUN:  | FileCheck %s --check-prefixes ALL,TARG


//RUN: %clang_cc1 -fopenmp -fintel-compatibility -fopenmp-late-outline \
//RUN:   -triple x86_64-unknown-linux-gnu -emit-pch %s -o %t

//RUN: %clang_cc1 -fopenmp -fintel-compatibility -fopenmp-late-outline \
//RUN:   -triple x86_64-unknown-linux-gnu -include-pch %t -emit-llvm %s -o - \
//RUN:   | FileCheck %s --check-prefixes ALL,HOST

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
  //TARG: [[DNUM_CAST:%[a-z.0-9]+]] = addrspacecast i32* [[DNUM]] to i32 addrspace(4)*
  //ALL: [[CAP:%.capture.*]] = alloca i32, align 4
  //TARG: [[CAP_CAST:%.capture_.*]] = addrspacecast i32* [[CAP]] to i32 addrspace(4)*

  //ALL: [[CAP1:%.capture.*]] = alloca i8, align 1
  //TARG: [[CAP_CAST1:%.capture_.*]] = addrspacecast i8* [[CAP1]] to i8 addrspace(4)*
  //ALL: [[CAP2:%.capture.*]] = alloca i8, align 1
  //TARG: [[CAP_CAST2:%.capture_.*]] = addrspacecast i8* [[CAP2]] to i8 addrspace(4)*
  //ALL: [[CAP3:%.capture.*]] = alloca i8, align 1
  //TARG: [[CAP_CAST3:%.capture_.*]] = addrspacecast i8* [[CAP3]] to i8 addrspace(4)*
  #pragma omp target data map(tofrom:x[0:n]) \
                          use_device_ptr(x) device(dnum)
  {
    //ALL: [[T0:%[0-9]+]] = {{.*}}region.entry(){{.*}}"DIR.OMP.TARGET"()
    #pragma omp target
    {
      //HOST: [[L:%[0-9]+]] = load i32, i32* [[DNUM]]
      //HOST: [[L1:%[0-9]+]] = load i32, i32* [[CAP]]
      //TARG: [[L:%[0-9]+]] = load i32, i32 addrspace(4)* [[DNUM_CAST]]
      //TARG: [[L1:%[0-9]+]] = load i32, i32 addrspace(4)* [[CAP_CAST]]
      //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
      //ALL-SAME: "QUAL.OMP.DEVICE"(i32 [[L1]])
      //ALL-NOT: "QUAL.OMP.FRISTPRIVATE"
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
      //HOST-SAME: "QUAL.OMP.SHARED"(i32* %m
      //TARG-SAME: "QUAL.OMP.SHARED"(i32 addrspace(4)* %m.ascast
      //HOST-SAME: "QUAL.OMP.SHARED"(float** %a
      //TARG-SAME: "QUAL.OMP.SHARED"(float addrspace(4)* addrspace(4)* %a.ascast
      //ALL: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
      //ALL-SAME: QUAL.OMP.NOWAIT
      #pragma omp dispatch nowait
      //ALL: call{{.*}}foo_base{{.*}}QUAL.OMP.DISPATCH.CALL
      foo_base(a, m);
      //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISPATCH"
      //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TASK"

      //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TASK
      //ALL-SAME: "QUAL.OMP.IMPLICIT"
      //ALL-SAME: "QUAL.OMP.DEPEND.IN"
      //HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* %m
      //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32 addrspace(4)* %m.ascast
      //HOST-SAME: "QUAL.OMP.SHARED"(float** %a
      //TARG-SAME: "QUAL.OMP.SHARED"(float addrspace(4)* addrspace(4)* %a.ascast
      //ALL: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
      //ALL-NOT: "QUAL.OMP.FIRSTPRIVATE"
      //ALL-NOT: "QUAL.OMP.DEPEND.IN"
      #pragma omp dispatch depend(in: a) firstprivate(m)
      //ALL: call{{.*}}foo_base{{.*}}QUAL.OMP.DISPATCH.CALL
      foo_base(a, m);
      //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.DISPATCH"
      //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TASK"

      //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
      //HOST-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(float** %a)
      //TARG-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(float addrspace(4)* addrspace(4)* %a.ascast
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
       //HOST: store i8 [[FB]], i8* [[CAP1]]
       //TARG: store i8 [[FB]], i8 addrspace(4)* [[CAP_CAST1]]
       //HOST: [[L:%[0-9]+]] =  load i8, i8* [[CAP1]], align 1
       //TARG: [[L:%[0-9]+]] =  load i8, i8 addrspace(4)* [[CAP_CAST1]], align 1
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
       //HOST: store i8 [[FB]], i8* [[CAP2]]
       //TARG: store i8 [[FB]], i8 addrspace(4)* [[CAP_CAST2]]
       //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TASK
       //ALL-SAME: "QUAL.OMP.IMPLICIT"()
       //HOST-SAME: "QUAL.OMP.SHARED"(i32* %dnum.addr)
       //TARG-SAME: "QUAL.OMP.SHARED"(i32 addrspace(4)* %dnum.addr.ascast)
       //HOST-SAME: "QUAL.OMP.SHARED"(float** %a.map.ptr.tmp
       //TARG-SAME: "QUAL.OMP.SHARED"(float addrspace(4)* addrspace(4)* %a.map.ptr.tmp
       //HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i8* [[CAP2]])
       //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i8 addrspace(4)* [[CAP_CAST2]])
       //HOST: [[L:%[0-9]+]] =  load i8, i8* [[CAP2]], align 1
       //TARG: [[L:%[0-9]+]] =  load i8, i8 addrspace(4)* [[CAP_CAST2]], align 1
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
       //HOST: store i8 [[FB1]], i8* [[CAP3]]
       //TARG: store i8 [[FB1]], i8 addrspace(4)* [[CAP_CAST3]]
       //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TASK
       //ALL-SAME: "QUAL.OMP.IMPLICIT"()
       //HOST-SAME: "QUAL.OMP.SHARED"(i32* %dnum.addr)
       //TARG-SAME: "QUAL.OMP.SHARED"(i32 addrspace(4)* %dnum.addr.ascast)
       //HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(float** %a.map.ptr.tmp
       //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(float addrspace(4)* addrspace(4)* %a.map.ptr.tmp
       //HOST-SAME: "QUAL.OMP.FIRSTPRIVATE"(i8* [[CAP3]])
       //TARG-SAME: "QUAL.OMP.FIRSTPRIVATE"(i8 addrspace(4)* [[CAP_CAST3]])
       //HOST: [[L1:%[0-9]+]] =  load i8, i8* [[CAP3]], align 1
       //TARG: [[L1:%[0-9]+]] =  load i8, i8 addrspace(4)* [[CAP_CAST3]], align 1
       //ALL: [[TB1:%tobool[0-9]+]] = trunc i8 [[L1]] to i1
       //ALL: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}DISPATCH
       //ALL-SAME: "QUAL.OMP.NOWAIT"()
       //ALL-SAME: "QUAL.OMP.NOCONTEXT"(i1 [[TB1]])
       //HOST-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(float** %a.map.ptr.tmp
       //TARG-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(float addrspace(4)* addrspace(4)* %a.map.ptr.tmp
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
