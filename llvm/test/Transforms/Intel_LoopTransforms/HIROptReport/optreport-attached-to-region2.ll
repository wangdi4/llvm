; Check the proper optreport order (structure and metadata) for deleted loops (Completely Unrolled) is attached to a region.
; Expected structure: first_child <- next_sibling

;void foo(int *restrict A, int *restrict B) {
;
;      for (int i = 0; i < 10; ++i) {
;        A[i] = i;
;      }
;      for (int i = 0; i < 10; ++i) {
;        B[i] = i;
;      }
;  return;
;}

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-loop-optreport=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN
; OPTREPORT:     remark #25532: Loop completely unrolled
; OPTREPORT: LOOP END{{[[:space:]]}}
; OPTREPORT: LOOP BEGIN
; OPTREPORT:     remark #25532: Loop completely unrolled
; OPTREPORT: LOOP END

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-loop-optreport=low < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{!"llvm.loop.optreport", [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.loop.optreport", [[M3:!.*]]}
; CHECK: [[M3]] = !{!"intel.optreport.first_child", [[M4:!.*]]}
; CHECK: [[M4]] = distinct !{!"llvm.loop.optreport", [[M5:!.*]]}
; CHECK: [[M5]] = distinct !{!"intel.loop.optreport", [[M6:!.*]], [[M8:!.*]]}
; CHECK: [[M6]] = !{!"intel.optreport.remarks", [[M7:!.*]]}
; CHECK: [[M7]] = !{!"intel.optreport.remark", i32 25532, !"Loop completely unrolled"}
; CHECK: [[M8]] = !{!"intel.optreport.next_sibling", [[M9:!.*]]}
; CHECK: [[M9]] = distinct !{!"llvm.loop.optreport", [[M10:!.*]]}
; CHECK: [[M10]] = distinct !{!"intel.loop.optreport", [[M6]]}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* noalias nocapture %B) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  br label %for.body5

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv11 = phi i64 [ 0, %entry ], [ %indvars.iv.next12, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv11
  %0 = trunc i64 %indvars.iv11 to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next12 = add nuw nsw i64 %indvars.iv11, 1
  %exitcond13 = icmp eq i64 %indvars.iv.next12, 10
  br i1 %exitcond13, label %for.cond.cleanup, label %for.body

for.cond.cleanup4:                                ; preds = %for.body5
  ret void

for.body5:                                        ; preds = %for.body5, %for.cond.cleanup
  %indvars.iv = phi i64 [ 0, %for.cond.cleanup ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx7 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx7, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup4, label %for.body5
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
