; Check IMF precision attribute is handled correctly, and make sure high
; accuracy (medium in fast math) is used by default.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -S -iml-trans < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @default_accuracy_no_fastmath(half %c) {
; CHECK-LABEL: @default_accuracy_no_fastmath
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call svml_cc <8 x half> @__svml_exps8_ha(<8 x half> %broadcast.splat)
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #1
  ret void
}

define void @default_accuracy_fastmath(half %c) {
; CHECK-LABEL: @default_accuracy_fastmath
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call fast svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call fast svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #1
  ret void
}

define void @low_accuracy_no_fastmath(half %c) {
; CHECK-LABEL: @low_accuracy_no_fastmath
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call svml_cc <8 x half> @__svml_exps8_ep(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #2
  ret void
}

define void @low_accuracy_fastmath(half %c) {
; CHECK-LABEL: @low_accuracy_fastmath
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call fast svml_cc <8 x half> @__svml_exps8_ep(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call fast svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #2
  ret void
}

define void @medium_accuracy_no_fastmath(half %c) {
; CHECK-LABEL: @medium_accuracy_no_fastmath
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #3
  ret void
}

define void @medium_accuracy_fastmath(half %c) {
; CHECK-LABEL: @medium_accuracy_fastmath
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call fast svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call fast svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #3
  ret void
}

define void @high_accuracy_no_fastmath(half %c) {
; CHECK-LABEL: @high_accuracy_no_fastmath
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call svml_cc <8 x half> @__svml_exps8_ha(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #4
  ret void
}

define void @high_accuracy_fastmath(half %c) {
; CHECK-LABEL: @high_accuracy_fastmath
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call fast svml_cc <8 x half> @__svml_exps8_ha(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call fast svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #4
  ret void
}

define void @default_accuracy_afn(half %c) {
; CHECK-LABEL: @default_accuracy_afn
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call afn svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call afn svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #1
  ret void
}

define void @low_accuracy_afn(half %c) {
; CHECK-LABEL: @low_accuracy_afn
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call afn svml_cc <8 x half> @__svml_exps8_ep(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call afn svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #2
  ret void
}

define void @medium_accuracy_afn(half %c) {
; CHECK-LABEL: @medium_accuracy_afn
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call afn svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call afn svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #3
  ret void
}

define void @high_accuracy_afn(half %c) {
; CHECK-LABEL: @high_accuracy_afn
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call afn svml_cc <8 x half> @__svml_exps8_ha(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call afn svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #4
  ret void
}

define void @default_accuracy_noafn(half %c) {
; CHECK-LABEL: @default_accuracy_noafn
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call reassoc nnan ninf nsz arcp contract svml_cc <8 x half> @__svml_exps8_ha(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call reassoc nnan ninf nsz arcp contract svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #1
  ret void
}

define void @low_accuracy_noafn(half %c) {
; CHECK-LABEL: @low_accuracy_noafn
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call reassoc nnan ninf nsz arcp contract svml_cc <8 x half> @__svml_exps8_ep(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call reassoc nnan ninf nsz arcp contract svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #2
  ret void
}

define void @medium_accuracy_noafn(half %c) {
; CHECK-LABEL: @medium_accuracy_noafn
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call reassoc nnan ninf nsz arcp contract svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call reassoc nnan ninf nsz arcp contract svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #3
  ret void
}

define void @high_accuracy_noafn(half %c) {
; CHECK-LABEL: @high_accuracy_noafn
; CHECK-NEXT: %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
; CHECK-NEXT: %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
; CHECK-NEXT: {{.*}} = call reassoc nnan ninf nsz arcp contract svml_cc <8 x half> @__svml_exps8_ha(<8 x half> %broadcast.splat)
; CHECK-NEXT: ret void
  %broadcast.splatinsert = insertelement <8 x half> undef, half %c, i32 0
  %broadcast.splat = shufflevector <8 x half> %broadcast.splatinsert, <8 x half> undef, <8 x i32> zeroinitializer
  %vec_call = call reassoc nnan ninf nsz arcp contract svml_cc <8 x half> @__svml_exps8(<8 x half> %broadcast.splat) #4
  ret void
}

declare <8 x half> @__svml_exps8(<8 x half>)

attributes #1 = { nounwind }
attributes #2 = { nounwind "imf-precision"="low" }
attributes #3 = { nounwind "imf-precision"="medium" }
attributes #4 = { nounwind "imf-precision"="high" }
