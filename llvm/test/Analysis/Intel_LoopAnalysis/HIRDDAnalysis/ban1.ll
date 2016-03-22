;  for (long int i = 1; i <= 10; i++) 
;    for (long int j = 1; j <= 10; j++) 
;      A[10*i + j] = A[10*i + j - 1] ;;

; RUN:  opt < %s  -loop-simplify  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' for function 'sub8'
; CHECK-DAG:  FLOW (<= <>)

; ModuleID = 'ban1.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @sub8(i64 %n) #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.cond.cleanup.3, %entry
  %i.020 = phi i64 [ 1, %entry ], [ %inc9, %for.cond.cleanup.3 ]
  %mul = mul nuw nsw i64 %i.020, 10
  br label %for.body.4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.3
  ret void

for.cond.cleanup.3:                               ; preds = %for.body.4
  %inc9 = add nuw nsw i64 %i.020, 1
  %exitcond22 = icmp eq i64 %inc9, 11
  br i1 %exitcond22, label %for.cond.cleanup, label %for.cond.1.preheader

for.body.4:                                       ; preds = %for.body.4, %for.cond.1.preheader
  %j.019 = phi i64 [ 1, %for.cond.1.preheader ], [ %inc, %for.body.4 ]
  %add = add nuw nsw i64 %j.019, %mul
  %sub = add nsw i64 %add, -1
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %sub
  %0 = bitcast float* %arrayidx to i32*
  %1 = load i32, i32* %0, align 4, !tbaa !1
  %arrayidx7 = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %add
  %2 = bitcast float* %arrayidx7 to i32*
  store i32 %1, i32* %2, align 4, !tbaa !1
  %inc = add nuw nsw i64 %j.019, 1
  %exitcond = icmp eq i64 %inc, 11
  br i1 %exitcond, label %for.cond.cleanup.3, label %for.body.4
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.7.0 (trunk 1049) (llvm/branches/loopopt 1130)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}


