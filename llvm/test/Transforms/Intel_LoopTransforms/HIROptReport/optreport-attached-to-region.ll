; Check the optreport (structure and metadata) for a deleted loop (Completely Unrolled) is attached to a region.

;void foo(int *restrict A, int sum) {
;
;  for (int i = 0; i < 10; ++i) {
;    A[i] += i*i ;
;  }
;  return;
;}

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT: LOOP END

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{!"intel.optreport.rootnode", [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.optreport", [[M3:!.*]]}
; CHECK: [[M3]] = !{!"intel.optreport.first_child", [[M4:!.*]]}
; CHECK: [[M4]] = distinct !{!"intel.optreport.rootnode", [[M5:!.*]]}
; CHECK: [[M5]] = distinct !{!"intel.optreport", [[M6:!.*]]}
; CHECK: [[M6]] = !{!"intel.optreport.remarks", [[M7:!.*]]}
; CHECK: [[M7]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 10}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32 %sum) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = mul nsw i64 %indvars.iv, %indvars.iv
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %2 = trunc i64 %0 to i32
  %add = add nsw i32 %1, %2
  store i32 %add, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
