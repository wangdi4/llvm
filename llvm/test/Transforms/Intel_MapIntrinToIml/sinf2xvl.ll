; Check to see that __svml_sinf8 is translated to two __svml_sinf4 medium accuracy variant calls.

; RUN: opt -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: call svml_cc <4 x float> @__svml_sinf4(
; CHECK: call svml_cc <4 x float> @__svml_sinf4(
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
  %broadcast.splatinsert6 = insertelement <8 x i32> undef, i32 %0, i32 0
  %broadcast.splat7 = shufflevector <8 x i32> %broadcast.splatinsert6, <8 x i32> undef, <8 x i32> zeroinitializer
  %induction8 = add <8 x i32> %broadcast.splat7, <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %1 = sitofp <8 x i32> %induction8 to <8 x float>
  %2 = call fast <8 x float> @__svml_sinf8(<8 x float> %1)
  %3 = getelementptr inbounds float, float* %varray, i64 %index
  %4 = bitcast float* %3 to <8 x float>*
  store <8 x float> %2, <8 x float>* %4, align 4
  %index.next = add i64 %index, 8
  %5 = icmp eq i64 %index.next, 1000
  br i1 %5, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

declare <8 x float> @__svml_sinf8(<8 x float>) #2

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { nounwind readnone }
