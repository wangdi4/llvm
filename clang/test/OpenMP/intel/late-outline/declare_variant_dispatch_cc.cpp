// INTEL_COLLAB

// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc19.16.27045 \
// RUN:  -fms-compatibility -fopenmp -fopenmp-late-outline   \
// RUN:  -fopenmp-targets=spir64 -emit-llvm -o - -x c %s     \
// RUN:   | FileCheck %s

//expected-no-diagnostics

int v1(float, void *interop_obj);

#pragma omp declare variant (v1) \
   match(construct={target variant dispatch}, device={arch(gen)})
int __cdecl base1(float b);

int __cdecl v2(float, void *interop_obj);

#pragma omp declare variant (v2) \
   match(construct={target variant dispatch}, device={arch(gen)})
int base2(float b);

int __cdecl v3(float, void *interop_obj);

#pragma omp declare variant (v3) \
   match(construct={target variant dispatch}, device={arch(gen)})
int __cdecl base3(float b);

int v4(float, void *interop_obj);

#pragma omp declare variant (v4) \
   match(construct={target variant dispatch}, device={arch(gen)})
int base4(float b);

void ff() {
  #pragma omp target
  {
    #pragma omp target variant dispatch
    base1(1.0);
    #pragma omp target variant dispatch
    base2(2.0);
    #pragma omp target variant dispatch
    base3(3.0);
    #pragma omp target variant dispatch
    base4(4.0);
  }
}

//CHECK: declare {{.*}}@base1(float noundef) #[[B1:[0-9]*]]
//CHECK: declare {{.*}}@base2(float noundef) #[[B2:[0-9]*]]
//CHECK: declare {{.*}}@base3(float noundef) #[[B3:[0-9]*]]
//CHECK: declare {{.*}}@base4(float noundef) #[[B4:[0-9]*]]

//CHECK:attributes #[[B1]] {{.*}}"openmp-variant"="name:v1;construct:target_variant_dispatch;arch:gen"
//CHECK:attributes #[[B2]] {{.*}}"openmp-variant"="name:v2;construct:target_variant_dispatch;arch:gen"
//CHECK:attributes #[[B3]] {{.*}}"openmp-variant"="name:v3;construct:target_variant_dispatch;arch:gen"
//CHECK:attributes #[[B4]] {{.*}}"openmp-variant"="name:v4;construct:target_variant_dispatch;arch:gen"

// end INTEL_COLLAB
