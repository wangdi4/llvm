; Check IMF precision attribute is handled correctly, and make sure high
; accuracy (medium in fast math) is used by default.

; RUN: opt -enable-new-pm=0 -mtriple=x86_64-unknown-linux-gnu -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,HAS-REF
; RUN: opt -enable-new-pm=0 -mtriple=i386-unknown-linux-gnu -mattr=+sse2 -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,NO-REF
; RUN: opt -enable-new-pm=0 -mtriple=x86_64-pc-windows-coff -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,NO-REF
; RUN: opt -enable-new-pm=0 -mtriple=i386-pc-windows-coff -mattr=+sse2 -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s --check-prefixes=CHECK,NO-REF

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

define void @default_accuracy_no_fastmath(double %c) {
; CHECK-LABEL: @default_accuracy_no_fastmath
; CHECK: call svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #1
  ret void
}

define void @default_accuracy_fastmath(double %c) {
; CHECK-LABEL: @default_accuracy_fastmath
; CHECK: call fast svml_cc <2 x double> @__svml_exp2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #1
  ret void
}

define void @low_accuracy_no_fastmath(double %c) {
; CHECK-LABEL: @low_accuracy_no_fastmath
; CHECK: call svml_cc <2 x double> @__svml_exp2_ep(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #2
  ret void
}

define void @low_accuracy_fastmath(double %c) {
; CHECK-LABEL: @low_accuracy_fastmath
; CHECK: call fast svml_cc <2 x double> @__svml_exp2_ep(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #2
  ret void
}

define void @medium_accuracy_no_fastmath(double %c) {
; CHECK-LABEL: @medium_accuracy_no_fastmath
; CHECK: call svml_cc <2 x double> @__svml_exp2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #3
  ret void
}

define void @medium_accuracy_fastmath(double %c) {
; CHECK-LABEL: @medium_accuracy_fastmath
; CHECK: call fast svml_cc <2 x double> @__svml_exp2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #3
  ret void
}

define void @high_accuracy_no_fastmath(double %c) {
; CHECK-LABEL: @high_accuracy_no_fastmath
; CHECK: call svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #4
  ret void
}

define void @high_accuracy_fastmath(double %c) {
; CHECK-LABEL: @high_accuracy_fastmath
; CHECK: call fast svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #4
  ret void
}

define void @reference_accuracy_no_fastmath(double %c) {
; CHECK-LABEL: @reference_accuracy_no_fastmath
; HAS-REF: call svml_cc <2 x double> @__svml_exp2_rf_ex(
; NO-REF: call svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #5
  ret void
}

define void @reference_accuracy_fastmath(double %c) {
; CHECK-LABEL: @reference_accuracy_fastmath
; HAS-REF: call fast svml_cc <2 x double> @__svml_exp2_rf_ex(
; NO-REF: call fast svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call fast svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #5
  ret void
}

define void @default_accuracy_afn(double %c) {
; CHECK-LABEL: @default_accuracy_afn
; CHECK: call afn svml_cc <2 x double> @__svml_exp2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call afn svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #1
  ret void
}

define void @low_accuracy_afn(double %c) {
; CHECK-LABEL: @low_accuracy_afn
; CHECK: call afn svml_cc <2 x double> @__svml_exp2_ep(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call afn svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #2
  ret void
}

define void @medium_accuracy_afn(double %c) {
; CHECK-LABEL: @medium_accuracy_afn
; CHECK: call afn svml_cc <2 x double> @__svml_exp2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call afn svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #3
  ret void
}

define void @high_accuracy_afn(double %c) {
; CHECK-LABEL: @high_accuracy_afn
; CHECK: call afn svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call afn svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #4
  ret void
}

define void @reference_accuracy_afn(double %c) {
; CHECK-LABEL: @reference_accuracy_afn
; HAS-REF: call afn svml_cc <2 x double> @__svml_exp2_rf_ex(
; NO-REF: call afn svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call afn svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #5
  ret void
}

define void @default_accuracy_noafn(double %c) {
; CHECK-LABEL: @default_accuracy_noafn
; CHECK: call reassoc nnan ninf nsz arcp contract svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call reassoc nnan ninf nsz arcp contract svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #1
  ret void
}

define void @low_accuracy_noafn(double %c) {
; CHECK-LABEL: @low_accuracy_noafn
; CHECK: call reassoc nnan ninf nsz arcp contract svml_cc <2 x double> @__svml_exp2_ep(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call reassoc nnan ninf nsz arcp contract svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #2
  ret void
}

define void @medium_accuracy_noafn(double %c) {
; CHECK-LABEL: @medium_accuracy_noafn
; CHECK: call reassoc nnan ninf nsz arcp contract svml_cc <2 x double> @__svml_exp2(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call reassoc nnan ninf nsz arcp contract svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #3
  ret void
}

define void @high_accuracy_noafn(double %c) {
; CHECK-LABEL: @high_accuracy_noafn
; CHECK: call reassoc nnan ninf nsz arcp contract svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call reassoc nnan ninf nsz arcp contract svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #4
  ret void
}

define void @reference_accuracy_noafn(double %c) {
; CHECK-LABEL: @reference_accuracy_noafn
; HAS-REF: call reassoc nnan ninf nsz arcp contract svml_cc <2 x double> @__svml_exp2_rf_ex(
; NO-REF: call reassoc nnan ninf nsz arcp contract svml_cc <2 x double> @__svml_exp2_ha(
  %broadcast.splatinsert = insertelement <4 x double> undef, double %c, i32 0
  %broadcast.splat = shufflevector <4 x double> %broadcast.splatinsert, <4 x double> undef, <4 x i32> zeroinitializer
  %vec_call = call reassoc nnan ninf nsz arcp contract svml_cc <4 x double> @__svml_exp4(<4 x double> %broadcast.splat) #5
  ret void
}

declare <4 x double> @__svml_exp4(<4 x double>)

attributes #1 = { nounwind }
attributes #2 = { nounwind "imf-precision"="low" }
attributes #3 = { nounwind "imf-precision"="medium" }
attributes #4 = { nounwind "imf-precision"="high" }
attributes #5 = { nounwind "imf-precision"="reference" }
