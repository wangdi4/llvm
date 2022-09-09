; RUN: opt -switch-to-offload -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s --check-prefix=DEFAULT
; RUN: opt -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S <%s | FileCheck %s --check-prefix=DEFAULT
;
; RUN: opt -vpo-paropt-spirv-offload-rne -switch-to-offload -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s --check-prefix=RNE
; RUN: opt -vpo-paropt-spirv-offload-rne -switch-to-offload -passes='function(vpo-cfg-restructuring),vpo-paropt' -S <%s | FileCheck %s --check-prefix=RNE
;
; When compiling for SPIRV target offloading we translate the "round" function to the corresponding OCL counterpart.
; By default roundf/round are translated into _Z17__spirv_ocl_roundf   / _Z17__spirv_ocl_roundd
; Under a flag, they are translated into      _Z19__spirv_ocl_roundnef / _Z19__spirv_ocl_roundned

; // C++ source
; #include <math.h>
; void foo() {
;   #pragma omp target
;   {
;     float  fff = 1.23f;
;     double ddd = 1.23;
;     float  fresult = roundf(fff);
;     double dresult = round(ddd);
;   }
; }
;
; DEFAULT-NOT: _Z19__spirv_ocl_roundnef
; DEFAULT-NOT: _Z19__spirv_ocl_roundned
; DEFAULT: declare float @_Z17__spirv_ocl_roundf(float)
; DEFAULT: declare double @_Z17__spirv_ocl_roundd(double)
; DEFAULT: call fast float @_Z17__spirv_ocl_roundf(float %{{.*}})
; DEFAULT: call fast double @_Z17__spirv_ocl_roundd(double %{{.*}})
;
; RNE-NOT: _Z17__spirv_ocl_roundf
; RNE-NOT: _Z17__spirv_ocl_roundd
; RNE: declare float @_Z19__spirv_ocl_roundnef(float)
; RNE: declare double @_Z19__spirv_ocl_roundned(double)
; RNE: call fast float @_Z19__spirv_ocl_roundnef(float %{{.*}})
; RNE: call fast double @_Z19__spirv_ocl_roundned(double %{{.*}})
;
; ModuleID = 'lit.cpp'
source_filename = "lit.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline nounwind optnone
define protected spir_func void @_Z3foov() #0 {
entry:
  %fff = alloca float, align 4
  %ddd = alloca double, align 8
  %fresult = alloca float, align 4
  %dresult = alloca double, align 8
  %fff.ascast = addrspacecast ptr %fff to ptr addrspace(4)
  %ddd.ascast = addrspacecast ptr %ddd to ptr addrspace(4)
  %fresult.ascast = addrspacecast ptr %fresult to ptr addrspace(4)
  %dresult.ascast = addrspacecast ptr %dresult to ptr addrspace(4)
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %fff.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %ddd.ascast, double 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %fresult.ascast, float 0.000000e+00, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) %dresult.ascast, double 0.000000e+00, i32 1) ]

  store float 0x3FF3AE1480000000, ptr addrspace(4) %fff.ascast, align 4
  store double 1.230000e+00, ptr addrspace(4) %ddd.ascast, align 8
  %1 = load float, ptr addrspace(4) %fff.ascast, align 4
  %2 = call fast float @llvm.round.f32(float %1) #3
  store float %2, ptr addrspace(4) %fresult.ascast, align 4
  %3 = load double, ptr addrspace(4) %ddd.ascast, align 8
  %4 = call fast double @llvm.round.f64(double %3) #3
  store double %4, ptr addrspace(4) %dresult.ascast, align 8
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.round.f32(float) #2

; Function Attrs: nocallback nofree nosync nounwind readnone speculatable willreturn
declare double @llvm.round.f64(double) #2

attributes #0 = { convergent mustprogress noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nocallback nofree nosync nounwind readnone speculatable willreturn }
attributes #3 = { convergent }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 58, i32 -692239870, !"_Z3foov", i32 3, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
