//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64,spir \
//RUN:  -fopenmp-late-outline -fintel-compatibility \
//RUN:  -Werror -Wsource-uses-openmp -o - %s \
//RUN:    | FileCheck %s --check-prefixes ALL,HOST

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

//RUN: %clang_cc1 -fopenmp -fintel-compatibility -fopenmp-late-outline \
//RUN:   -triple x86_64-unknown-linux-gnu -emit-pch %s -o %t

//RUN: %clang_cc1 -fopenmp -fintel-compatibility -fopenmp-late-outline \
//RUN:   -triple x86_64-unknown-linux-gnu -include-pch %t -emit-llvm %s -o - \
//RUN:   | FileCheck %s --check-prefixes ALL,HOST

//expected-no-diagnostics

#ifndef HEADER
#define HEADER

void foo_gpu(float *A, int dnum) { }

//HOST: define{{.*}}foo_base{{.*}}#[[FOOBASE:[0-9]*]]
#pragma omp declare variant(foo_gpu) \
    match(construct={target variant dispatch}, device={arch(gen)})
void foo_base(float *A, int dnum) {
}

// ALL-LABEL: caller2
void caller2(int n, float* x, int dnum)
{
  //ALL: [[DNUM:%dnum.*]] = alloca i32, align 4
  //TARG: [[DNUM_CAST:%[a-z.0-9]+]] = addrspacecast i32* [[DNUM]] to i32 addrspace(4)*

  #pragma omp target data map(tofrom:x[0:n]) \
                          use_device_ptr(x) device(dnum)
  {
    //ALL: [[T0:%[0-9]+]] = {{.*}}region.entry(){{.*}}"DIR.OMP.TARGET"()
    #pragma omp target
    {
      //HOST: [[L:%[0-9]+]] = load i32, i32* [[DNUM]]
      //TARG: [[L:%[0-9]+]] = load i32, i32 addrspace(4)* [[DNUM_CAST]]
      //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH
      //ALL-SAME: "QUAL.OMP.DEVICE"(i32 [[L]])
      #pragma omp target variant dispatch device(dnum)
      //ALL: call{{.*}}foo_base
      foo_base(x, dnum);  // <-- may call foo_base or foo_gpu
      //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"
    }
    //ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET"
  }
  {
    int m;
    int sizea, sizeb, sizec;
    float *a, *b, *c;
    //ALL: [[T0:%[0-9]+]] = {{.*}}region.entry(){{.*}}"DIR.OMP.TARGET.DATA"()
    //ALL: [[T1:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH
    //TARGET-SAME: "QUAL.OMP.USE_DEVICE_PTR"
    //TARGET-SAME: (float addrspace(4)* addrspace(4)* %a
    //TARGET-SAME: float addrspace(4)* addrspace(4)* %b
    //TARGET-SAME: float addrspace(4)* addrspace(4)* %c) ]
    //HOST-SAME: "QUAL.OMP.USE_DEVICE_PTR"(float** %a
    //HOST-SAME: float** %b{{.*}}, float** %c) ]
    #pragma omp target data map(tofrom:c[0:sizec]) map(to:a[0:sizea]) \
                            map(to:b[0:sizeb])
    {
      #pragma omp target variant dispatch  use_device_ptr(a,b,c)
      //ALL: call{{.*}}foo_base
      foo_base(a, m);
      //ALL: region.exit(token [[T1]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"

      //ALL: [[T2:%[0-9]+]] = {{.*}}region.entry(){{.*}}TARGET.VARIANT.DISPATCH
      //ALL-SAME: QUAL.OMP.NOWAIT
      #pragma omp target variant dispatch use_device_ptr(a,b,c) nowait
      //ALL: call{{.*}}foo_base
      foo_base(a, m);
      //ALL: region.exit(token [[T2]]) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"
      //ALL: directive.region.exit(token [[T0]]) [ "DIR.OMP.END.TARGET.DATA"
    }
  }
}
#endif // HEADER

//TARG: define{{.*}}foo_gpu
//TARG: define{{.*}}foo_base{{.*}}#[[FOOBASE:[0-9]*]]

//ALL: attributes #[[FOOBASE]] = {{.*}}"openmp-variant"=
//ALL-SAME:name:{{.*}}foo_gpu
//ALL-SAME:construct:target_variant_dispatch;arch:gen
