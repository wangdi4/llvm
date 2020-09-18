; Check to see that __svml_expm1f is translated to the medium accuracy svml variant.

; RUN: opt -vector-library=SVML -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: call fast svml_cc <4 x float> @__svml_expm1f4(
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @vector_foo(float* nocapture %varray, float* nocapture readonly %iarray) #0 {
entry:
  %scevgep = getelementptr float, float* %varray, i64 999
  %scevgep11 = getelementptr float, float* %iarray, i64 999
  %bound0 = icmp uge float* %scevgep11, %varray
  %bound1 = icmp uge float* %scevgep, %iarray
  %memcheck.conflict = and i1 %bound0, %bound1
  br i1 %memcheck.conflict, label %for.body, label %vector.body

vector.body:                                      ; preds = %entry, %vector.body
  %index = phi i64 [ %index.next, %vector.body ], [ 0, %entry ]
  %0 = getelementptr inbounds float, float* %iarray, i64 %index
  %1 = bitcast float* %0 to <4 x float>*
  %wide.load = load <4 x float>, <4 x float>* %1, align 4
  %2 = call fast <4 x float> @__svml_expm1f4(<4 x float> %wide.load)
  %3 = getelementptr inbounds float, float* %varray, i64 %index
  %4 = bitcast float* %3 to <4 x float>*
  store <4 x float> %2, <4 x float>* %4, align 4
  %index.next = add i64 %index, 4
  %5 = icmp eq i64 %index.next, 1000
  br i1 %5, label %for.end, label %vector.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds float, float* %iarray, i64 %indvars.iv
  %6 = load float, float* %arrayidx, align 4
  %call = tail call fast float @expm1f(float %6) #6
  %arrayidx2 = getelementptr inbounds float, float* %varray, i64 %indvars.iv
  store float %call, float* %arrayidx2, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %vector.body
  ret void
}

; Function Attrs: nounwind readnone
declare float @expm1f(float) #2

; Function Attrs: nounwind readnone
declare <4 x float> @__svml_expm1f4(<4 x float>) #6

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { nounwind readnone }
