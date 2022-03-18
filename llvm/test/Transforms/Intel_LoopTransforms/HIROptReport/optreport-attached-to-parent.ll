; Check that optreport (structure and metadata) for a deleted loop (Completely Unrolled) is attached to a parent loop (first_child field).

;int foo(int **restrict B, int N) {
;
;  int sum =0;
;  for (int j = 0 ; j < N; ++j) {
;    sum += j+1;
;    for (int i = 0; i < 10; ++i) {
;      B[j][i] = i;
;    }
;  }
;  return sum;
;}

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
;
; RUN: opt -opaque-pointers -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
;
; OPTREPORT: LOOP BEGIN{{[[:space:]]}}
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
; CHECK: [[M6]] = distinct !{!"intel.optreport", [[M7:!.*]]}
; CHECK: [[M7]] = !{!"intel.optreport.remarks", [[M8:!.*]]}
; CHECK: [[M8]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 10}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32** noalias nocapture readonly %B, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp12 = icmp sgt i32 %N, 0
  br i1 %cmp12, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup4
  %add1.lcssa = phi i32 [ %add1, %for.cond.cleanup4 ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add1.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %sum.0.lcssa

for.body:                                         ; preds = %for.cond.cleanup4, %for.body.lr.ph
  %indvars.iv15 = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next16, %for.cond.cleanup4 ]
  %sum.013 = phi i32 [ 0, %for.body.lr.ph ], [ %add1, %for.cond.cleanup4 ]
  %indvars.iv.next16 = add nuw nsw i64 %indvars.iv15, 1
  %arrayidx = getelementptr inbounds i32*, i32** %B, i64 %indvars.iv15
  %0 = load i32*, i32** %arrayidx, align 8, !tbaa !2
  br label %for.body5

for.cond.cleanup4:                                ; preds = %for.body5
  %1 = trunc i64 %indvars.iv.next16 to i32
  %add1 = add nsw i32 %sum.013, %1
  %exitcond17 = icmp eq i64 %indvars.iv.next16, %wide.trip.count
  br i1 %exitcond17, label %for.cond.cleanup.loopexit, label %for.body

for.body5:                                        ; preds = %for.body5, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx7 = getelementptr inbounds i32, i32* %0, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %arrayidx7, align 4, !tbaa !6
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
