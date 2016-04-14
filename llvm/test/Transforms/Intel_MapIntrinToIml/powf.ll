; Check to see that the llvm.pow.v4f32 intrinsic is translated to svml.

; RUN: opt -iml-trans -S < %s | FileCheck %s

; CHECK-LABEL: @foo
; CHECK: call <4 x float> @__svml_powf4
; CHECK: ret

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(float* nocapture %array, float* noalias nocapture readonly %array2, float* noalias nocapture readonly %array3) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = getelementptr inbounds float, float* %array2, i64 %index
  %1 = bitcast float* %0 to <4 x float>*
  %wide.load = load <4 x float>, <4 x float>* %1, align 4
  %2 = getelementptr inbounds float, float* %array3, i64 %index
  %3 = bitcast float* %2 to <4 x float>*
  %wide.load11 = load <4 x float>, <4 x float>* %3, align 4
  %4 = call <4 x float> @llvm.pow.v4f32(<4 x float> %wide.load, <4 x float> %wide.load11)
  %5 = getelementptr inbounds float, float* %array, i64 %index
  %6 = bitcast float* %5 to <4 x float>*
  store <4 x float> %4, <4 x float>* %6, align 4
  %index.next = add i64 %index, 4
  %7 = icmp eq i64 %index.next, 1000
  br i1 %7, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; Function Attrs: norecurse nounwind readnone uwtable
define i32 @main() #1 {
entry:
  ret i32 0
}

; Function Attrs: nounwind readnone
declare <4 x float> @llvm.pow.v4f32(<4 x float>, <4 x float>) #2

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { norecurse nounwind readnone uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }
