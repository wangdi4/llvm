; Check that proper optreport order (structure and metadata) for a deleted loop (Completely Unrolled) is attached to proper loop after splitting (Vectorized loop with remainder).

;int foo(int *restrict A, int **restrict B, int N) {
;
;  int sum = 0;
;  for (int j = 0 ; j < N; ++j) {
;    for (int i = 0; i < N; ++i) {
;      sum += A[i];
;    }
;    for (int i = 0; i < 10; ++i) {
;      B[j][i] = i;
;    }
;  }
;  return sum;
;}

; TODO: Different checks are seen because of difference in ordering blocks between legacy and merged CFG-based CG. Scalar remainder
; is on false branch of its top test in merged CFG-based HIR.
; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-vec-dir-insert -hir-vplan-vec -vplan-enable-new-cfg-merge-hir=false -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter -vplan-force-vf=4 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-vec-dir-insert -hir-vplan-vec -vplan-enable-new-cfg-merge-hir -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter -vplan-force-vf=4 2>&1 < %s -S | FileCheck %s -check-prefix=MERGED-CFG-OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         remark #15300: LOOP WAS VECTORIZED
; OPTREPORT-NEXT:         remark #15305: vectorization support: vector length {{.*}}
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         <Remainder loop for vectorization>
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT:     LOOP END
; OPTREPORT-NEXT: LOOP END

; MERGED-CFG-OPTREPORT: LOOP BEGIN{{[[:space:]]}}
; MERGED-CFG-OPTREPORT-NEXT:     LOOP BEGIN
; MERGED-CFG-OPTREPORT-NEXT:         remark #15300: LOOP WAS VECTORIZED
; MERGED-CFG-OPTREPORT-NEXT:         remark #15305: vectorization support: vector length {{.*}}
; MERGED-CFG-OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; MERGED-CFG-OPTREPORT-NEXT:     LOOP BEGIN
; MERGED-CFG-OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 10
; MERGED-CFG-OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; MERGED-CFG-OPTREPORT-NEXT:     LOOP BEGIN
; MERGED-CFG-OPTREPORT-NEXT:         <Remainder loop for vectorization>
; MERGED-CFG-OPTREPORT-NEXT:     LOOP END
; MERGED-CFG-OPTREPORT-NEXT: LOOP END

; TODO: Different checks are seen because of difference in ordering blocks between legacy and merged CFG-based CG. Scalar remainder
; is on false branch of its top test in merged CFG-based HIR.
; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-vec-dir-insert -hir-vplan-vec -vplan-enable-new-cfg-merge-hir=false -hir-cg -vplan-force-vf=4 -intel-opt-report=low < %s -S | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-vec-dir-insert -hir-vplan-vec -vplan-enable-new-cfg-merge-hir=true -hir-cg -vplan-force-vf=4 -intel-opt-report=low < %s -S | FileCheck %s --check-prefix=MERGED-CFG

; CHECK: [[M1:!.*]] = distinct !{[[M1]]{{.*}}[[M2:!.*]]{{.*}}}
; CHECK: [[M2]] = distinct !{!"intel.optreport.rootnode", [[M3:!.*]]}
; CHECK: [[M3]] = distinct !{!"intel.optreport", [[M4:!.*]]}
; CHECK: [[M4]] = !{!"intel.optreport.remarks", [[M5:!.*]], [[M6:!.*]]}
; CHECK: [[M5]] = !{!"intel.optreport.remark", i32 15300, !"LOOP WAS VECTORIZED"}
; CHECK: [[M6]] = !{!"intel.optreport.remark", i32 15305, !"vectorization support: vector length %s", {{.*}}}
; CHECK: [[M7:!.*]] = distinct !{[[M7]]{{.*}}[[M8:!.*]]{{.*}}}
; CHECK: [[M8]] = distinct !{!"intel.optreport.rootnode", [[M9:!.*]]}
; CHECK: [[M9]] = distinct !{!"intel.optreport", [[M10:!.*]], [[M15:!.*]]}
; CHECK: [[M10]] = !{!"intel.optreport.next_sibling", [[M11:!.*]]}
; CHECK: [[M11]] = distinct !{!"intel.optreport.rootnode", [[M12:!.*]]}
; CHECK: [[M12]] = distinct !{!"intel.optreport", [[M13:!.*]]}
; CHECK: [[M13]] = !{!"intel.optreport.remarks", [[M14:!.*]]}
; CHECK: [[M14]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 10}
; CHECK: [[M15]] = !{!"intel.optreport.origin", [[M16:!.*]]}
; CHECK: [[M16]] = !{!"intel.optreport.remark", i32 25519, !"Remainder loop for vectorization"}

; MERGED-CFG: [[M1:!.*]] = distinct !{[[M1]]{{.*}}[[M2:!.*]]{{.*}}}
; MERGED-CFG: [[M2]] = distinct !{!"intel.optreport.rootnode", [[M3:!.*]]}
; MERGED-CFG: [[M3]] = distinct !{!"intel.optreport", [[M10:!.*]], [[M4:!.*]]}
; MERGED-CFG: [[M10]] = !{!"intel.optreport.next_sibling", [[M11:!.*]]}
; MERGED-CFG: [[M11]] = distinct !{!"intel.optreport.rootnode", [[M12:!.*]]}
; MERGED-CFG: [[M12]] = distinct !{!"intel.optreport", [[M13:!.*]]}
; MERGED-CFG: [[M13]] = !{!"intel.optreport.remarks", [[M14:!.*]]}
; MERGED-CFG: [[M14]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 10}
; MERGED-CFG: [[M4]] = !{!"intel.optreport.remarks", [[M5:!.*]], [[M6:!.*]]}
; MERGED-CFG: [[M5]] = !{!"intel.optreport.remark", i32 15300, !"LOOP WAS VECTORIZED"}
; MERGED-CFG: [[M6]] = !{!"intel.optreport.remark", i32 15305, !"vectorization support: vector length %s", {{.*}}}
; MERGED-CFG: [[M7:!.*]] = distinct !{[[M7]]{{.*}}[[M8:!.*]]{{.*}}}
; MERGED-CFG: [[M8]] = distinct !{!"intel.optreport.rootnode", [[M9:!.*]]}
; MERGED-CFG: [[M9]] = distinct !{!"intel.optreport", [[M15:!.*]]}
; MERGED-CFG: [[M15]] = !{!"intel.optreport.origin", [[M16:!.*]]}
; MERGED-CFG: [[M16]] = !{!"intel.optreport.remark", i32 25519, !"Remainder loop for vectorization"}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @foo(i32* noalias nocapture readonly %A, i32** noalias nocapture readonly %B, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp18 = icmp sgt i32 %N, 0
  br i1 %cmp18, label %for.body.lr.ph.split.us, label %for.cond.cleanup

for.body.lr.ph.split.us:                          ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body.us

for.body.us:                                      ; preds = %for.cond.cleanup8.us, %for.body.lr.ph.split.us
  %indvars.iv25 = phi i64 [ %indvars.iv.next26, %for.cond.cleanup8.us ], [ 0, %for.body.lr.ph.split.us ]
  %sum.019.us = phi i32 [ %add.us.lcssa, %for.cond.cleanup8.us ], [ 0, %for.body.lr.ph.split.us ]
  br label %for.body4.us

for.cond.cleanup8.us:                             ; preds = %for.body9.us
  %indvars.iv.next26 = add nuw nsw i64 %indvars.iv25, 1
  %exitcond28 = icmp eq i64 %indvars.iv.next26, %wide.trip.count
  br i1 %exitcond28, label %for.cond.cleanup.loopexit, label %for.body.us

for.body9.us:                                     ; preds = %for.body9.us, %for.cond1.for.cond.cleanup3_crit_edge.us
  %indvars.iv22 = phi i64 [ %indvars.iv.next23, %for.body9.us ], [ 0, %for.cond1.for.cond.cleanup3_crit_edge.us ]
  %arrayidx13.us = getelementptr inbounds i32, i32* %2, i64 %indvars.iv22
  %0 = trunc i64 %indvars.iv22 to i32
  store i32 %0, i32* %arrayidx13.us, align 4, !tbaa !2
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next23, 10
  br i1 %exitcond24, label %for.cond.cleanup8.us, label %for.body9.us

for.body4.us:                                     ; preds = %for.body4.us, %for.body.us
  %indvars.iv = phi i64 [ 0, %for.body.us ], [ %indvars.iv.next, %for.body4.us ]
  %sum.115.us = phi i32 [ %sum.019.us, %for.body.us ], [ %add.us, %for.body4.us ]
  %arrayidx.us = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx.us, align 4, !tbaa !2
  %add.us = add nsw i32 %1, %sum.115.us
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond1.for.cond.cleanup3_crit_edge.us, label %for.body4.us

for.cond1.for.cond.cleanup3_crit_edge.us:         ; preds = %for.body4.us
  %add.us.lcssa = phi i32 [ %add.us, %for.body4.us ]
  %indvars.iv.next.lcssa = phi i64 [ %indvars.iv.next, %for.body4.us ]
  %arrayidx11.us = getelementptr inbounds i32*, i32** %B, i64 %indvars.iv25
  %2 = load i32*, i32** %arrayidx11.us, align 8, !tbaa !6
  br label %for.body9.us

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup8.us
  %add.us.lcssa.lcssa = phi i32 [ %add.us.lcssa, %for.cond.cleanup8.us ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %add.us.lcssa.lcssa, %for.cond.cleanup.loopexit ]
  ret i32 %sum.0.lcssa
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"pointer@_ZTSPi", !4, i64 0}
