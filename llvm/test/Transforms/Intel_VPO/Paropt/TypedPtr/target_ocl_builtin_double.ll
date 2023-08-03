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

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local spir_func double @sin(double)
; CHECK: declare dso_local spir_func double @sin(double)

declare dso_local spir_func double @cos(double)
; CHECK: declare dso_local spir_func double @cos(double)

declare dso_local spir_func double @tan(double)
; CHECK: declare dso_local spir_func double @tan(double)

declare dso_local spir_func double @pow(double, double)
; CHECK: declare dso_local spir_func double @pow(double, double)

declare dso_local spir_func double @exp(double)
; CHECK: declare dso_local spir_func double @exp(double)

declare dso_local spir_func double @log(double)
; CHECK: declare dso_local spir_func double @log(double)

declare dso_local spir_func double @llvm.ceil.f64(double)
; CHECK: declare dso_local spir_func double @_Z16__spirv_ocl_ceild(double)

declare dso_local spir_func double @llvm.floor.f64(double)
; CHECK: declare dso_local spir_func double @_Z17__spirv_ocl_floord(double)

declare dso_local spir_func double @llvm.fabs.f64(double)
; CHECK: declare dso_local spir_func double @llvm.fabs.f64(double)

declare dso_local spir_func double @sqrt(double)
; CHECK: declare dso_local spir_func double @sqrt(double)

declare dso_local spir_func double @log2(double)
; CHECK: declare dso_local spir_func double @log2(double)

declare dso_local spir_func double @erf(double)
; CHECK: declare dso_local spir_func double @erf(double)

declare dso_local spir_func double @llvm.maxnum.f64(double, double)
; CHECK: declare dso_local spir_func double @_Z16__spirv_ocl_fmaxdd(double, double)

declare dso_local spir_func double @llvm.minnum.f64(double, double)
; CHECK: declare dso_local spir_func double @_Z16__spirv_ocl_fmindd(double, double)

; MANUALLY ADDED

declare dso_local spir_func double @asin(double)
; CHECK: declare dso_local spir_func double @asin(double)

declare dso_local spir_func double @asinh(double)
; CHECK: declare dso_local spir_func double @asinh(double)

declare dso_local spir_func double @sinh(double)
; CHECK: declare dso_local spir_func double @sinh(double)

declare dso_local spir_func double @acos(double)
; CHECK: declare dso_local spir_func double @acos(double)

declare dso_local spir_func double @acosh(double)
; CHECK: declare dso_local spir_func double @acosh(double)

declare dso_local spir_func double @cosh(double)
; CHECK: declare dso_local spir_func double @cosh(double)

declare dso_local spir_func double @atan(double)
; CHECK: declare dso_local spir_func double @atan(double)

declare dso_local spir_func double @atanh(double)
; CHECK: declare dso_local spir_func double @atanh(double)

declare dso_local spir_func double @atan2(double, double)
; CHECK: declare dso_local spir_func double @atan2(double, double)

declare dso_local spir_func double @tanh(double)
; CHECK: declare dso_local spir_func double @tanh(double)

declare dso_local spir_func double @invsqrt(double)
; CHECK: declare dso_local spir_func double @_Z17__spirv_ocl_rsqrtd(double)

define dso_local spir_kernel void @__omp_offloading_fd02_d323b8_main_l37([20 x double] addrspace(1)* %array) #0 {
newFuncRoot:
  br label %for.end

DIR.OMP.END.TARGET.232.exitStub:                  ; preds = %DIR.OMP.END.TARGET.2
  ret void

for.end:                                          ; preds = %newFuncRoot
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %for.end
  %call = call spir_func double @sin(double 1.000000e+00)
; CHECK: {{.*}} call spir_func double @sin
  %arrayidx1 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 0
  store double %call, double addrspace(1)* %arrayidx1, align 8
  %call2 = call spir_func double @cos(double 1.000000e+00)
; CHECK: {{.*}} call spir_func double @cos
  %arrayidx3 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 1
  store double %call2, double addrspace(1)* %arrayidx3, align 8
  %call4 = call spir_func double @tan(double 1.000000e+00)
; CHECK: {{.*}} call spir_func double @tan
  %arrayidx5 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 2
  store double %call4, double addrspace(1)* %arrayidx5, align 8
  %call6 = call spir_func double @pow(double 2.000000e+00, double 3.000000e+00)
; CHECK: {{.*}} call spir_func double @pow
  %arrayidx7 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 3
  store double %call6, double addrspace(1)* %arrayidx7, align 8
  %call8 = call spir_func double @exp(double 2.000000e+00)
; CHECK: {{.*}} call spir_func double @exp
  %arrayidx9 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 4
  store double %call8, double addrspace(1)* %arrayidx9, align 8
  %call10 = call spir_func double @log(double 2.000000e+00)
; CHECK: {{.*}} call spir_func double @log
  %arrayidx11 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 5
  store double %call10, double addrspace(1)* %arrayidx11, align 8
  %call12 = call spir_func double @llvm.ceil.f64(double 2.500000e+00)
; CHECK: {{.*}} call spir_func double @_Z16__spirv_ocl_ceild
  %arrayidx13 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 6
  store double %call12, double addrspace(1)* %arrayidx13, align 8
  %call14 = call spir_func double @llvm.floor.f64(double 2.500000e+00)
; CHECK: {{.*}} call spir_func double @_Z17__spirv_ocl_floord
  %arrayidx15 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 7
  store double %call14, double addrspace(1)* %arrayidx15, align 8
  %call16 = call spir_func double @llvm.fabs.f64(double -2.000000e+00)
; CHECK: {{.*}} call spir_func double @llvm.fabs.f64
  %arrayidx17 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 8
  store double %call16, double addrspace(1)* %arrayidx17, align 8
  %call18 = call spir_func double @sqrt(double 3.000000e+00)
; CHECK: {{.*}} call spir_func double @sqrt
  %arrayidx19 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 9
  store double %call18, double addrspace(1)* %arrayidx19, align 8
  %call20 = call spir_func double @log2(double 3.000000e+00)
; CHECK: {{.*}} call spir_func double @log2
  %arrayidx21 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 10
  store double %call20, double addrspace(1)* %arrayidx21, align 8
  %call22 = call spir_func double @erf(double 3.000000e+00)
; CHECK: {{.*}} call spir_func double @erf
  %arrayidx23 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 11
  store double %call22, double addrspace(1)* %arrayidx23, align 8
  %call24 = call spir_func double @llvm.maxnum.f64(double 2.000000e+00, double 3.000000e+00)
; CHECK: {{.*}} call spir_func double @_Z16__spirv_ocl_fmaxdd
  %arrayidx25 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 12
  store double %call24, double addrspace(1)* %arrayidx25, align 8
  %call26 = call spir_func double @llvm.minnum.f64(double 2.000000e+00, double 3.000000e+00)
; CHECK: {{.*}} call spir_func double @_Z16__spirv_ocl_fmindd
  %arrayidx27 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 13
  store double %call26, double addrspace(1)* %arrayidx27, align 8

; MANUALLY ADDED
; Since there's no dead store elimination in this test, use the same address.

  %call27 = call spir_func double @asin(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @asin
  store double %call27, double addrspace(1)* %arrayidx25, align 8

  %call28 = call spir_func double @asinh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @asinh
  store double %call28, double addrspace(1)* %arrayidx25, align 8

  %call29 = call spir_func double @sinh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @sinh
  store double %call29, double addrspace(1)* %arrayidx25, align 8

  %call30 = call spir_func double @acos(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @acos
  store double %call30, double addrspace(1)* %arrayidx25, align 8

  %call31 = call spir_func double @acosh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @acosh
  store double %call31, double addrspace(1)* %arrayidx25, align 8

  %call32 = call spir_func double @cosh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @cosh
  store double %call32, double addrspace(1)* %arrayidx25, align 8

  %call33 = call spir_func double @atan(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @atan
  store double %call33, double addrspace(1)* %arrayidx25, align 8

  %call34 = call spir_func double @atanh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @atanh
  store double %call34, double addrspace(1)* %arrayidx25, align 8

  %call35 = call spir_func double @atan2(double 1.00e+00, double 1.00e+00)
; CHECK: {{.*}} call spir_func double @atan2
  store double %call35, double addrspace(1)* %arrayidx25, align 8

  %call36 = call spir_func double @tanh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @tanh
  store double %call36, double addrspace(1)* %arrayidx25, align 8

  %call37 = call spir_func double @invsqrt(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z17__spirv_ocl_rsqrtd
  store double %call37, double addrspace(1)* %arrayidx25, align 8

  %sineptr = addrspacecast double addrspace(1)* %arrayidx25 to double addrspace(4)*
  %cosptr = addrspacecast double addrspace(1)* %arrayidx25 to double addrspace(4)*
  call spir_func void @sincos(double 1.00e+00, double addrspace(4)* %sineptr, double addrspace(4)* %cosptr)
; CHECK: [[SINE:%[A-Za-z0-9_.]+]] = call spir_func double @_Z18__spirv_ocl_sincosdPd
; CHECK: store double [[SINE]], double addrspace(4)* %sineptr

  br label %DIR.OMP.END.TARGET.2

DIR.OMP.END.TARGET.2:                             ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.END.TARGET.232.exitStub
}

declare dso_local spir_func void @sincos(double, double addrspace(4)*, double addrspace(4)*)
; CHECK: declare dso_local spir_func double @_Z18__spirv_ocl_sincosdPd(double, double addrspace(4)*)

attributes #0 = { "target.declare"="true" }
