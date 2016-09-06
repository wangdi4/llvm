; Check to see that __svml_sinf2 is translated to a __svml_sinf4 high accuracy variant call. For this test, also check to see
; that the lower half elements are repeated in the upper half of the vector and that only the lower two elements are selected
; from the call result vector.

; RUN: opt -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK shufflevector <2 x float> %{{.*}}, <2 x float> undef, <4 x i32> <i32 0, i32 1, i32 0, i32 1>
; CHECK call <4 x float> @__svml_sinf4_ha
; CHECK shufflevector <4 x float> %{{.*}}, <4 x float> undef, <2 x i32> <i32 0, i32 1>
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
  %broadcast.splatinsert6 = insertelement <2 x i32> undef, i32 %0, i32 0
  %broadcast.splat7 = shufflevector <2 x i32> %broadcast.splatinsert6, <2 x i32> undef, <2 x i32> zeroinitializer
  %induction8 = add <2 x i32> %broadcast.splat7, <i32 0, i32 1>
  %1 = sitofp <2 x i32> %induction8 to <2 x float>
  %2 = call fast <2 x float> @__svml_sinf2(<2 x float> %1)
  %3 = getelementptr inbounds float, float* %varray, i64 %index
  %4 = bitcast float* %3 to <2 x float>*
  store <2 x float> %2, <2 x float>* %4, align 4
  %index.next = add i64 %index, 2
  %5 = icmp eq i64 %index.next, 1000
  br i1 %5, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; Function Attrs: nounwind readnone
declare <2 x float> @__svml_sinf2(<2 x float>) #2

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
