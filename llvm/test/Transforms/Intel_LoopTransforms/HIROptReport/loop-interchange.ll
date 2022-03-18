; Check that proper optreport (structure and metadata) is emitted for Loop Interchange.

;void foo(int **restrict A, int N) {
;
;  for (int j = 0; j < N; ++j) {
;    for (int i = 0; i < N; ++i) {
;      A[i][j] = j+i;
;    }
;  }
;  return;
;}

; RUN: opt -hir-ssa-deconstruction -hir-loop-interchange -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
; RUN: opt -hir-ssa-deconstruction -hir-loop-interchange -hir-optreport-emitter -hir-cg -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #25444: Loopnest Interchanged: ( 1 2 ) --> ( 2 1 ){{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:     LOOP END
; OPTREPORT-NEXT: LOOP END

; RUN: opt -hir-ssa-deconstruction -hir-loop-interchange -hir-cg -intel-opt-report=low < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{[[M1]], [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.optreport.rootnode", [[M3:!.*]]}
; CHECK: [[M3]] = distinct !{!"intel.optreport", [[M4:!.*]]}
; CHECK: [[M4]] = !{!"intel.optreport.remarks", [[M5:!.*]]}
; CHECK: [[M5]] = !{!"intel.optreport.remark", i32 25444, !"Loopnest Interchanged: %s", !"( 1 2 ) --> ( 2 1 )"}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32** noalias nocapture readonly %A, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp21 = icmp sgt i32 %N, 0
  br i1 %cmp21, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.body.lr.ph, %for.cond.cleanup3
  %indvars.iv25 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next26, %for.cond.cleanup3 ]
  br label %for.body4

for.body4:                                        ; preds = %for.body4, %for.body4.lr.ph
  %indvars.iv = phi i64 [ 0, %for.body4.lr.ph ], [ %indvars.iv.next, %for.body4 ]
  %0 = add nuw nsw i64 %indvars.iv, %indvars.iv25
  %arrayidx = getelementptr inbounds i32*, i32** %A, i64 %indvars.iv
  %1 = load i32*, i32** %arrayidx, align 8, !tbaa !2
  %arrayidx6 = getelementptr inbounds i32, i32* %1, i64 %indvars.iv25
  %2 = trunc i64 %0 to i32
  store i32 %2, i32* %arrayidx6, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  %indvars.iv.next26 = add nuw nsw i64 %indvars.iv25, 1
  %exitcond28 = icmp eq i64 %indvars.iv.next26, %wide.trip.count
  br i1 %exitcond28, label %for.cond.cleanup.loopexit, label %for.body4.lr.ph

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup3
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPi", !4, i64 0}
