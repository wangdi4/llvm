; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

declare dso_local spir_func float @sinf(float)
; CHECK: declare dso_local spir_func float @sinf(float)

declare dso_local spir_func float @cosf(float)
; CHECK: declare dso_local spir_func float @cosf(float)

declare dso_local spir_func float @tanf(float)
; CHECK: declare dso_local spir_func float @tanf(float)

declare dso_local spir_func float @powf(float, float)
; CHECK: declare dso_local spir_func float @powf(float, float)

declare dso_local spir_func float @expf(float)
; CHECK: declare dso_local spir_func float @expf(float)

declare dso_local spir_func float @logf(float)
; CHECK: declare dso_local spir_func float @logf(float)

declare dso_local spir_func float @llvm.ceil.f32(float)
; CHECK: declare dso_local spir_func float @llvm.ceil.f32(float)

declare dso_local spir_func float @llvm.floor.f32(float)
; CHECK: declare dso_local spir_func float @_Z17__spirv_ocl_floorf(float)

declare dso_local spir_func float @llvm.fabs.f32(float)
; CHECK: declare dso_local spir_func float @llvm.fabs.f32(float)

declare dso_local spir_func float @sqrtf(float)
; CHECK: declare dso_local spir_func float @sqrtf(float)

declare dso_local spir_func float @log2f(float)
; CHECK: declare dso_local spir_func float @log2f(float)

declare dso_local spir_func float @erff(float)
; CHECK: declare dso_local spir_func float @erff(float)

declare dso_local spir_func float @llvm.maxnum.f32(float, float)
; CHECK: declare dso_local spir_func float @_Z16__spirv_ocl_fmaxff(float, float)

declare dso_local spir_func float @llvm.minnum.f32(float, float)
; CHECK: declare dso_local spir_func float @_Z16__spirv_ocl_fminff(float, float)

; MANUALLY ADDED

declare dso_local spir_func float @asinf(float)
; CHECK: declare dso_local spir_func float @asinf(float)

declare dso_local spir_func float @asinhf(float)
; CHECK: declare dso_local spir_func float @asinhf(float)

declare dso_local spir_func float @sinhf(float)
; CHECK: declare dso_local spir_func float @sinhf(float)

declare dso_local spir_func float @acosf(float)
; CHECK: declare dso_local spir_func float @acosf(float)

declare dso_local spir_func float @acoshf(float)
; CHECK: declare dso_local spir_func float @acoshf(float)

declare dso_local spir_func float @coshf(float)
; CHECK: declare dso_local spir_func float @coshf(float)

declare dso_local spir_func float @atanf(float)
; CHECK: declare dso_local spir_func float @atanf(float)

declare dso_local spir_func float @atanhf(float)
; CHECK: declare dso_local spir_func float @atanhf(float)

declare dso_local spir_func float @atan2f(float, float)
; CHECK: declare dso_local spir_func float @atan2f(float, float)

declare dso_local spir_func float @tanhf(float)
; CHECK: declare dso_local spir_func float @tanhf(float)

declare dso_local spir_func float @invsqrtf(float)
; CHECK: declare dso_local spir_func float @_Z17__spirv_ocl_rsqrtf(float)

define dso_local spir_kernel void @__omp_offloading_fd02_d323ad_main_l46(ptr addrspace(1) %array) #0 {
newFuncRoot:
  br label %for.end

DIR.OMP.END.TARGET.232.exitStub:                  ; preds = %DIR.OMP.END.TARGET.2
  ret void

for.end:                                          ; preds = %newFuncRoot
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %for.end
  %call = call spir_func float @sinf(float 1.000000e+00)
; CHECK: {{.*}}  call spir_func float @sinf
  store float %call, ptr addrspace(1) %array, align 4
  %call2 = call spir_func float @cosf(float 1.000000e+00)
; CHECK: {{.*}}  call spir_func float @cosf
  %arrayidx3 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 1
  store float %call2, ptr addrspace(1) %arrayidx3, align 4
  %call4 = call spir_func float @tanf(float 1.000000e+00)
; CHECK: {{.*}}  call spir_func float @tanf
  %arrayidx5 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 2
  store float %call4, ptr addrspace(1) %arrayidx5, align 4
  %call6 = call spir_func float @powf(float 2.000000e+00, float 3.000000e+00)
; CHECK: {{.*}}  call spir_func float @powf
  %arrayidx7 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 3
  store float %call6, ptr addrspace(1) %arrayidx7, align 4
  %call8 = call spir_func float @expf(float 2.000000e+00)
; CHECK: {{.*}}  call spir_func float @expf
  %arrayidx9 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 4
  store float %call8, ptr addrspace(1) %arrayidx9, align 4
  %call10 = call spir_func float @logf(float 2.000000e+00)
; CHECK: {{.*}}  call spir_func float @logf
  %arrayidx11 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 5
  store float %call10, ptr addrspace(1) %arrayidx11, align 4
  %call12 = call spir_func float @llvm.ceil.f32(float 2.500000e+00)
; CHECK: {{.*}}  call spir_func float @llvm.ceil.f32
  %arrayidx13 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 6
  store float %call12, ptr addrspace(1) %arrayidx13, align 4
  %call14 = call spir_func float @llvm.floor.f32(float 2.500000e+00)
; CHECK: {{.*}}  call spir_func float @_Z17__spirv_ocl_floorf
  %arrayidx15 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 7
  store float %call14, ptr addrspace(1) %arrayidx15, align 4
  %call16 = call spir_func float @llvm.fabs.f32(float -2.000000e+00)
; CHECK: {{.*}}  call spir_func float @llvm.fabs.f32
  %arrayidx17 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 8
  store float %call16, ptr addrspace(1) %arrayidx17, align 4
  %call18 = call spir_func float @sqrtf(float 3.000000e+00)
; CHECK: {{.*}}  call spir_func float @sqrtf
  %arrayidx19 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 9
  store float %call18, ptr addrspace(1) %arrayidx19, align 4
  %call20 = call spir_func float @log2f(float 3.000000e+00)
; CHECK: {{.*}}  call spir_func float @log2f
  %arrayidx21 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 10
  store float %call20, ptr addrspace(1) %arrayidx21, align 4
  %call22 = call spir_func float @erff(float 3.000000e+00)
; CHECK: {{.*}}  call spir_func float @erff
  %arrayidx23 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 10
  store float %call22, ptr addrspace(1) %arrayidx23, align 4
  %call24 = call spir_func float @llvm.maxnum.f32(float 2.000000e+00, float 3.000000e+00)
; CHECK: {{.*}} call spir_func float @_Z16__spirv_ocl_fmaxff
  %arrayidx25 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 12
  store float %call24, ptr addrspace(1) %arrayidx25, align 8
  %call26 = call spir_func float @llvm.minnum.f32(float 2.000000e+00, float 3.000000e+00)
; CHECK: {{.*}} call spir_func float @_Z16__spirv_ocl_fminff
  %arrayidx27 = getelementptr inbounds [20 x float], ptr addrspace(1) %array, i64 0, i64 13

; MANUALLY ADDED
; Since there's no dead store elimination in this test, use the same address.

  %call27 = call spir_func float @asinf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @asinf
  store float %call27, ptr addrspace(1) %arrayidx25, align 8

  %call28 = call spir_func float @asinhf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @asinhf
  store float %call28, ptr addrspace(1) %arrayidx25, align 8

  %call29 = call spir_func float @sinhf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @sinhf
  store float %call29, ptr addrspace(1) %arrayidx25, align 8

  %call30 = call spir_func float @acosf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @acosf
  store float %call30, ptr addrspace(1) %arrayidx25, align 8

  %call31 = call spir_func float @acoshf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @acoshf
  store float %call31, ptr addrspace(1) %arrayidx25, align 8

  %call32 = call spir_func float @coshf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @coshf
  store float %call32, ptr addrspace(1) %arrayidx25, align 8

  %call33 = call spir_func float @atanf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @atanf
  store float %call33, ptr addrspace(1) %arrayidx25, align 8

  %call34 = call spir_func float @atanhf(float 1.00e+00)
; CHECK: {{.*}} call spir_func float @atanhf
  store float %call34, ptr addrspace(1) %arrayidx25, align 8

  %call35 = call spir_func float @atan2f(float 1.00e+00, float 1.00e+00)
; CHECK: {{.*}} call spir_func float @atan2f
  store float %call35, ptr addrspace(1) %arrayidx25, align 8

  %call36 = call spir_func float @invsqrtf(float 1.00e+00)
; CHECK: {{.*}}  call spir_func float @_Z17__spirv_ocl_rsqrtf
  store float %call36, ptr addrspace(1) %arrayidx25, align 8

  %sineptr = addrspacecast ptr addrspace(1) %arrayidx25 to ptr addrspace(4)
  %cosptr = addrspacecast ptr addrspace(1) %arrayidx25 to ptr addrspace(4)
  call spir_func void @sincosf(float 1.00e+00, ptr addrspace(4) %sineptr, ptr addrspace(4) %cosptr)
; CHECK: [[SINE:%[A-Za-z0-9_.]+]] = call spir_func float @_Z18__spirv_ocl_sincosfPf
; CHECK: store float [[SINE]], ptr addrspace(4) %sineptr

  br label %DIR.OMP.END.TARGET.2

DIR.OMP.END.TARGET.2:                             ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.END.TARGET.232.exitStub
}

declare dso_local spir_func void @sincosf(float, ptr addrspace(4), ptr addrspace(4))
; CHECK: declare dso_local spir_func float @_Z18__spirv_ocl_sincosfPf(float, ptr addrspace(4))

attributes #0 = { "target.declare"="true" }
