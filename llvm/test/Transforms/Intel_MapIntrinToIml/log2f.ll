; Check to see that __svml_log2f4 is translated to the high accuracy svml variant.

; RUN: opt -iml-trans -S < %s | FileCheck %s
; RUN: opt -passes="map-intrin-to-iml" -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: call svml_cc <4 x float> @__svml_log2f4_ha
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @vector_foo(float* nocapture %varray) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = trunc i64 %index to i32
  %broadcast.splatinsert6 = insertelement <4 x i32> undef, i32 %0, i32 0
  %broadcast.splat7 = shufflevector <4 x i32> %broadcast.splatinsert6, <4 x i32> undef, <4 x i32> zeroinitializer
  %induction8 = add <4 x i32> %broadcast.splat7, <i32 0, i32 1, i32 2, i32 3>
  %1 = sitofp <4 x i32> %induction8 to <4 x float>
  %2 = fadd fast <4 x float> %1, <float 1.000000e+00, float 1.000000e+00, float 1.000000e+00, float 1.000000e+00>
  %3 = call fast <4 x float> @__svml_log2f4(<4 x float> %2)
  %4 = getelementptr inbounds float, float* %varray, i64 %index
  %5 = bitcast float* %4 to <4 x float>*
  store <4 x float> %3, <4 x float>* %5, align 4
  %index.next = add i64 %index, 4
  %6 = icmp eq i64 %index.next, 1000
  br i1 %6, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; Function Attrs: nounwind readnone
declare <4 x float> @__svml_log2f4(<4 x float>) #6

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { nounwind readnone }
