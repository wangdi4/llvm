;  for (long int i = 1; i <= n; i++) {
;      for (long int j = 1; j <= n; j++) {
;          for (long int k = 1; k <= n; k++) {
;              A[-i + k + 22 ] = A[1]; 
;
; RUN:  opt < %s  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' 
; CHECK-DAG:  ANTI (* *)
; CHECK-DAG:  FLOW (* *)
;
; ModuleID = 'RDIV.c'
source_filename = "RDIV.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [100 x float] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define void @sub8(i64 %n) #0 {
entry:
  %cmp30 = icmp slt i64 %n, 1
  br i1 %cmp30, label %for.cond.cleanup, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %0 = add i64 %n, 1
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond.cleanup3, %for.cond1.preheader.lr.ph
  %i.031 = phi i64 [ 1, %for.cond1.preheader.lr.ph ], [ %inc14, %for.cond.cleanup3 ]
  %add = sub nsw i64 22, %i.031
  br label %for.cond5.preheader

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  ret void

for.cond5.preheader:                              ; preds = %for.cond.cleanup7, %for.cond1.preheader
  %j.029 = phi i64 [ 1, %for.cond1.preheader ], [ %inc11, %for.cond.cleanup7 ]
  br label %for.body8

for.cond.cleanup3:                                ; preds = %for.cond.cleanup7
  %inc14 = add nuw nsw i64 %i.031, 1
  %exitcond33 = icmp eq i64 %inc14, %0
  br i1 %exitcond33, label %for.cond.cleanup, label %for.cond1.preheader

for.cond.cleanup7:                                ; preds = %for.body8
  %inc11 = add nuw nsw i64 %j.029, 1
  %exitcond32 = icmp eq i64 %inc11, %0
  br i1 %exitcond32, label %for.cond.cleanup3, label %for.cond5.preheader

for.body8:                                        ; preds = %for.cond5.preheader, %for.body8
  %k.027 = phi i64 [ %inc, %for.body8 ], [ 1, %for.cond5.preheader ]
  %1 = load i32, i32* bitcast (float* getelementptr inbounds ([100 x float], [100 x float]* @A, i64 0, i64 1) to i32*), align 4, !tbaa !1
  %add9 = add i64 %add, %k.027
  %arrayidx = getelementptr inbounds [100 x float], [100 x float]* @A, i64 0, i64 %add9
  %2 = bitcast float* %arrayidx to i32*
  store i32 %1, i32* %2, align 4, !tbaa !1
  %inc = add nuw nsw i64 %k.027, 1
  %exitcond = icmp eq i64 %inc, %0
  br i1 %exitcond, label %for.cond.cleanup7, label %for.body8
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12496)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"float", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
