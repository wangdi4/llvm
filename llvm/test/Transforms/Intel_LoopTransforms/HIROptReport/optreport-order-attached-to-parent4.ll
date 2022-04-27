; Check that proper optreport order (structure and metadata) for deleted loops (Completely Unrolled) is attached to a parent loop.

;void foo(int *restrict A, int *restrict B, int *restrict C, int N) {
;
;  for (int j = 0; j < N; ++j) {
;    for (int i = 0; i < 10; ++i) {
;      B[i] = i;
;    }
;    for (int i = 0; i < 10; ++i) {
;      A[i] = i;
;    }
;    for (int i = 0; i < 10; ++i) {
;      C[i] = i;
;    }
;  }
;  return;
;}

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT:     LOOP END
; OPTREPORT-NEXT: LOOP END

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{[[M1]], [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.optreport.rootnode", [[M3:!.*]]}
; CHECK: [[M3]] = distinct !{!"intel.optreport", [[M4:!.*]]}
; CHECK: [[M4]] = !{!"intel.optreport.first_child", [[M5:!.*]]}
; CHECK: [[M5]] = distinct !{!"intel.optreport.rootnode", [[M6:!.*]]}
; CHECK: [[M6]] = distinct !{!"intel.optreport", [[M7:!.*]], [[M9:!.*]]}
; CHECK: [[M7]] = !{!"intel.optreport.remarks", [[M8:!.*]]}
; CHECK: [[M8]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 10}
; CHECK: [[M9]] = !{!"intel.optreport.next_sibling", [[M10:!.*]]}
; CHECK: [[M10]] = distinct !{!"intel.optreport.rootnode", [[M11:!.*]]}
; CHECK: [[M11]] = distinct !{!"intel.optreport", [[M7]], [[M12:!.*]]}
; CHECK: [[M12]] = !{!"intel.optreport.next_sibling", [[M13:!.*]]}
; CHECK: [[M13]] = distinct !{!"intel.optreport.rootnode", [[M14:!.*]]}
; CHECK: [[M14]] = distinct !{!"intel.optreport", [[M7]]}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* noalias nocapture %B, i32* noalias nocapture %C, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp18 = icmp sgt i32 %N, 0
  br i1 %cmp18, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup18
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.cond.cleanup18, %for.body.lr.ph
  %j.019 = phi i32 [ 0, %for.body.lr.ph ], [ %inc26, %for.cond.cleanup18 ]
  br label %for.body4

for.cond.cleanup3:                                ; preds = %for.body4
  br label %for.body9

for.body4:                                        ; preds = %for.body4, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body4 ]
  %arrayidx = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup3, label %for.body4

for.cond.cleanup8:                                ; preds = %for.body9
  br label %for.body19

for.body9:                                        ; preds = %for.body9, %for.cond.cleanup3
  %indvars.iv20 = phi i64 [ 0, %for.cond.cleanup3 ], [ %indvars.iv.next21, %for.body9 ]
  %arrayidx11 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv20
  %1 = trunc i64 %indvars.iv20 to i32
  store i32 %1, i32* %arrayidx11, align 4, !tbaa !2
  %indvars.iv.next21 = add nuw nsw i64 %indvars.iv20, 1
  %exitcond22 = icmp eq i64 %indvars.iv.next21, 10
  br i1 %exitcond22, label %for.cond.cleanup8, label %for.body9

for.cond.cleanup18:                               ; preds = %for.body19
  %inc26 = add nuw nsw i32 %j.019, 1
  %exitcond26 = icmp eq i32 %inc26, %N
  br i1 %exitcond26, label %for.cond.cleanup.loopexit, label %for.body

for.body19:                                       ; preds = %for.body19, %for.cond.cleanup8
  %indvars.iv23 = phi i64 [ 0, %for.cond.cleanup8 ], [ %indvars.iv.next24, %for.body19 ]
  %arrayidx21 = getelementptr inbounds i32, i32* %C, i64 %indvars.iv23
  %2 = trunc i64 %indvars.iv23 to i32
  store i32 %2, i32* %arrayidx21, align 4, !tbaa !2
  %indvars.iv.next24 = add nuw nsw i64 %indvars.iv23, 1
  %exitcond25 = icmp eq i64 %indvars.iv.next24, 10
  br i1 %exitcond25, label %for.cond.cleanup18, label %for.body19
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
