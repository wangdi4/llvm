; RUN: llc <%s | FileCheck --implicit-check-not pack %s
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

; Function Attrs: nounwind uwtable
define dso_local <2 x float> @RunReStream() {
entry:
; CHECK-LABEL: RunReStream
; CHECK: mulf32x2
; CHECK-SAME: 0x3e9e01b3
; CHECK-SAME: 0x3f0000003e800000
  %a86 = tail call <2 x float> @llvm.csa.mulf32x2(<2 x float> <float 0x3FD3C03660000000, float undef>, <2 x float> <float 0.25, float 0.5>, i8 0, i8 0, i8 1)
  ret <2 x float> %a86
}

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local <2 x float> @reduc(<2 x float>* nocapture readonly %vals) local_unnamed_addr #1 {
entry:
; CHECK-LABEL: reduc
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %sum.06 = phi <2 x float> [ zeroinitializer, %entry ], [ %add, %for.body ]
; CHECK: pick64
; CHECK-SAME: , 0
  %arrayidx = getelementptr inbounds <2 x float>, <2 x float>* %vals, i64 %indvars.iv
  %0 = load <2 x float>, <2 x float>* %arrayidx, align 8
  %add = fadd <2 x float> %sum.06, %0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  ret <2 x float> %add
}

; Function Attrs: nounwind readnone
declare <2 x float> @llvm.csa.mulf32x2(<2 x float>, <2 x float>, i8, i8, i8) #0

attributes #0 = { nounwind readnone }
attributes #1 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="64" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
