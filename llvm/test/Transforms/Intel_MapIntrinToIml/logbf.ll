; __svml_logbf4 does not have an _ha variant. The iml query results in the same variant function name when using attributes of max-error=0.5 and precision="high". This
; variant is defined to be a correctly rounded version of the function, but for some reason the function name is not appended with _cr.

; RUN: opt -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: call <4 x float> @__svml_logbf4
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
  %0 = sub <4 x i64> <i64 0, i64 -1, i64 -2, i64 -3>, %broadcast.splat
  %1 = trunc <4 x i64> %0 to <4 x i32>
  %2 = sitofp <4 x i32> %1 to <4 x float>
  %3 = fadd fast <4 x float> %2, <float -1.000000e+00, float -1.000000e+00, float -1.000000e+00, float -1.000000e+00>
  %4 = call fast <4 x float> @__svml_logbf4(<4 x float> %3)
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
declare <4 x float> @__svml_logbf4(<4 x float>) #6

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { nounwind readnone }
