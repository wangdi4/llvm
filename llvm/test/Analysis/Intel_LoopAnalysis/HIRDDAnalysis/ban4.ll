;  for (long int i = 0; i < 20; i++) 
;    for (long int j = 0; j < 20; j++) 
;      A[30*i + 500*j] =  A[i - 500*j + 11];

; RUN:  opt < %s  -loop-simplify  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 

; CHECK: 'HIR Data Dependence Analysis' for function 'sub8'
; CHECK-DAG: (@A)[0][i1 + -500 * i2 + 11] FLOW (<= =)
; CHECK-DAG: (@A)[0][30 * i1 + 500 * i2] OUTPUT (* *)


; ModuleID = 'ban4.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub8(i64 %n, i64 %m) #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.cond.cleanup.3, %entry
  %i.021 = phi i64 [ 0, %entry ], [ %inc10, %for.cond.cleanup.3 ]
  %sub = add nuw nsw i64 %i.021, 11
  %mul5 = mul nuw nsw i64 %i.021, 30
  br label %for.body.4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.3
  ret void

for.cond.cleanup.3:                               ; preds = %for.body.4
  %inc10 = add nuw nsw i64 %i.021, 1
  %exitcond22 = icmp eq i64 %inc10, 20
  br i1 %exitcond22, label %for.cond.cleanup, label %for.cond.1.preheader

for.body.4:                                       ; preds = %for.body.4, %for.cond.1.preheader
  %j.020 = phi i64 [ 0, %for.cond.1.preheader ], [ %inc, %for.body.4 ]
  %mul = mul nuw nsw i64 %j.020, 500
  %add = sub nsw i64 %sub, %mul
  %arrayidx = getelementptr inbounds [1000 x float], [1000 x float]* @A, i64 0, i64 %add
  %0 = bitcast float* %arrayidx to i32*
  %1 = load i32, i32* %0, align 4, !tbaa !1
  %add7 = add nuw nsw i64 %mul, %mul5
  %arrayidx8 = getelementptr inbounds [1000 x float], [1000 x float]* @A, i64 0, i64 %add7
  %2 = bitcast float* %arrayidx8 to i32*
  store i32 %1, i32* %2, align 8, !tbaa !1
  %inc = add nuw nsw i64 %j.020, 1
  %exitcond = icmp eq i64 %inc, 20
  br i1 %exitcond, label %for.cond.cleanup.3, label %for.body.4
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 1049) (llvm/branches/loopopt 1130)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

