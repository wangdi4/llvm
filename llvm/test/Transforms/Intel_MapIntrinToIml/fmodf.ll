; __svml_fmodf4 only has a low precision svml variant, so no match during the iml query should be made. This results in scalarizing the call.

; RUN: opt -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @vector_foo
; CHECK: vector.body
; CHECK: call float @fmodf
; CHECK: call float @fmodf
; CHECK: call float @fmodf
; CHECK: call float @fmodf
; CHECK-NOT: call <4 x float> @__svml_fmodf4
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @vector_foo(float* nocapture %varray, float* nocapture readonly %numer, float* nocapture readonly %denom) #0 {
entry:
  %scevgep = getelementptr float, float* %varray, i64 999
  %scevgep14 = getelementptr float, float* %numer, i64 999
  %scevgep17 = getelementptr float, float* %denom, i64 999
  %bound0 = icmp uge float* %scevgep14, %varray
  %bound1 = icmp uge float* %scevgep, %numer
  %found.conflict = and i1 %bound0, %bound1
  %bound019 = icmp uge float* %scevgep17, %varray
  %bound120 = icmp uge float* %scevgep, %denom
  %found.conflict21 = and i1 %bound019, %bound120
  %conflict.rdx = or i1 %found.conflict, %found.conflict21
  br i1 %conflict.rdx, label %for.body, label %vector.body

vector.body:                                      ; preds = %entry, %vector.body
  %index = phi i64 [ %index.next, %vector.body ], [ 0, %entry ]
  %0 = getelementptr inbounds float, float* %numer, i64 %index
  %1 = bitcast float* %0 to <4 x float>*
  %wide.load = load <4 x float>, <4 x float>* %1, align 4
  %2 = getelementptr inbounds float, float* %denom, i64 %index
  %3 = bitcast float* %2 to <4 x float>*
  %wide.load22 = load <4 x float>, <4 x float>* %3, align 4
  %4 = call fast <4 x float> @__svml_fmodf4(<4 x float> %wide.load, <4 x float> %wide.load22)
  %5 = getelementptr inbounds float, float* %varray, i64 %index
  %6 = bitcast float* %5 to <4 x float>*
  store <4 x float> %4, <4 x float>* %6, align 4
  %index.next = add i64 %index, 4
  %7 = icmp eq i64 %index.next, 1000
  br i1 %7, label %for.end, label %vector.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %entry ]
  %arrayidx = getelementptr inbounds float, float* %numer, i64 %indvars.iv
  %8 = load float, float* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds float, float* %denom, i64 %indvars.iv
  %9 = load float, float* %arrayidx2, align 4
  %call = tail call fast float @fmodf(float %8, float %9) #6
  %arrayidx4 = getelementptr inbounds float, float* %varray, i64 %indvars.iv
  store float %call, float* %arrayidx4, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %vector.body
  ret void
}

; Function Attrs: nounwind readnone
declare float @fmodf(float, float) #2

; Function Attrs: nounwind readnone
declare <4 x float> @__svml_fmodf4(<4 x float>, <4 x float>) #6

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { nounwind readnone }
