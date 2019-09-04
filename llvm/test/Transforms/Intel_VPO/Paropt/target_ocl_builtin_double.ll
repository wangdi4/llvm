; RUN: opt -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S < %s  | FileCheck %s

; This test checks that VPOParopt translates names of math functions (double)
; into OpenCL builtin names. This is needed for spir64 targets.
;
; List of functions:
; sin,cos,tan,pow,exp,log,ceil,floor,fabs,sqrt,log2,erf,fmax,fmin,
; asin,asinh,acos,acosh,atan,atanh,atan2
; To add new functions, add them to the 2 areas marked MANUALLY ADDED.

;   - For C++, clang will generate calls to ceil, floor, fabs, fmax, fmin.
;   - For C, clang will generate calls to
;       llvm.(ceil|floor|fabs|maxnum|minnum).f64
; We test both forms in one test.

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
source_filename = "target_ocl_builtin_double.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"
target device_triples = "spir64"

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

; Function Attrs: nounwind
declare dso_local spir_func double @sin(double) #1
; CHECK: declare dso_local spir_func double @_Z3sind(double)

; Function Attrs: nounwind
declare dso_local spir_func double @cos(double) #1
; CHECK: declare dso_local spir_func double @_Z3cosd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @tan(double) #1
; CHECK: declare dso_local spir_func double @_Z3tand(double)

; Function Attrs: nounwind
declare dso_local spir_func double @pow(double, double) #1
; CHECK: declare dso_local spir_func double @_Z3powdd(double, double)

; Function Attrs: nounwind
declare dso_local spir_func double @exp(double) #1
; CHECK: declare dso_local spir_func double @_Z3expd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @log(double) #1
; CHECK: declare dso_local spir_func double @_Z3logd(double)

; Function Attrs: nounwind readnone
declare dso_local spir_func double @ceil(double) #2

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func double @llvm.ceil.f64(double) #6
; CHECK: declare dso_local spir_func double @_Z4ceild(double)

; Function Attrs: nounwind readnone
declare dso_local spir_func double @floor(double) #2

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func double @llvm.floor.f64(double) #6
; CHECK: declare dso_local spir_func double @_Z5floord(double)

; Function Attrs: nounwind readnone
declare dso_local spir_func double @fabs(double) #2

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func double @llvm.fabs.f64(double) #6
; CHECK: declare dso_local spir_func double @_Z4fabsd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @sqrt(double) #1
; CHECK: declare dso_local spir_func double @_Z4sqrtd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @log2(double) #1
; CHECK: declare dso_local spir_func double @_Z4log2d(double)

; Function Attrs: nounwind
declare dso_local spir_func double @erf(double) #1
; CHECK: declare dso_local spir_func double @_Z3erfd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @fmax(double, double) #1

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func double @llvm.maxnum.f64(double, double) #6
; CHECK: declare dso_local spir_func double @_Z4fmaxdd(double, double)

; Function Attrs: nounwind
declare dso_local spir_func double @fmin(double, double) #1

; Function Attrs: nounwind readnone speculatable
declare dso_local spir_func double @llvm.minnum.f64(double, double) #6
; CHECK: declare dso_local spir_func double @_Z4fmindd(double, double)

; MANUALLY ADDED

; Function Attrs: nounwind
declare dso_local spir_func double @asin(double) #1
; CHECK: declare dso_local spir_func double @_Z4asind(double)

; Function Attrs: nounwind
declare dso_local spir_func double @asinh(double) #1
; CHECK: declare dso_local spir_func double @_Z5asinhd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @sinh(double) #1
; CHECK: declare dso_local spir_func double @_Z4sinhd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @acos(double) #1
; CHECK: declare dso_local spir_func double @_Z4acosd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @acosh(double) #1
; CHECK: declare dso_local spir_func double @_Z5acoshd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @cosh(double) #1
; CHECK: declare dso_local spir_func double @_Z4coshd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @atan(double) #1
; CHECK: declare dso_local spir_func double @_Z4atand(double)

; Function Attrs: nounwind
declare dso_local spir_func double @atanh(double) #1
; CHECK: declare dso_local spir_func double @_Z5atanhd(double)

; Function Attrs: nounwind
declare dso_local spir_func double @atan2(double, double) #1
; CHECK: declare dso_local spir_func double @_Z5atan2d(double, double)

; Function Attrs: nounwind
declare dso_local spir_func double @tanh(double) #1
; CHECK: declare dso_local spir_func double @_Z4tanhd(double)

; Function Attrs: noinline norecurse optnone uwtable
define dso_local spir_kernel void @__omp_offloading_fd02_d323b8_main_l37([20 x double] addrspace(1)* %array) #4 {
newFuncRoot:
  br label %for.end

DIR.OMP.END.TARGET.232.exitStub:                  ; preds = %DIR.OMP.END.TARGET.2
  ret void

for.end:                                          ; preds = %newFuncRoot
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %for.end
  %call = call spir_func double @sin(double 1.000000e+00) #0
; CHECK: {{.*}} call spir_func double @_Z3sind
  %arrayidx1 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 0
  store double %call, double addrspace(1)* %arrayidx1, align 8
  %call2 = call spir_func double @cos(double 1.000000e+00) #0
; CHECK: {{.*}} call spir_func double @_Z3cosd
  %arrayidx3 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 1
  store double %call2, double addrspace(1)* %arrayidx3, align 8
  %call4 = call spir_func double @tan(double 1.000000e+00) #0
; CHECK: {{.*}} call spir_func double @_Z3tand
  %arrayidx5 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 2
  store double %call4, double addrspace(1)* %arrayidx5, align 8
  %call6 = call spir_func double @pow(double 2.000000e+00, double 3.000000e+00) #0
; CHECK: {{.*}} call spir_func double @_Z3powdd
  %arrayidx7 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 3
  store double %call6, double addrspace(1)* %arrayidx7, align 8
  %call8 = call spir_func double @exp(double 2.000000e+00) #0
; CHECK: {{.*}} call spir_func double @_Z3expd
  %arrayidx9 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 4
  store double %call8, double addrspace(1)* %arrayidx9, align 8
  %call10 = call spir_func double @log(double 2.000000e+00) #0
; CHECK: {{.*}} call spir_func double @_Z3logd
  %arrayidx11 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 5
  store double %call10, double addrspace(1)* %arrayidx11, align 8
  %call12.1 = call spir_func double @ceil(double 2.500000e+00) #5
  %call12.2 = call spir_func double @llvm.ceil.f64(double 2.500000e+00)
; CHECK: {{.*}} call spir_func double @_Z4ceild
; CHECK: {{.*}} call spir_func double @_Z4ceild
  %call12 = fadd double %call12.1, %call12.2
  %arrayidx13 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 6
  store double %call12, double addrspace(1)* %arrayidx13, align 8
  %call14.1 = call spir_func double @floor(double 2.500000e+00)
  %call14.2 = call spir_func double @llvm.floor.f64(double 2.500000e+00)
; CHECK: {{.*}} call spir_func double @_Z5floord
; CHECK: {{.*}} call spir_func double @_Z5floord
  %call14 = fadd double %call14.1, %call14.2
  %arrayidx15 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 7
  store double %call14, double addrspace(1)* %arrayidx15, align 8
  %call16.1 = call spir_func double @fabs(double -2.000000e+00)
  %call16.2 = call spir_func double @llvm.fabs.f64(double -2.000000e+00)
; CHECK: {{.*}} call spir_func double @_Z4fabsd
; CHECK: {{.*}} call spir_func double @_Z4fabsd
  %call16 = fadd double %call16.1, %call16.2
  %arrayidx17 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 8
  store double %call16, double addrspace(1)* %arrayidx17, align 8
  %call18 = call spir_func double @sqrt(double 3.000000e+00) #0
; CHECK: {{.*}} call spir_func double @_Z4sqrtd
  %arrayidx19 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 9
  store double %call18, double addrspace(1)* %arrayidx19, align 8
  %call20 = call spir_func double @log2(double 3.000000e+00) #0
; CHECK: {{.*}} call spir_func double @_Z4log2d
  %arrayidx21 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 10
  store double %call20, double addrspace(1)* %arrayidx21, align 8
  %call22 = call spir_func double @erf(double 3.000000e+00) #0
; CHECK: {{.*}} call spir_func double @_Z3erfd
  %arrayidx23 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 11
  store double %call22, double addrspace(1)* %arrayidx23, align 8
  %call24.1 = call spir_func double @fmax(double 2.000000e+00, double 3.000000e+00)
  %call24.2 = call spir_func double @llvm.maxnum.f64(double 2.000000e+00, double 3.000000e+00)
; CHECK: {{.*}} call spir_func double @_Z4fmaxdd
; CHECK: {{.*}} call spir_func double @_Z4fmaxdd
  %call24 = fadd double %call24.1, %call24.2
  %arrayidx25 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 12
  store double %call24, double addrspace(1)* %arrayidx25, align 8
  %call26.1 = call spir_func double @fmin(double 2.000000e+00, double 3.000000e+00)
  %call26.2 = call spir_func double @llvm.minnum.f64(double 2.000000e+00, double 3.000000e+00)
; CHECK: {{.*}} call spir_func double @_Z4fmindd
; CHECK: {{.*}} call spir_func double @_Z4fmindd
  %call26 = fadd double %call26.1, %call26.2
  %arrayidx27 = getelementptr inbounds [20 x double], [20 x double] addrspace(1)* %array, i64 0, i64 13
  store double %call26, double addrspace(1)* %arrayidx27, align 8

; MANUALLY ADDED
; Since there's no dead store elimination in this test, use the same address.

  %call27 = call spir_func double @asin(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z4asind
  store double %call27, double addrspace(1)* %arrayidx25, align 8
  %call28 = call spir_func double @asinh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z5asinhd
  store double %call28, double addrspace(1)* %arrayidx25, align 8
  %call29 = call spir_func double @sinh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z4sinhd
  store double %call29, double addrspace(1)* %arrayidx25, align 8
  %call30 = call spir_func double @acos(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z4acosd
  store double %call30, double addrspace(1)* %arrayidx25, align 8
  %call31 = call spir_func double @acosh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z5acoshd
  store double %call31, double addrspace(1)* %arrayidx25, align 8
  %call32 = call spir_func double @cosh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z4coshd
  store double %call32, double addrspace(1)* %arrayidx25, align 8
  %call33 = call spir_func double @atan(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z4atand
  store double %call33, double addrspace(1)* %arrayidx25, align 8
  %call34 = call spir_func double @atanh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z5atanhd
  store double %call34, double addrspace(1)* %arrayidx25, align 8
  %call35 = call spir_func double @atan2(double 1.00e+00, double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z5atan2d
  store double %call35, double addrspace(1)* %arrayidx25, align 8
  %call36 = call spir_func double @tanh(double 1.00e+00)
; CHECK: {{.*}} call spir_func double @_Z4tanhd
  store double %call36, double addrspace(1)* %arrayidx25, align 8

  br label %DIR.OMP.END.TARGET.2

DIR.OMP.END.TARGET.2:                             ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.END.TARGET.232.exitStub
}

attributes #0 = { nounwind }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline norecurse optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target.declare"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind readnone }
attributes #6 = { nounwind readnone speculatable }

!llvm.module.flags = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!3}
!spirv.Source = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{}
!2 = !{!"cl_doubles"}
!3 = !{!"clang version 8.0.0"}
!4 = !{i32 4, i32 200000}
