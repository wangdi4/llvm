; Check that proper optreport (structure and metadata) is emitted for Partial Unrolling.

;void foo(int *restrict A, int N) {
;
;  for (int j = 1; j < N; ++j) {
;    A[j]=A[j-1];
;  }
;  return;
;}

; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,hir-cg,simplifycfg,intel-ir-optreport-emitter" -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #25439: Loop unrolled with remainder by {{[0-9]+}}
; OPTREPORT-NEXT: LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT: LOOP BEGIN
; OPTREPORT-NEXT:     <Remainder loop>
; OPTREPORT-NEXT: LOOP END

; RUN: opt -passes="hir-ssa-deconstruction,hir-general-unroll,hir-cg" -intel-opt-report=low < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{[[M1]], {{!.*}}, [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.optreport.rootnode", [[M3:!.*]]}
; CHECK: [[M3]] = distinct !{!"intel.optreport", [[M4:!.*]]}
; CHECK: [[M4]] = !{!"intel.optreport.remarks", [[M5:!.*]]}
; CHECK: [[M5]] = !{!"intel.optreport.remark", i32 25439, !"Loop unrolled with remainder by %d", {{.*}}}
; CHECK: [[M6:!.*]] = distinct !{[[M6]], {{!.*}}, [[M7:!.*]]}
; CHECK: [[M7]] = distinct !{!"intel.optreport.rootnode", [[M8:!.*]]}
; CHECK: [[M8]] = distinct !{!"intel.optreport", [[M9:!.*]]}
; CHECK: [[M9]] = !{!"intel.optreport.origin", [[M10:!.*]]}
; CHECK: [[M10]] = !{!"intel.optreport.remark", i32 25491, !"Remainder loop"}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr noalias nocapture %A, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp6 = icmp sgt i32 %N, 1
  br i1 %cmp6, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %wide.trip.count = sext i32 %N to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.body
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.body, %for.body.lr.ph
  %indvars.iv = phi i64 [ 1, %for.body.lr.ph ], [ %indvars.iv.next, %for.body ]
  %0 = add nsw i64 %indvars.iv, -1
  %arrayidx = getelementptr inbounds i32, ptr %A, i64 %0
  %1 = load i32, ptr %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds i32, ptr %A, i64 %indvars.iv
  store i32 %1, ptr %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.cond.cleanup.loopexit, label %for.body
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
