; Check the proper optreport order (structure and metadata) for deleted loops (Completely Unrolled) and vectorized loop with remainder.
; Expected structure: first_child <- next_sibling = Vectorized + Remainder
;                                                                    \ <- next_sibling

;void foo(int *restrict A, int *restrict B, int *restrict C, int *restrict D, int *G, int N) {
;
;  for (int i = 0; i < 3; ++i) {
;    C[i] = i;
;  }
;  *G += N;
;  for (int i = 0; i < N; ++i) {
;    D[i] = i;
;  }
;  for (int i = 0; i < 3; ++i) {
;    D[i] = i;
;  }
;}

; When we completely unroll third loop in HIR, we attach it's opt report to its region as a first child/next sibling.
; Instead, we have to find a lexically previous sibling loop, but because this sibling loop is located in a different
; region, we fail to do that.
; XFAIL: *

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-vec-dir-insert -VPODriverHIR -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strich-whitespace

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #XXXXX: Loop completely unrolled
; OPTREPORT-NEXT: LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #XXXXX: Loop has been vectorized with vector {{.*}} factor
; OPTREPORT-NEXT: LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT: LOOP BEGIN
; OPTREPORT-NEXT:     <Remainder loop for vectorization>
; OPTREPORT-NEXT: LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #XXXXX: Loop completely unrolled
; OPTREPORT-NEXT: LOOP END

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-vec-dir-insert -VPODriverHIR -hir-cg -intel-opt-report=low < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{!"intel.optreport.rootnode", [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.optreport", [[M3:!.*]]}
; CHECK: [[M3]] = !{!"intel.optreport.first_child", [[M4:!.*]]}
; CHECK: [[M4]] = distinct !{!"intel.optreport.rootnode", [[M5:!.*]]}
; CHECK: [[M5]] = distinct !{!"intel.optreport", [[M6:!.*]], [[M8:!.*]]}
; CHECK: [[M6]] = !{!"intel.optreport.remarks", [[M7:!.*]]}
; CHECK: [[M7]] = !{!"intel.optreport.remark", !"Loop completely unrolled"}
; CHECK: [[M8]] = !{!"intel.optreport.next_sibling", [[M9:!.*]]}
; CHECK: [[M9]] = distinct !{!"intel.optreport.rootnode", [[M10:!.*]]}
; CHECK: [[M10]] = distinct !{!"intel.optreport", [[M11:!]]}
; CHECK: [[M11]] = !{!"intel.optreport.remarks", [[M12:!.*]]}
; CHECK: [[M12]] = !{!"intel.optreport.remark", !"Loop has been vectorized with vector %d factor", {{.*}}}
; CHECK: [[M13:!.*]] = distinct !{[[M13]]{{.*}}[[M14:!.*]]{{.*}}}
; CHECK: [[M14]] = distinct !{!"intel.optreport.rootnode", [[M15:!.*]]}
; CHECK: [[M15]] = distinct !{!"intel.optreport", [[M16:!.*]], [[M19:!.*]]}
; CHECK: [[M16]] = !{!"intel.optreport.next_sibling", [[M17:!.*]]}
; CHECK: [[M17]] = distinct !{!"intel.optreport.rootnode", [[M18:!.*]]}
; CHECK: [[M18]] = distinct !{!"intel.optreport", [[M6]]}
; CHECK: [[M19]] = !{!"intel.optreport.origin", [[M20:!.*]]}
; CHECK: [[M20]] = !{!"intel.optreport.remark", !"Remainder loop for vectorization"}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture readnone %A, i32* noalias nocapture readnone %B, i32* noalias nocapture %C, i32* noalias nocapture %D, i32* nocapture %G, i32 %N) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %0 = load i32, i32* %G, align 4, !tbaa !2
  %add = add nsw i32 %0, %N
  store i32 %add, i32* %G, align 4, !tbaa !2
  %cmp316 = icmp sgt i32 %N, 0
  br i1 %cmp316, label %for.body5.lr.ph, label %for.cond.cleanup4

for.body5.lr.ph:                                  ; preds = %for.cond.cleanup
  %wide.trip.count = sext i32 %N to i64
  br label %for.body5

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv22 = phi i64 [ 0, %entry ], [ %indvars.iv.next23, %for.body ], !in.de.ssa !6
  %arrayidx = getelementptr inbounds i32, i32* %C, i64 %indvars.iv22
  %1 = trunc i64 %indvars.iv22 to i32
  store i32 %1, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next23, 3
  %indvars.iv22.in = bitcast i64 %indvars.iv.next23 to i64, !in.de.ssa !6
  br i1 %exitcond24, label %for.cond.cleanup, label %for.body

for.cond.cleanup4.loopexit:                       ; preds = %for.body5
  br label %for.cond.cleanup4

for.cond.cleanup4:                                ; preds = %for.cond.cleanup4.loopexit, %for.cond.cleanup
  br label %for.body15

for.body5:                                        ; preds = %for.body5, %for.body5.lr.ph
  %indvars.iv19 = phi i64 [ 0, %for.body5.lr.ph ], [ %indvars.iv.next20, %for.body5 ], !in.de.ssa !7
  %arrayidx7 = getelementptr inbounds i32, i32* %D, i64 %indvars.iv19
  %2 = trunc i64 %indvars.iv19 to i32
  store i32 %2, i32* %arrayidx7, align 4, !tbaa !2
  %indvars.iv.next20 = add nuw nsw i64 %indvars.iv19, 1
  %exitcond21 = icmp eq i64 %indvars.iv.next20, %wide.trip.count
  %indvars.iv19.in = bitcast i64 %indvars.iv.next20 to i64, !in.de.ssa !7
  br i1 %exitcond21, label %for.cond.cleanup4.loopexit, label %for.body5

for.cond.cleanup14:                               ; preds = %for.body15
  ret void

for.body15:                                       ; preds = %for.body15, %for.cond.cleanup4
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup4 ], [ %indvars.iv.next, %for.body15 ], !in.de.ssa !8
  %arrayidx17 = getelementptr inbounds i32, i32* %D, i64 %indvars.iv
  %3 = trunc i64 %indvars.iv to i32
  store i32 %3, i32* %arrayidx17, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 3
  %indvars.iv.in = bitcast i64 %indvars.iv.next to i64, !in.de.ssa !8
  br i1 %exitcond, label %for.cond.cleanup14, label %for.body15
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!"indvars.iv22.de.ssa"}
!7 = !{!"indvars.iv19.de.ssa"}
!8 = !{!"indvars.iv.de.ssa"}
