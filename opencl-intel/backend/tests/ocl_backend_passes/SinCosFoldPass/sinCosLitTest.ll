; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -runtimelib=%t.rtl.bc -cos-sin-pass -S < %s | FileCheck %s
; ModuleID = 'main'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

;;two instances of sin with different parameters.

define void @sin_two(float %a, float %b, float* nocapture %c) nounwind {
entry:
  %call = tail call float @_Z3sinf(float %a) nounwind readnone
  %call1 = tail call float @_Z3sinf(float %b) nounwind readnone
  %add = fadd float %call, %call1
  store float %add, float* %c, align 4
  ret void
}

;CHECK: define void @sin_two
;CHECK-NOT: call float @_Z6sincos
;CHECK: call float @_Z3sinf(float %a)
;CHECK: call float @_Z3sinf(float %b)
;CHECK: ret void

declare float @_Z3sinf(float) nounwind readnone

;;two instances of cos with different parameters.

define void @cos_two(<2 x double> %a, <2 x double> %b, <2 x double>* nocapture %c) nounwind {
entry:
  %call = tail call <2 x double> @_Z3cosDv2_d(<2 x double> %a) nounwind readnone
  %call1 = tail call <2 x double> @_Z3cosDv2_d(<2 x double> %b) nounwind readnone
  %add = fadd <2 x double> %call, %call1
  store <2 x double> %add, <2 x double>* %c, align 16
  ret void
}
;CHECK: define void @cos_two
;CHECK-NOT: call <2 x double> @_Z6sincos
;CHECK: call <2 x double> @_Z3cosDv2_d(<2 x double> %a)
;CHECK: call <2 x double> @_Z3cosDv2_d(<2 x double> %b)
;CHECK: ret void

declare <2 x double> @_Z3cosDv2_d(<2 x double>) nounwind readnone

;;sin & cos with different parameters.


define void @cos_sin(double %a, double %b, double* nocapture %c) nounwind {
entry:
  %call = tail call double @_Z3cosd(double %a) nounwind readnone
  %call1 = tail call double @_Z3sind(double %b) nounwind readnone
  %add = fadd double %call, %call1
  store double %add, double* %c, align 8
  ret void
}
;CHECK: define void @cos_sin
;CHECK-NOT: call double @_Z6sincos
;CHECK: call double @_Z3cosd(double %a)
;CHECK: call double @_Z3sind(double %b)
;CHECK: ret void

declare double @_Z3cosd(double) nounwind readnone

declare double @_Z3sind(double) nounwind readnone

;;sin & cos with the same parameter.

define void @cos_sin_replace(<4 x float> %a, <4 x float> %b, <4 x float>* nocapture %c) nounwind {
entry:
  %call = tail call <4 x float> @_Z3cosDv4_f(<4 x float> %a) nounwind readnone
  %call1 = tail call <4 x float> @_Z3sinDv4_f(<4 x float> %a) nounwind readnone
  %add = fadd <4 x float> %call, %call1
  store <4 x float> %add, <4 x float>* %c, align 16
  ret void
}
;CHECK: define void @cos_sin_replace
;CHECK-NOT: call <4 x float> @_Z3cosDv4_f
;CHECK-NOT: call <4 x float> @_Z3sinDv4_f
;CHECK: call <4 x float> @_Z6sincosDv4_fPS_
;CHECK: ret void

declare <4 x float> @_Z3cosDv4_f(<4 x float>) nounwind readnone

declare <4 x float> @_Z3sinDv4_f(<4 x float>) nounwind readnone

;; 2 pairs of sin & cos
;; 1 with the same parameter.
;; 1 with the different parameters.

define void @sin_cos_2Cases(<8 x float> %a, <8 x float> %b, <8 x float>* nocapture %c) nounwind {
entry:
  %call = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %b) nounwind readnone
  %add = fadd <8 x float> %call, %call1
  %mul = fmul <8 x float> %a, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  %call2 = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %mul) nounwind readnone
  %add3 = fadd <8 x float> %add, %call2
  %call4 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %a) nounwind readnone
  %add5 = fadd <8 x float> %add3, %call4
  store <8 x float> %add5, <8 x float>* %c, align 32
  ret void
}

;CHECK: define void @sin_cos_2Cases
;CHECK: call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> %a, <8 x float>*
;CHECK-NOT: call <8 x float> @_Z3cosDv8_f(<8 x float> %a)
;CHECK:call <8 x float> @_Z3sinDv8_f(<8 x float> %b)
;CHECK:call <8 x float> @_Z3cosDv8_f(<8 x float>
;CHECK-NOT: call <8 x float> @_Z3sinDv8_f(<8 x float> %a)
;CHECK: ret void

declare <8 x float> @_Z3cosDv8_f(<8 x float>) nounwind readnone

declare <8 x float> @_Z3sinDv8_f(<8 x float>) nounwind readnone

;; 2 pairs of sin & cos each pair with its own common parameter

define void @sin_cos_2Common(<8 x float> %a, <8 x float> %b, <8 x float>* nocapture %c) nounwind {
entry:
  %call = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %b) nounwind readnone
  %add = fadd <8 x float> %call, %call1
  %call2 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %a) nounwind readnone
  %add3 = fadd <8 x float> %add, %call2
  %call4 = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %b) nounwind readnone
  %add5 = fadd <8 x float> %add3, %call4
  store <8 x float> %add5, <8 x float>* %c, align 32
  ret void
}

;CHECK: define void @sin_cos_2Common
;CHECK: call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> %a, <8 x float>*
;CHECK: call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> %b, <8 x float>*
;CHECK-NOT: call <8 x float> @_Z3cosDv8_f
;CHECK-NOT: call <8 x float> @_Z3sinDv8_f
;CHECK: ret void

define void @sin_cos_2Common_diff_types(<8 x float> %a, <4 x float> %b, <8 x float>* nocapture %c) nounwind {
entry:
  %call = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <4 x float> @_Z3sinDv4_f(<4 x float> %b) nounwind readnone
  %call2 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %a) nounwind readnone
  %call4 = tail call <4 x float> @_Z3cosDv4_f(<4 x float> %b) nounwind readnone
  store <8 x float> %call2, <8 x float>* %c, align 32
  ret void
}

;CHECK: define void @sin_cos_2Common_diff_types
;CHECK: call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> %a, <8 x float>*
;CHECK-NOT: call <4 x float> @_Z6cosDv4_f
;CHECK-NOT: call <4 x float> @_Z6sinDv4_f
;CHECK-NOT: call <8 x float> @_Z3cosDv8_f
;CHECK-NOT: call <8 x float> @_Z3sinDv8_f
;CHECK: call <4 x float> @_Z6sincosDv4_fPS_(<4 x float> %b, <4 x float>*
;CHECK-NOT: call <8 x float> @_Z3cosDv8_f
;CHECK-NOT: call <8 x float> @_Z3sinDv8_f
;CHECK-NOT: call <4 x float> @_Z6cosDv4_f
;CHECK-NOT: call <4 x float> @_Z6sinDv4_f
;CHECK: ret void

;; 2 pairs of sin & cos each pair with its own common parameter
;; additional sin with the same parameter as one of the pair - a stats counter should be incremented.

define void @sin_cos_not_replaced(<8 x float> %a, <8 x float> %b, <8 x float>* nocapture %c) nounwind {
entry:
  %call = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %b) nounwind readnone
  %add = fadd <8 x float> %call, %call1
  %call2 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %a) nounwind readnone
  %add3 = fadd <8 x float> %add, %call2
  %call4 = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %b) nounwind readnone
  %call6 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %a) nounwind readnone
  %add5 = fadd <8 x float> %add3, %call4
  store <8 x float> %add5, <8 x float>* %c, align 32
  ret void
}

;CHECK: define void @sin_cos_not_replaced
;CHECK: call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> %a, <8 x float>*
;CHECK: call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> %b, <8 x float>*
;CHECK-NOT: call <8 x float> @_Z3cosDv8_f
;CHECK: call <8 x float> @_Z3sinDv8_f(<8 x float> %a)
;CHECK: ret void

;; 2 pairs of sin & cos each pair with its own common parameter
;; additional cos with the same parameter as one of the pair - a stats counter should be incremented.

define void @sin_cos_not_replaced2(<8 x float> %a, <8 x float> %b, <8 x float>* nocapture %c) nounwind {
entry:
  %call = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %b) nounwind readnone
  %add = fadd <8 x float> %call, %call1
  %call2 = tail call <8 x float> @_Z3sinDv8_f(<8 x float> %a) nounwind readnone
  %add3 = fadd <8 x float> %add, %call2
  %call4 = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %b) nounwind readnone
  %call6 = tail call <8 x float> @_Z3cosDv8_f(<8 x float> %b) nounwind readnone
  %add5 = fadd <8 x float> %add3, %call4
  store <8 x float> %add5, <8 x float>* %c, align 32
  ret void
}

;CHECK: define void @sin_cos_not_replaced2
;CHECK: call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> %a, <8 x float>*
;CHECK: call <8 x float> @_Z6sincosDv8_fPS_(<8 x float> %b, <8 x float>*
;CHECK-NOT: call <8 x float> @_Z3sinDv8_f
;CHECK: call <8 x float> @_Z3cosDv8_f(<8 x float> %b)
;CHECK: ret void

define void @ifElse(double %a, double %b, double* nocapture %c) nounwind {
entry:
  %cmp = fcmp ogt double %a, 1.000000e+01
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call = tail call double @_Z3cosd(double %a) nounwind readnone
  %call1 = tail call double @_Z3sind(double %a) nounwind readnone
  %add = fadd double %call, %call1
  br label %if.end

if.else:                                          ; preds = %entry
  %call2 = tail call double @_Z3sind(double %b) nounwind readnone
  %call3 = tail call double @_Z3cosd(double %b) nounwind readnone
  %add4 = fadd double %call2, %call3
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %storemerge = phi double [ %add4, %if.else ], [ %add, %if.then ]
  store double %storemerge, double* %c, align 8
  ret void
}

;CHECK: define void @ifElse
;CHECK: call double @_Z6sincosdPd(double %a, double*
;CHECK-NOT: call <8 x float> @_Z3cosd
;CHECK-NOT: call <8 x float> @_Z3sind
;CHECK: call double @_Z6sincosdPd(double %b, double*
;CHECK-NOT: call <8 x float> @_Z3sind
;CHECK-NOT: call <8 x float> @_Z3cosd
;CHECK: ret void

define void @ifElse_cantReplace(double %a, double %b, double* nocapture %c) nounwind {
entry:
  %cmp = fcmp ogt double %a, 1.000000e+01
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call = tail call double @_Z3cosd(double %a) nounwind readnone
  %call1 = tail call double @_Z3sind(double %b) nounwind readnone
  %add = fadd double %call, %call1
  br label %if.end

if.else:                                          ; preds = %entry
  %call2 = tail call double @_Z3sind(double %a) nounwind readnone
  %call3 = tail call double @_Z3cosd(double %b) nounwind readnone
  %add4 = fadd double %call2, %call3
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %storemerge = phi double [ %add4, %if.else ], [ %add, %if.then ]
  store double %storemerge, double* %c, align 8
  ret void
}

;CHECK: define void @ifElse
;CHECK: call double @_Z3cosd(double %a)
;CHECK: call double @_Z3sind(double %b)
;CHECK: call double @_Z3sind(double %a)
;CHECK: call double @_Z3cosd(double %b)
;CHECK: ret void

