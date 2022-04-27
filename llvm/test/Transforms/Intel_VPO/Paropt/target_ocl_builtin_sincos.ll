; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s
;
; This test checks that VPOParopt translates the calls
;   sincosf(a,&s,&c)   into   s = @_Z18__spirv_ocl_sincosfPf(a,&c)
;   sincos (a,&s,&c)   into   s = @_Z18__spirv_ocl_sincosdPd(a,&c)

;
; #include <stdio.h>
; #include <math.h>
; void foo() {
;   #pragma omp target
;   {
;     float  fs, fc;
;     double ds, dc;
;     ::sincosf(2.0f, &fs, &fc);
;     ::sincos (3.0,  &ds, &dc);
;   }
; }
;
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: convergent mustprogress noinline nounwind optnone
define hidden spir_func void @_Z3foov() #0 {
entry:
  %fs = alloca float, align 4
  %fc = alloca float, align 4
  %ds = alloca double, align 8
  %dc = alloca double, align 8
  %fs.ascast = addrspacecast float* %fs to float addrspace(4)*
  %fc.ascast = addrspacecast float* %fc to float addrspace(4)*
  %ds.ascast = addrspacecast double* %ds to double addrspace(4)*
  %dc.ascast = addrspacecast double* %dc to double addrspace(4)*

  call spir_func void @sincosf(float noundef 2.000000e+00, float addrspace(4)* noundef %fs.ascast, float addrspace(4)* noundef %fc.ascast) #4
;
; CHECK: [[SINE0:%[^ ]+]] = call spir_func float @_Z18__spirv_ocl_sincosfPf(float 2.000000e+00, float addrspace(4)* %fc.ascast)
; CHECK: store float [[SINE0]], float addrspace(4)* %fs.ascast, align 4

  call spir_func void @sincos(double noundef 3.000000e+00, double addrspace(4)* noundef %ds.ascast, double addrspace(4)* noundef %dc.ascast) #4
;
; CHECK: [[SINE1:%[^ ]+]] = call spir_func double @_Z18__spirv_ocl_sincosdPd(double 3.000000e+00, double addrspace(4)* %dc.ascast)
; CHECK: store double [[SINE1]], double addrspace(4)* %ds.ascast, align 8

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent nounwind
declare spir_func void @sincosf(float noundef, float addrspace(4)* noundef, float addrspace(4)* noundef) #2
;
; CHECK: declare dso_local spir_func float @_Z18__spirv_ocl_sincosfPf(float, float addrspace(4)*)
;

; Function Attrs: convergent nounwind
declare spir_func void @sincos(double noundef, double addrspace(4)* noundef, double addrspace(4)* noundef) #3
;
; CHECK: declare dso_local spir_func double @_Z18__spirv_ocl_sincosdPd(double, double addrspace(4)*)
;

attributes #0 = { convergent mustprogress noinline nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { convergent nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "imf-precision"="medium" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN4vvv_sincosf,_ZGVcN8vvv_sincosf,_ZGVdN8vvv_sincosf,_ZGVeN16vvv_sincosf" }
attributes #3 = { convergent nounwind "approx-func-fp-math"="true" "frame-pointer"="all" "imf-precision"="medium" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-target-declare"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" "vector-variants"="_ZGVbN2vvv_sincos,_ZGVcN4vvv_sincos,_ZGVdN4vvv_sincos,_ZGVeN8vvv_sincos" }
attributes #4 = { convergent nounwind "imf-precision"="medium" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}
!opencl.used.extensions = !{!6}
!opencl.used.optional.core.features = !{!7}
!opencl.compiler.options = !{!6}

!0 = !{i32 0, i32 57, i32 -703114093, !"_Z3foov", i32 4, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"openmp", i32 50}
!3 = !{i32 7, !"openmp-device", i32 50}
!4 = !{i32 7, !"PIC Level", i32 2}
!5 = !{i32 7, !"frame-pointer", i32 2}
!6 = !{}
!7 = !{!"cl_doubles"}
