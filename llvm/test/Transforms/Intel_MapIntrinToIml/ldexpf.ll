; Check to see that SVML ldexp/ldexpf functions are preserved.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @vector_ldexpf
; CHECK: call svml_cc <4 x float> @__svml_ldexpf4
; CHECK: ret
define void @vector_ldexpf(float* nocapture %array, float* noalias nocapture readonly %array2, i32* noalias nocapture readonly %array3) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = getelementptr inbounds float, float* %array2, i64 %index
  %1 = bitcast float* %0 to <4 x float>*
  %wide.load = load <4 x float>, <4 x float>* %1, align 4
  %2 = getelementptr inbounds i32, i32* %array3, i64 %index
  %3 = bitcast i32* %2 to <4 x i32>*
  %wide.load11 = load <4 x i32>, <4 x i32>* %3, align 4
  %4 = call <4 x float> @__svml_ldexpf4(<4 x float> %wide.load, <4 x i32> %wide.load11)
  %5 = getelementptr inbounds float, float* %array, i64 %index
  %6 = bitcast float* %5 to <4 x float>*
  store <4 x float> %4, <4 x float>* %6, align 4
  %index.next = add i64 %index, 4
  %7 = icmp eq i64 %index.next, 1000
  br i1 %7, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_ldexp
; CHECK: call svml_cc <2 x double> @__svml_ldexp2
; CHECK: ret
define void @vector_ldexp(double* nocapture %array, double* noalias nocapture readonly %array2, i32* noalias nocapture readonly %array3) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = getelementptr inbounds double, double* %array2, i64 %index
  %1 = bitcast double* %0 to <2 x double>*
  %wide.load = load <2 x double>, <2 x double>* %1, align 4
  %2 = getelementptr inbounds i32, i32* %array3, i64 %index
  %3 = bitcast i32* %2 to <2 x i32>*
  %wide.load11 = load <2 x i32>, <2 x i32>* %3, align 4
  %4 = call <2 x double> @__svml_ldexp2(<2 x double> %wide.load, <2 x i32> %wide.load11)
  %5 = getelementptr inbounds double, double* %array, i64 %index
  %6 = bitcast double* %5 to <2 x double>*
  store <2 x double> %4, <2 x double>* %6, align 4
  %index.next = add i64 %index, 2
  %7 = icmp eq i64 %index.next, 1000
  br i1 %7, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

declare <4 x float> @__svml_ldexpf4(<4 x float>, <4 x i32>) #1
declare <2 x double> @__svml_ldexp2(<2 x double>, <2 x i32>) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
