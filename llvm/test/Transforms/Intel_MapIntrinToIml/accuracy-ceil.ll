; Some SVML functions (such as floor and ceil) don't have variants of different
; accuracies. This test checks these functions are always translated to the
; same function regardless of the accuracy specified.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @default_accuracy_no_fastmath(double %c) {
; CHECK-LABEL: @default_accuracy_no_fastmath
; CHECK: call svml_cc <2 x double> @__svml_ceil2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_ceil4(<4 x double> %broadcast.splat) #1
  ret void
}

define void @default_accuracy_fastmath(double %c) {
; CHECK-LABEL: @default_accuracy_fastmath
; CHECK: call fast svml_cc <2 x double> @__svml_ceil2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_ceil4(<4 x double> %broadcast.splat) #1
  ret void
}

define void @low_accuracy_no_fastmath(double %c) {
; CHECK-LABEL: @low_accuracy_no_fastmath
; CHECK: call svml_cc <2 x double> @__svml_ceil2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_ceil4(<4 x double> %broadcast.splat) #2
  ret void
}

define void @low_accuracy_fastmath(double %c) {
; CHECK-LABEL: @low_accuracy_fastmath
; CHECK: call fast svml_cc <2 x double> @__svml_ceil2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_ceil4(<4 x double> %broadcast.splat) #2
  ret void
}

define void @medium_accuracy_no_fastmath(double %c) {
; CHECK-LABEL: @medium_accuracy_no_fastmath
; CHECK: call svml_cc <2 x double> @__svml_ceil2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_ceil4(<4 x double> %broadcast.splat) #3
  ret void
}

define void @medium_accuracy_fastmath(double %c) {
; CHECK-LABEL: @medium_accuracy_fastmath
; CHECK: call fast svml_cc <2 x double> @__svml_ceil2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_ceil4(<4 x double> %broadcast.splat) #3
  ret void
}

define void @high_accuracy_no_fastmath(double %c) {
; CHECK-LABEL: @high_accuracy_no_fastmath
; CHECK: call svml_cc <2 x double> @__svml_ceil2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_ceil4(<4 x double> %broadcast.splat) #4
  ret void
}

define void @high_accuracy_fastmath(double %c) {
; CHECK-LABEL: @high_accuracy_fastmath
; CHECK: call fast svml_cc <2 x double> @__svml_ceil2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_ceil4(<4 x double> %broadcast.splat) #4
  ret void
}

declare <4 x double> @__svml_ceil4(<4 x double>)

attributes #1 = { nounwind }
attributes #2 = { nounwind "imf-precision"="low" }
attributes #3 = { nounwind "imf-precision"="medium" }
attributes #4 = { nounwind "imf-precision"="high" }
