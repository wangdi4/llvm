; Check to see that SVML pown/pownf functions are translated to the high accuracy svml variant.

; RUN: opt -enable-new-pm=0 -vector-library=SVML -iml-trans -S < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @vector_pownf
; CHECK: call svml_avx_cc <8 x float> @__svml_pownf8_ha_l9
; CHECK: ret
define void @vector_pownf(float* nocapture %array, float* noalias nocapture readonly %array2, i32* noalias nocapture readonly %array3) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = getelementptr inbounds float, float* %array2, i64 %index
  %1 = bitcast float* %0 to <8 x float>*
  %wide.load = load <8 x float>, <8 x float>* %1, align 4
  %2 = getelementptr inbounds i32, i32* %array3, i64 %index
  %3 = bitcast i32* %2 to <8 x i32>*
  %wide.load11 = load <8 x i32>, <8 x i32>* %3, align 4
  %4 = call <8 x float> @__svml_pownf8(<8 x float> %wide.load, <8 x i32> %wide.load11)
  %5 = getelementptr inbounds float, float* %array, i64 %index
  %6 = bitcast float* %5 to <8 x float>*
  store <8 x float> %4, <8 x float>* %6, align 4
  %index.next = add i64 %index, 8
  %7 = icmp eq i64 %index.next, 1000
  br i1 %7, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

; CHECK-LABEL: @vector_pown
; CHECK: call svml_avx_cc <4 x double> @__svml_pown4_ha_l9
; CHECK: ret
define void @vector_pown(double* nocapture %array, double* noalias nocapture readonly %array2, i32* noalias nocapture readonly %array3) #0 {
entry:
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %entry
  %index = phi i64 [ 0, %entry ], [ %index.next, %vector.body ]
  %0 = getelementptr inbounds double, double* %array2, i64 %index
  %1 = bitcast double* %0 to <4 x double>*
  %wide.load = load <4 x double>, <4 x double>* %1, align 4
  %2 = getelementptr inbounds i32, i32* %array3, i64 %index
  %3 = bitcast i32* %2 to <4 x i32>*
  %wide.load11 = load <4 x i32>, <4 x i32>* %3, align 4
  %4 = call <4 x double> @__svml_pown4(<4 x double> %wide.load, <4 x i32> %wide.load11)
  %5 = getelementptr inbounds double, double* %array, i64 %index
  %6 = bitcast double* %5 to <4 x double>*
  store <4 x double> %4, <4 x double>* %6, align 4
  %index.next = add i64 %index, 4
  %7 = icmp eq i64 %index.next, 1000
  br i1 %7, label %for.end, label %vector.body

for.end:                                          ; preds = %vector.body
  ret void
}

declare <8 x float> @__svml_pownf8(<8 x float>, <8 x i32>) #1
declare <4 x double> @__svml_pown4(<4 x double>, <4 x i32>) #1

attributes #0 = { norecurse nounwind uwtable "min-legal-vector-width"="256" "prefer-vector-width"="256" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
