; Check that proper optreport (structure and metadata) is emitted for Vectorization with Remainder when verbosity level is set to medium.

;void foo(int *restrict A, int* G, int N) {
;
;  for (int j = 0; j < 3; ++j) {
;    for (int i = 0; i < N; ++i) {
;      A[i] += G[j];
;    }
;  }
;  return;
;}

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -hir-cg -intel-loop-optreport=medium -simplifycfg -intel-ir-optreport-emitter %s 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -hir-optreport-emitter -hir-cg -intel-loop-optreport=medium %s 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         Remark: LOOP WAS VECTORIZED
; OPTREPORT-NEXT:         Remark: vectorization support: vector length {{.*}}
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         <Remainder loop for vectorization>
; OPTREPORT-NEXT:         Remark: remainder loop was not vectorized:
; OPTREPORT-NEXT:     LOOP END
; OPTREPORT-NEXT: LOOP END

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -VPlanDriverHIR -vplan-force-vf=4 -hir-cg -intel-loop-optreport=medium < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{!"llvm.loop.optreport", [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.loop.optreport", [[M3:!.*]]}
; CHECK: [[M3]] = !{!"intel.optreport.remarks", [[M4:!.*]], [[M5:!.*]]}
; CHECK: [[M4]] = !{!"intel.optreport.remark", !"LOOP WAS VECTORIZED"}
; CHECK: [[M5]] = !{!"intel.optreport.remark", !"vectorization support: vector length %s", {{.*}}}
; CHECK: [[M6:!.*]] = distinct !{[[M6]]{{.*}}[[M7:!.*]]{{.*}}}
; CHECK: [[M7]] = distinct !{!"llvm.loop.optreport", [[M8:!.*]]}
; CHECK: [[M8]] = distinct !{!"intel.loop.optreport", [[M9:!.*]], [[M11:!.*]]}
; CHECK: [[M9]] = !{!"intel.optreport.origin", [[M10:!.*]]}
; CHECK: [[M10]] = !{!"intel.optreport.remark", !"Remainder loop for vectorization"}
; CHECK: [[M11]] = !{!"intel.optreport.remarks", [[M12:!.*]]}
; CHECK: [[M12]] = !{!"intel.optreport.remark", !"remainder loop was not vectorized: %s ", {{.*}}}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* nocapture readonly %G, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp27 = icmp sgt i32 %N, 0
  br i1 %cmp27, label %entry.split.us, label %for.cond.cleanup

entry.split.us:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body.us

for.body.us:                                      ; preds = %for.cond1.for.cond.cleanup3_crit_edge.us, %entry.split.us
  %indvars.iv11 = phi i64 [ %indvars.iv.next12, %for.cond1.for.cond.cleanup3_crit_edge.us ], [ 0, %entry.split.us ]
  %arrayidx.us = getelementptr inbounds i32, i32* %G, i64 %indvars.iv11
  %0 = load i32, i32* %arrayidx.us, align 4, !tbaa !2
  br label %for.body4.us

for.body4.us:                                     ; preds = %for.body4.us, %for.body.us
  %indvars.iv = phi i64 [ 0, %for.body.us ], [ %indvars.iv.next, %for.body4.us ]
  %arrayidx6.us = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx6.us, align 4, !tbaa !2
  %add.us = add nsw i32 %1, %0
  store i32 %add.us, i32* %arrayidx6.us, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond1.for.cond.cleanup3_crit_edge.us, label %for.body4.us

for.cond1.for.cond.cleanup3_crit_edge.us:         ; preds = %for.body4.us
  %indvars.iv.next12 = add nuw nsw i64 %indvars.iv11, 1
  %exitcond13 = icmp eq i64 %indvars.iv.next12, 3
  br i1 %exitcond13, label %for.cond.cleanup.loopexit, label %for.body.us

for.cond.cleanup.loopexit:                        ; preds = %for.cond1.for.cond.cleanup3_crit_edge.us
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
