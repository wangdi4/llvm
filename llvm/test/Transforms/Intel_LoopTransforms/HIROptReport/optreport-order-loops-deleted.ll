; Check the proper optreport order (structure and metadata) for deleted loops (Completely Unrolled).
; Expected structure: first_child <- first_child <- first_child
;                                        \ <- next_sibling

;void foo(int *restrict A, int *restrict B, int *restrict C) {
;
;  for (int i = 0; i < 10; ++i) {
;    A[i] = i;
;    for (int i = 0; i < 10; ++i) {
;      B[i] = i;
;        for (int i = 0; i < 2; ++i) {
;          B[i] = i;
;        }
;    }
;    for (int i = 0; i < 3; ++i) {
;      C[i] = i;
;    }
;  }
;  return;
;}

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-optreport-emitter -hir-cg -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
; If we unroll loop nest, we add remark only to the outer loop in the nest.

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #25436: Loop completely unrolled by 10

; OPTREPORT:          LOOP BEGIN
; OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 10

; OPTREPORT:              LOOP BEGIN
; OPTREPORT-NEXT:             remark #25436: Loop completely unrolled by 2
; OPTREPORT-NEXT:         LOOP END
; OPTREPORT-NEXT:     LOOP END

; OPTREPORT:          LOOP BEGIN
; OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 3
; OPTREPORT-NEXT:     LOOP END
; OPTREPORT-NEXT: LOOP END

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{!"intel.optreport.rootnode", [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.optreport", [[M3:!.*]]}
; CHECK: [[M3]] = !{!"intel.optreport.first_child", [[M4:!.*]]}
; CHECK: [[M4]] = distinct !{!"intel.optreport.rootnode", [[M5:!.*]]}
; CHECK: [[M5]] = distinct !{!"intel.optreport", [[M6:!.*]], [[M8:!.*]]}
; CHECK: [[M6]] = !{!"intel.optreport.remarks", [[M7:!.*]]}
; CHECK: [[M7]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 10}
; CHECK: [[M8]] = !{!"intel.optreport.first_child", [[M9:!.*]]}
; CHECK: [[M9]] = distinct !{!"intel.optreport.rootnode", [[M10:!.*]]}
; CHECK: [[M10]] = distinct !{!"intel.optreport", [[M6]], [[M11:!.*]], [[M16:!.*]]}
; CHECK: [[M11]] = !{!"intel.optreport.next_sibling", [[M12:!.*]]}
; CHECK: [[M12]] = distinct !{!"intel.optreport.rootnode", [[M13:!.*]]}
; CHECK: [[M13]] = distinct !{!"intel.optreport", [[M14:!.*]]}
; CHECK: [[M14]] = !{!"intel.optreport.remarks", [[M15:!.*]]}
; CHECK: [[M15]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 3}
; CHECK: [[M16]] = !{!"intel.optreport.first_child", [[M17:!.*]]}
; CHECK: [[M17]] = distinct !{!"intel.optreport.rootnode", [[M18:!.*]]}
; CHECK: [[M18]] = distinct !{!"intel.optreport", [[M19:!.*]]}
; CHECK: [[M19]] = !{!"intel.optreport.remarks", [[M20:!.*]]}
; CHECK: [[M20]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 2}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* noalias nocapture %B, i32* noalias nocapture %C) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.cond.cleanup21
  ret void

for.body:                                         ; preds = %for.cond.cleanup21, %entry
  %indvars.iv28 = phi i64 [ 0, %entry ], [ %indvars.iv.next29, %for.cond.cleanup21 ], !in.de.ssa !2
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv28
  %0 = trunc i64 %indvars.iv28 to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !3
  %indvars.iv22.in = bitcast i64 0 to i64, !in.de.ssa !7
  br label %for.body5

for.cond.cleanup4:                                ; preds = %for.cond.cleanup11
  %indvars.iv25.in = bitcast i64 0 to i64, !in.de.ssa !8
  br label %for.body22

for.body5:                                        ; preds = %for.cond.cleanup11, %for.body
  %indvars.iv22 = phi i64 [ 0, %for.body ], [ %indvars.iv.next23, %for.cond.cleanup11 ], !in.de.ssa !7
  %arrayidx7 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv22
  %1 = trunc i64 %indvars.iv22 to i32
  store i32 %1, i32* %arrayidx7, align 4, !tbaa !3
  %indvars.iv.in = bitcast i64 0 to i64, !in.de.ssa !9
  br label %for.body12

for.cond.cleanup11:                               ; preds = %for.body12
  %indvars.iv.next23 = add nuw nsw i64 %indvars.iv22, 1
  %exitcond24 = icmp eq i64 %indvars.iv.next23, 10
  %indvars.iv22.in31 = bitcast i64 %indvars.iv.next23 to i64, !in.de.ssa !7
  br i1 %exitcond24, label %for.cond.cleanup4, label %for.body5

for.body12:                                       ; preds = %for.body12, %for.body5
  %indvars.iv = phi i64 [ 0, %for.body5 ], [ %indvars.iv.next, %for.body12 ], !in.de.ssa !9
  %arrayidx14 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %arrayidx14, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 2
  %indvars.iv.in32 = bitcast i64 %indvars.iv.next to i64, !in.de.ssa !9
  br i1 %exitcond, label %for.cond.cleanup11, label %for.body12

for.cond.cleanup21:                               ; preds = %for.body22
  %indvars.iv.next29 = add nuw nsw i64 %indvars.iv28, 1
  %exitcond30 = icmp eq i64 %indvars.iv.next29, 10
  %indvars.iv28.in = bitcast i64 %indvars.iv.next29 to i64, !in.de.ssa !2
  br i1 %exitcond30, label %for.cond.cleanup, label %for.body

for.body22:                                       ; preds = %for.body22, %for.cond.cleanup4
  %indvars.iv25 = phi i64 [ 0, %for.cond.cleanup4 ], [ %indvars.iv.next26, %for.body22 ], !in.de.ssa !8
  %arrayidx24 = getelementptr inbounds i32, i32* %C, i64 %indvars.iv25
  %3 = trunc i64 %indvars.iv25 to i32
  store i32 %3, i32* %arrayidx24, align 4, !tbaa !3
  %indvars.iv.next26 = add nuw nsw i64 %indvars.iv25, 1
  %exitcond27 = icmp eq i64 %indvars.iv.next26, 3
  %indvars.iv25.in33 = bitcast i64 %indvars.iv.next26 to i64, !in.de.ssa !8
  br i1 %exitcond27, label %for.cond.cleanup21, label %for.body22
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"indvars.iv28.de.ssa"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!"indvars.iv22.de.ssa"}
!8 = !{!"indvars.iv25.de.ssa"}
!9 = !{!"indvars.iv.de.ssa"}
