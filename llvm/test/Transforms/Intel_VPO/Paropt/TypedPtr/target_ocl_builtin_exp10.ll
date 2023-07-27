; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -switch-to-offload -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Check that we emit the device built-in math functions for:
;   double exp10()  -->  _Z17__spirv_ocl_exp10d
;   float exp10f()  -->  _Z17__spirv_ocl_exp10f
;
; // C test case
; #include <math.h>
; void bar(double,float);
; void foo() {
;   double dres;
;   float  fres;
;   #pragma omp target map(from:dres,fres)
;   {
;     dres = exp10(3.0);
;     fres = exp10f(3.0);
;   }
;   bar(dres,fres);
; }

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare spir_func double @exp10(double)
; CHECK: declare spir_func double @_Z17__spirv_ocl_exp10d(double)

declare spir_func float @exp10f(float)
; CHECK: declare spir_func float @_Z17__spirv_ocl_exp10f(float)

declare spir_func void @_Z3bardf(double, float)

define hidden spir_func void @_Z3foov() {
entry:
  %dres = alloca double, align 8
  %fres = alloca float, align 4
  %dres.ascast = addrspacecast double* %dres to double addrspace(4)*
  %fres.ascast = addrspacecast float* %fres to float addrspace(4)*
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.FROM"(double addrspace(4)* %dres.ascast, double addrspace(4)* %dres.ascast, i64 8, i64 34, i8* null, i8* null),
    "QUAL.OMP.MAP.FROM"(float addrspace(4)* %fres.ascast, float addrspace(4)* %fres.ascast, i64 4, i64 34, i8* null, i8* null) ]

  %call = call fast spir_func double @exp10(double 3.000000e+00)
;CHECK:   call fast spir_func double @_Z17__spirv_ocl_exp10d(double 3.000000e+00)

  store double %call, double addrspace(4)* %dres.ascast, align 8

  %call1 = call fast spir_func float @exp10f(float 3.000000e+00)
;CHECK:    call fast spir_func float @_Z17__spirv_ocl_exp10f(float 3.000000e+00)

  store float %call1, float addrspace(4)* %fres.ascast, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %1 = load double, double addrspace(4)* %dres.ascast, align 8
  %2 = load float, float addrspace(4)* %fres.ascast, align 4
  call spir_func void @_Z3bardf(double %1, float %2)
  ret void
}

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 73, i32 -703058349, !"_Z3foov", i32 6, i32 0, i32 0}
