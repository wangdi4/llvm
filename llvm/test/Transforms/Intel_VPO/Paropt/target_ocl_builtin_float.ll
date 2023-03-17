; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -opaque-pointers=0 -passes='vpo-paropt' -S %s | FileCheck %s

; This test checks that VPOParopt translates names of math functions (double)
; into OpenCL builtin names. This is needed for spir64 targets.
;
; List of functions:
; sin,cos,tan,pow,exp,log,ceil,floor,fabs,sqrt,log2,erf,fmax,fmin,
; asin,asinh,acos,acosh,atan,atanh,atan2
; To add new functions, add them to the 2 areas marked MANUALLY ADDED.

;
; #include <stdio.h>
; #include <mathimf.h>
; int main() {
;   double array[20];
;   #pragma omp target map(tofrom:array)
;   {
;      array[0] = sin(1.0);
;      array[1] = cos(1.0);
;      array[2] = tan(1.0);
;      array[3] = pow(2.0, 3.0);
;      array[4] = exp(2.0);
;      array[5] = log(2.0);
;      array[6] = ceil(2.5);
;      array[7] = floor(2.5);
;      array[8] = fabs(-2.0);
;      array[9] = sqrt(3.0);
;      array[10] = log2(3.0);
;      array[11] = erf(3.0);
;      array[12] = fmax(2.0, 3.0);
;      array[13] = fmin(2.0, 3.0);
;         ... other functions ...
;   }
;   for (int i = 0; i<14; i++)
;     printf("array[%d] = %lf\n", i, array[i]);
;   return 0;
; }

; ModuleID = '<stdin>'
source_filename = "target_ocl_builtin_float.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: nounwind
declare dso_local spir_func float @sinf(float) #1
; CHECK: declare dso_local spir_func float @sinf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @cosf(float) #1
; CHECK: declare dso_local spir_func float @cosf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @tanf(float) #1
; CHECK: declare dso_local spir_func float @tanf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @powf(float, float) #1
; CHECK: declare dso_local spir_func float @powf(float, float)

; Function Attrs: nounwind
declare dso_local spir_func float @expf(float) #1
; CHECK: declare dso_local spir_func float @expf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @logf(float) #1
; CHECK: declare dso_local spir_func float @logf(float)

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func float @llvm.ceil.f32(float) #6
; CHECK: declare dso_local spir_func float @llvm.ceil.f32(float)

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func float @llvm.floor.f32(float) #6
; CHECK: declare dso_local spir_func float @_Z17__spirv_ocl_floorf(float)

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func float @llvm.fabs.f32(float) #6
; CHECK: declare dso_local spir_func float @llvm.fabs.f32(float)

; Function Attrs: nounwind
declare dso_local spir_func float @sqrtf(float) #1
; CHECK: declare dso_local spir_func float @sqrtf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @log2f(float) #1
; CHECK: declare dso_local spir_func float @log2f(float)

; Function Attrs: nounwind
declare dso_local spir_func float @erff(float) #1
; CHECK: declare dso_local spir_func float @erff(float)

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func float @llvm.maxnum.f32(float, float) #6
; CHECK: declare dso_local spir_func float @_Z16__spirv_ocl_fmaxff(float, float)

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func float @llvm.minnum.f32(float, float) #6
; CHECK: declare dso_local spir_func float @_Z16__spirv_ocl_fminff(float, float)

; MANUALLY ADDED

; Function Attrs: nounwind
declare dso_local spir_func float @asinf(float) #1
; CHECK: declare dso_local spir_func float @asinf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @asinhf(float) #1
; CHECK: declare dso_local spir_func float @asinhf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @sinhf(float) #1
; CHECK: declare dso_local spir_func float @sinhf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @acosf(float) #1
; CHECK: declare dso_local spir_func float @acosf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @acoshf(float) #1
; CHECK: declare dso_local spir_func float @acoshf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @coshf(float) #1
; CHECK: declare dso_local spir_func float @coshf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @atanf(float) #1
; CHECK: declare dso_local spir_func float @atanf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @atanhf(float) #1
; CHECK: declare dso_local spir_func float @atanhf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @atan2f(float, float) #1
; CHECK: declare dso_local spir_func float @atan2f(float, float)

; Function Attrs: nounwind
declare dso_local spir_func float @tanhf(float) #1
; CHECK: declare dso_local spir_func float @tanhf(float)

; Function Attrs: nounwind
declare dso_local spir_func float @invsqrtf(float) #1
; CHECK: declare dso_local spir_func float @_Z17__spirv_ocl_rsqrtf(float)

; Function Attrs: noinline norecurse optnone uwtable
define dso_local spir_kernel void @__omp_offloading_fd02_d323ad_main_l46([20 x float] addrspace(1)* %array) #4 {
newFuncRoot:
  br label %for.end

DIR.OMP.END.TARGET.232.exitStub:                  ; preds = %DIR.OMP.END.TARGET.2
  ret void

for.end:                                          ; preds = %newFuncRoot
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %for.end
  %call = call spir_func float @sinf(float 1.000000e+00) #0
; CHECK: {{.*}}  call spir_func float @sinf
  %arrayidx1 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 0
  store float %call, float addrspace(1)* %arrayidx1, align 4
  %call2 = call spir_func float @cosf(float 1.000000e+00) #0
; CHECK: {{.*}}  call spir_func float @cosf
  %arrayidx3 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 1
  store float %call2, float addrspace(1)* %arrayidx3, align 4
  %call4 = call spir_func float @tanf(float 1.000000e+00) #0
; CHECK: {{.*}}  call spir_func float @tanf
  %arrayidx5 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 2
  store float %call4, float addrspace(1)* %arrayidx5, align 4
  %call6 = call spir_func float @powf(float 2.000000e+00, float 3.000000e+00) #0
; CHECK: {{.*}}  call spir_func float @powf
  %arrayidx7 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 3
  store float %call6, float addrspace(1)* %arrayidx7, align 4
  %call8 = call spir_func float @expf(float 2.000000e+00) #0
; CHECK: {{.*}}  call spir_func float @expf
  %arrayidx9 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 4
  store float %call8, float addrspace(1)* %arrayidx9, align 4
  %call10 = call spir_func float @logf(float 2.000000e+00) #0
; CHECK: {{.*}}  call spir_func float @logf
  %arrayidx11 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 5
  store float %call10, float addrspace(1)* %arrayidx11, align 4
  %call12 = call spir_func float @llvm.ceil.f32(float 2.500000e+00)
; CHECK: {{.*}}  call spir_func float @llvm.ceil.f32
  %arrayidx13 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 6
  store float %call12, float addrspace(1)* %arrayidx13, align 4
  %call14 = call spir_func float @llvm.floor.f32(float 2.500000e+00)
; CHECK: {{.*}}  call spir_func float @_Z17__spirv_ocl_floorf
  %arrayidx15 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 7
  store float %call14, float addrspace(1)* %arrayidx15, align 4
  %call16 = call spir_func float @llvm.fabs.f32(float -2.000000e+00)
; CHECK: {{.*}}  call spir_func float @llvm.fabs.f32
  %arrayidx17 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 8
  store float %call16, float addrspace(1)* %arrayidx17, align 4
  %call18 = call spir_func float @sqrtf(float 3.000000e+00) #0
; CHECK: {{.*}}  call spir_func float @sqrtf
  %arrayidx19 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 9
  store float %call18, float addrspace(1)* %arrayidx19, align 4
  %call20 = call spir_func float @log2f(float 3.000000e+00) #0
; CHECK: {{.*}}  call spir_func float @log2f
  %arrayidx21 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 10
  store float %call20, float addrspace(1)* %arrayidx21, align 4
  %call22 = call spir_func float @erff(float 3.000000e+00) #0
; CHECK: {{.*}}  call spir_func float @erff
  %arrayidx23 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 10
  store float %call22, float addrspace(1)* %arrayidx23, align 4
  %call24 = call spir_func float @llvm.maxnum.f32(float 2.000000e+00, float 3.000000e+00)
; CHECK: {{.*}} call spir_func float @_Z16__spirv_ocl_fmaxff
  %arrayidx25 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 12
  store float %call24, float addrspace(1)* %arrayidx25, align 8
  %call26 = call spir_func float @llvm.minnum.f32(float 2.000000e+00, float 3.000000e+00)
; CHECK: {{.*}} call spir_func float @_Z16__spirv_ocl_fminff
  %arrayidx27 = getelementptr inbounds [20 x float], [20 x float] addrspace(1)* %array, i64 0, i64 13

; MANUALLY ADDED
; Since there's no dead store elimination in this test, use the same address.

  %call27 = call spir_func float @asinf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @asinf
  store float %call27, float addrspace(1)* %arrayidx25, align 8

  %call28 = call spir_func float @asinhf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @asinhf
  store float %call28, float addrspace(1)* %arrayidx25, align 8

  %call29 = call spir_func float @sinhf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @sinhf
  store float %call29, float addrspace(1)* %arrayidx25, align 8

  %call30 = call spir_func float @acosf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @acosf
  store float %call30, float addrspace(1)* %arrayidx25, align 8

  %call31 = call spir_func float @acoshf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @acoshf
  store float %call31, float addrspace(1)* %arrayidx25, align 8

  %call32 = call spir_func float @coshf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @coshf
  store float %call32, float addrspace(1)* %arrayidx25, align 8

  %call33 = call spir_func float @atanf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @atanf
  store float %call33, float addrspace(1)* %arrayidx25, align 8

  %call34 = call spir_func float @atanhf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @atanhf
  store float %call34, float addrspace(1)* %arrayidx25, align 8

  %call35 = call spir_func float @atan2f(float 1.00e+00, float 1.00e+00)
; CHECK: {{.*}} call spir_func float @atan2f
  store float %call35, float addrspace(1)* %arrayidx25, align 8

  %call36 = call spir_func float @invsqrtf(float 1.00e+00)
; CHECK: {{.*}}  call spir_func float @_Z17__spirv_ocl_rsqrtf
  store float %call36, float addrspace(1)* %arrayidx25, align 8

  %sineptr = addrspacecast float addrspace(1)* %arrayidx25 to float addrspace(4)*
  %cosptr = addrspacecast float addrspace(1)* %arrayidx25 to float addrspace(4)*
  call spir_func void @sincosf(float 1.00e+00, float addrspace(4)* %sineptr, float addrspace(4)* %cosptr)
; CHECK: [[SINE:%[A-Za-z0-9_.]+]] = call spir_func float @_Z18__spirv_ocl_sincosfPf
; CHECK: store float [[SINE]], float addrspace(4)* %sineptr

  br label %DIR.OMP.END.TARGET.2

DIR.OMP.END.TARGET.2:                             ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.END.TARGET.232.exitStub
}

; Function Attrs: nounwind
declare dso_local spir_func void @sincosf(float, float addrspace(4)*, float addrspace(4)*) #1
; CHECK: declare dso_local spir_func float @_Z18__spirv_ocl_sincosfPf(float, float addrspace(4)*)

attributes #0 = { nounwind }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target.declare"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind readnone }
attributes #6 = { nounwind readnone speculatable }

!llvm.module.flags = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!spirv.Source = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{}
!2 = !{!"clang version 8.0.0"}
!3 = !{i32 4, i32 200000}
