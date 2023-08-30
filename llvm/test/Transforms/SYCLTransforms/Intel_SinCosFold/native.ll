; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes='require<sycl-kernel-builtin-info-analysis>,sycl-kernel-sin-cos-fold' -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes='require<sycl-kernel-builtin-info-analysis>,sycl-kernel-sin-cos-fold' -S < %s | FileCheck %s

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

;;two instances of sin with different parameters.

define void @sin_two(float %a, float %b, ptr nocapture %c) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %call = tail call float @_Z10native_sinf(float %a) nounwind readnone
  %call1 = tail call float @_Z10native_sinf(float %b) nounwind readnone
  %add = fadd float %call, %call1
  store float %add, ptr %c, align 4
  ret void
}

;CHECK: define void @sin_two
;CHECK-NOT: call float @_Z13native_sincos
;CHECK: call float @_Z10native_sinf(float %a)
;CHECK: call float @_Z10native_sinf(float %b)
;CHECK: ret void

declare float @_Z10native_sinf(float) nounwind readnone

;;two instances of cos with different parameters.

define void @cos_two(<2 x double> %a, <2 x double> %b, ptr nocapture %c) nounwind !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  %call = tail call <2 x double> @_Z10native_cosDv2_d(<2 x double> %a) nounwind readnone
  %call1 = tail call <2 x double> @_Z10native_cosDv2_d(<2 x double> %b) nounwind readnone
  %add = fadd <2 x double> %call, %call1
  store <2 x double> %add, ptr %c, align 16
  ret void
}
;CHECK: define void @cos_two
;CHECK-NOT: call <2 x double> @_Z13native_sincos
;CHECK: call <2 x double> @_Z10native_cosDv2_d(<2 x double> %a)
;CHECK: call <2 x double> @_Z10native_cosDv2_d(<2 x double> %b)
;CHECK: ret void

declare <2 x double> @_Z10native_cosDv2_d(<2 x double>) nounwind readnone

;;sin & cos with different parameters.


define void @cos_sin(double %a, double %b, ptr nocapture %c) nounwind !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
  %call = tail call double @_Z10native_cosd(double %a) nounwind readnone
  %call1 = tail call double @_Z10native_sind(double %b) nounwind readnone
  %add = fadd double %call, %call1
  store double %add, ptr %c, align 8
  ret void
}
;CHECK: define void @cos_sin
;CHECK-NOT: call double @_Z13native_sincos
;CHECK: call double @_Z10native_cosd(double %a)
;CHECK: call double @_Z10native_sind(double %b)
;CHECK: ret void

declare double @_Z10native_cosd(double) nounwind readnone

declare double @_Z10native_sind(double) nounwind readnone

;;sin & cos with the same parameter.

define void @cos_sin_replace(<4 x float> %a, <4 x float> %b, ptr nocapture %c) nounwind !kernel_arg_base_type !6 !arg_type_null_val !7 {
entry:
  %call = tail call <4 x float> @_Z10native_cosDv4_f(<4 x float> %a) nounwind readnone
  %call1 = tail call <4 x float> @_Z10native_sinDv4_f(<4 x float> %a) nounwind readnone
  %add = fadd <4 x float> %call, %call1
  store <4 x float> %add, ptr %c, align 16
  ret void
}
;CHECK: define void @cos_sin_replace
;CHECK-NOT: call <4 x float> @_Z10native_cosDv4_f
;CHECK-NOT: call <4 x float> @_Z10native_sinDv4_f
;CHECK: call <4 x float> @_Z13native_sincosDv4_fPS_
;CHECK: ret void

declare <4 x float> @_Z10native_cosDv4_f(<4 x float>) nounwind readnone

declare <4 x float> @_Z10native_sinDv4_f(<4 x float>) nounwind readnone

;; 2 pairs of sin & cos
;; 1 with the same parameter.
;; 1 with the different parameters.

define void @sin_cos_2Cases(<8 x float> %a, <8 x float> %b, ptr nocapture %c) nounwind !kernel_arg_base_type !8 !arg_type_null_val !9 {
entry:
  %call = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %b) nounwind readnone
  %add = fadd <8 x float> %call, %call1
  %mul = fmul <8 x float> %a, <float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00, float 2.000000e+00>
  %call2 = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %mul) nounwind readnone
  %add3 = fadd <8 x float> %add, %call2
  %call4 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %a) nounwind readnone
  %add5 = fadd <8 x float> %add3, %call4
  store <8 x float> %add5, ptr %c, align 32
  ret void
}

;CHECK: define void @sin_cos_2Cases
;CHECK: call <8 x float> @_Z13native_sincosDv8_fPS_(<8 x float> %a, ptr
;CHECK-NOT: call <8 x float> @_Z10native_cosDv8_f(<8 x float> %a)
;CHECK:call <8 x float> @_Z10native_sinDv8_f(<8 x float> %b)
;CHECK:call <8 x float> @_Z10native_cosDv8_f(<8 x float>
;CHECK-NOT: call <8 x float> @_Z10native_sinDv8_f(<8 x float> %a)
;CHECK: ret void

declare <8 x float> @_Z10native_cosDv8_f(<8 x float>) nounwind readnone

declare <8 x float> @_Z10native_sinDv8_f(<8 x float>) nounwind readnone

;; 2 pairs of sin & cos each pair with its own common parameter

define void @sin_cos_2Common(<8 x float> %a, <8 x float> %b, ptr nocapture %c) nounwind !kernel_arg_base_type !8 !arg_type_null_val !9 {
entry:
  %call = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %b) nounwind readnone
  %add = fadd <8 x float> %call, %call1
  %call2 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %a) nounwind readnone
  %add3 = fadd <8 x float> %add, %call2
  %call4 = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %b) nounwind readnone
  %add5 = fadd <8 x float> %add3, %call4
  store <8 x float> %add5, ptr %c, align 32
  ret void
}

;CHECK: define void @sin_cos_2Common
;CHECK: call <8 x float> @_Z13native_sincosDv8_fPS_(<8 x float> %a, ptr
;CHECK: call <8 x float> @_Z13native_sincosDv8_fPS_(<8 x float> %b, ptr
;CHECK-NOT: call <8 x float> @_Z10native_cosDv8_f
;CHECK-NOT: call <8 x float> @_Z10native_sinDv8_f
;CHECK: ret void

define void @sin_cos_2Common_diff_types(<8 x float> %a, <4 x float> %b, ptr nocapture %c) nounwind !kernel_arg_base_type !10 !arg_type_null_val !11 {
entry:
  %call = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <4 x float> @_Z10native_sinDv4_f(<4 x float> %b) nounwind readnone
  %call2 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %a) nounwind readnone
  %call4 = tail call <4 x float> @_Z10native_cosDv4_f(<4 x float> %b) nounwind readnone
  store <8 x float> %call2, ptr %c, align 32
  ret void
}

;CHECK: define void @sin_cos_2Common_diff_types
;CHECK: call <8 x float> @_Z13native_sincosDv8_fPS_(<8 x float> %a, ptr
;CHECK-NOT: call <4 x float> @_Z6cosDv4_f
;CHECK-NOT: call <4 x float> @_Z6sinDv4_f
;CHECK-NOT: call <8 x float> @_Z10native_cosDv8_f
;CHECK-NOT: call <8 x float> @_Z10native_sinDv8_f
;CHECK: call <4 x float> @_Z13native_sincosDv4_fPS_(<4 x float> %b, ptr
;CHECK-NOT: call <8 x float> @_Z10native_cosDv8_f
;CHECK-NOT: call <8 x float> @_Z10native_sinDv8_f
;CHECK-NOT: call <4 x float> @_Z6cosDv4_f
;CHECK-NOT: call <4 x float> @_Z6sinDv4_f
;CHECK: ret void

;; 2 pairs of sin & cos each pair with its own common parameter
;; additional sin with the same parameter as one of the pair - a stats counter should be incremented.

define void @sin_cos_not_replaced(<8 x float> %a, <8 x float> %b, ptr nocapture %c) nounwind !kernel_arg_base_type !8 !arg_type_null_val !9 {
entry:
  %call = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %b) nounwind readnone
  %add = fadd <8 x float> %call, %call1
  %call2 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %a) nounwind readnone
  %add3 = fadd <8 x float> %add, %call2
  %call4 = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %b) nounwind readnone
  %call6 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %a) nounwind readnone
  %add5 = fadd <8 x float> %add3, %call4
  store <8 x float> %add5, ptr %c, align 32
  ret void
}

;CHECK: define void @sin_cos_not_replaced
;CHECK: call <8 x float> @_Z13native_sincosDv8_fPS_(<8 x float> %a, ptr
;CHECK: call <8 x float> @_Z13native_sincosDv8_fPS_(<8 x float> %b, ptr
;CHECK-NOT: call <8 x float> @_Z10native_cosDv8_f
;CHECK: call <8 x float> @_Z10native_sinDv8_f(<8 x float> %a)
;CHECK: ret void

;; 2 pairs of sin & cos each pair with its own common parameter
;; additional cos with the same parameter as one of the pair - a stats counter should be incremented.

define void @sin_cos_not_replaced2(<8 x float> %a, <8 x float> %b, ptr nocapture %c) nounwind !kernel_arg_base_type !8 !arg_type_null_val !9 {
entry:
  %call = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %a) nounwind readnone
  %call1 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %b) nounwind readnone
  %add = fadd <8 x float> %call, %call1
  %call2 = tail call <8 x float> @_Z10native_sinDv8_f(<8 x float> %a) nounwind readnone
  %add3 = fadd <8 x float> %add, %call2
  %call4 = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %b) nounwind readnone
  %call6 = tail call <8 x float> @_Z10native_cosDv8_f(<8 x float> %b) nounwind readnone
  %add5 = fadd <8 x float> %add3, %call4
  store <8 x float> %add5, ptr %c, align 32
  ret void
}

;CHECK: define void @sin_cos_not_replaced2
;CHECK: call <8 x float> @_Z13native_sincosDv8_fPS_(<8 x float> %a, ptr
;CHECK: call <8 x float> @_Z13native_sincosDv8_fPS_(<8 x float> %b, ptr
;CHECK-NOT: call <8 x float> @_Z10native_sinDv8_f
;CHECK: call <8 x float> @_Z10native_cosDv8_f(<8 x float> %b)
;CHECK: ret void

define void @ifElse(double %a, double %b, ptr nocapture %c) nounwind !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
  %cmp = fcmp ogt double %a, 1.000000e+01
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call = tail call double @_Z10native_cosd(double %a) nounwind readnone
  %call1 = tail call double @_Z10native_sind(double %a) nounwind readnone
  %add = fadd double %call, %call1
  br label %if.end

if.else:                                          ; preds = %entry
  %call2 = tail call double @_Z10native_sind(double %b) nounwind readnone
  %call3 = tail call double @_Z10native_cosd(double %b) nounwind readnone
  %add4 = fadd double %call2, %call3
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %storemerge = phi double [ %add4, %if.else ], [ %add, %if.then ]
  store double %storemerge, ptr %c, align 8
  ret void
}

;CHECK: define void @ifElse
;CHECK: call double @_Z13native_sincosdPd(double %a, ptr
;CHECK-NOT: call <8 x float> @_Z10native_cosd
;CHECK-NOT: call <8 x float> @_Z10native_sind
;CHECK: call double @_Z13native_sincosdPd(double %b, ptr
;CHECK-NOT: call <8 x float> @_Z10native_sind
;CHECK-NOT: call <8 x float> @_Z10native_cosd
;CHECK: ret void

define void @ifElse_cantReplace(double %a, double %b, ptr nocapture %c) nounwind !kernel_arg_base_type !4 !arg_type_null_val !5 {
entry:
  %cmp = fcmp ogt double %a, 1.000000e+01
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %call = tail call double @_Z10native_cosd(double %a) nounwind readnone
  %call1 = tail call double @_Z10native_sind(double %b) nounwind readnone
  %add = fadd double %call, %call1
  br label %if.end

if.else:                                          ; preds = %entry
  %call2 = tail call double @_Z10native_sind(double %a) nounwind readnone
  %call3 = tail call double @_Z10native_cosd(double %b) nounwind readnone
  %add4 = fadd double %call2, %call3
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %storemerge = phi double [ %add4, %if.else ], [ %add, %if.then ]
  store double %storemerge, ptr %c, align 8
  ret void
}

;CHECK: define void @ifElse_cantReplace
;CHECK: call double @_Z10native_cosd(double %a)
;CHECK: call double @_Z10native_sind(double %b)
;CHECK: call double @_Z10native_sind(double %a)
;CHECK: call double @_Z10native_cosd(double %b)
;CHECK: ret void

; DEBUGIFY-NOT: WARNING
; DEBUGIFY-COUNT-12: WARNING: Instruction with empty DebugLoc in function {{.*}} alloca
; DEBUGIFY: PASS

!0 = !{!"float", !"float", !"float*"}
!1 = !{float 0.000000e+00, float 0.000000e+00, ptr null}
!2 = !{!"double2", !"double2", !"double*"}
!3 = !{<2 x double> <double 0.000000e+00, double 0.000000e+00>, <2 x double> <double 0.000000e+00, double 0.000000e+00>, ptr null}
!4 = !{!"double", !"double", !"double*"}
!5 = !{double 0.000000e+00, double 0.000000e+00, ptr null}
!6 = !{!"float4", !"float4", !"float4*"}
!7 = !{<4 x float> zeroinitializer, <4 x float> zeroinitializer, ptr null}
!8 = !{!"float8", !"float8", !"float8*"}
!9 = !{<8 x float> zeroinitializer, <8 x float> zeroinitializer, ptr null}
!10 = !{!"float8", !"float4", !"float8*"}
!11 = !{<8 x float> zeroinitializer, <4 x float> zeroinitializer, ptr null}
