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

; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter" -vplan-force-vf=4 -intel-opt-report=medium %s 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-optreport-emitter,hir-cg" -vplan-force-vf=4 -intel-opt-report=medium %s 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #15553: loop was not vectorized: outer loop is not an auto-vectorization candidate.{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         remark #15300: LOOP WAS VECTORIZED
; OPTREPORT-NEXT:         remark #15305: vectorization support: vector length {{.*}}
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         <Remainder loop for vectorization>
; OPTREPORT-NEXT:     LOOP END
; OPTREPORT-NEXT: LOOP END

; TODO: There is still a small issue where the merged CFG-based HIR causes
; the opt report layout to mismatch slightly with the loop layout.
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,hir-cg" -vplan-force-vf=4 -intel-opt-report=medium < %s -S | FileCheck %s --check-prefix=MERGED-CFG

; MERGED-CFG: [[M1:!.*]] = distinct !{!"intel.optreport", [[M3:!.*]]}
; MERGED-CFG: [[M3]] = !{!"intel.optreport.remarks", [[M4:!.*]], [[M5:!.*]]}
; MERGED-CFG: [[M4]] = !{!"intel.optreport.remark", i32 15300}
; MERGED-CFG: [[M5]] = !{!"intel.optreport.remark", i32 15305, {{.*}}}
; MERGED-CFG: [[OUTER_LOOP:!.*]] = distinct !{[[OUTER_LOOP]], [[OM1:!.*]]}
; MERGED-CFG: [[OM1]] = distinct !{!"intel.optreport", [[OM3:!.*]]}
; MERGED-CFG: [[OM3]] = !{!"intel.optreport.remarks", [[OM4:!.*]]}
; MERGED-CFG: [[OM4]] = !{!"intel.optreport.remark", i32 15553}
; MERGED-CFG: [[M6:!.*]] = distinct !{[[M6]]{{.*}}[[M7:!.*]]{{.*}}}
; MERGED-CFG: [[M7]] = distinct !{!"intel.optreport", [[M10:!.*]]}
; MERGED-CFG: [[M10]] = !{!"intel.optreport.origin", [[M11:!.*]]}
; MERGED-CFG: [[M11]] = !{!"intel.optreport.remark", i32 25519}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias nocapture %A, ptr nocapture readonly %G, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp27 = icmp sgt i32 %N, 0
  br i1 %cmp27, label %entry.split.us, label %for.cond.cleanup

entry.split.us:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body.us

for.body.us:                                      ; preds = %for.cond1.for.cond.cleanup3_crit_edge.us, %entry.split.us
  %indvars.iv11 = phi i64 [ %indvars.iv.next12, %for.cond1.for.cond.cleanup3_crit_edge.us ], [ 0, %entry.split.us ]
  %arrayidx.us = getelementptr inbounds i32, ptr %G, i64 %indvars.iv11
  %0 = load i32, ptr %arrayidx.us, align 4, !tbaa !2
  br label %for.body4.us

for.body4.us:                                     ; preds = %for.body4.us, %for.body.us
  %indvars.iv = phi i64 [ 0, %for.body.us ], [ %indvars.iv.next, %for.body4.us ]
  %arrayidx6.us = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  %1 = load i32, ptr %arrayidx6.us, align 4, !tbaa !2
  %add.us = add nsw i32 %1, %0
  store i32 %add.us, ptr %arrayidx6.us, align 4, !tbaa !2
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
