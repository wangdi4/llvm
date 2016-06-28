; Check to see that __svml_erfcinvf4 is translated to the high accuracy svml variant.

; RUN: opt -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: call <4 x float> @__svml_erfcinvf4_ha
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @vector_foo(float* nocapture %varray) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %broadcast.splatinsert = insertelement <4 x i64> undef, i64 %index, i32 0
  %broadcast.splat = shufflevector <4 x i64> %broadcast.splatinsert, <4 x i64> undef, <4 x i32> zeroinitializer
  %0 = add <4 x i64> %broadcast.splat, <i64 1, i64 2, i64 3, i64 4>
  %1 = trunc <4 x i64> %0 to <4 x i32>
  %2 = sitofp <4 x i32> %1 to <4 x float>
  %3 = fmul fast <4 x float> %2, <float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000, float 0x3F50624DE0000000>
  %4 = call fast <4 x float> @__svml_erfcinvf4(<4 x float> %3)
  %5 = getelementptr inbounds float, float* %varray, i64 %index
  %6 = bitcast float* %5 to <4 x float>*
  store <4 x float> %4, <4 x float>* %6, align 4
  %index.next = add i64 %index, 4
  %7 = icmp eq i64 %index.next, 1000
  br i1 %7, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; Function Attrs: nounwind readnone
declare <4 x float> @__svml_erfcinvf4(<4 x float>) #7

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #7 = { nounwind readnone }
