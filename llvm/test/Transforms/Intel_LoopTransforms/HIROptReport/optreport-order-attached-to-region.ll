; Check that proper optreport order (structure and metadata) for deleted loops (Completely Unrolled) is attached to a region.
; Expected structure: first_child <- next_sibling <- next_sibling

;void foo(int *restrict A, int *restrict B, int *restrict C, int N) {
;
;  if ( N < 10 ) {
;     for (int i = 0; i < 10; ++i) {
;        A[i] = i;
;     }
;  }
;  else {
;    for (int i = 0; i < 10; ++i) {
;      B[i] = i;
;    }
;  }
;  for (int i = 0; i < 10; ++i) {
;     C[i] = i;
;  }
;  return;
;}

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace
; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-optreport-emitter -hir-cg -intel-opt-report=low 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT: LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT: LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT: LOOP BEGIN
; OPTREPORT-NEXT:     remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT: LOOP END

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low < %s -S | FileCheck %s

; CHECK: [[M1:!.*]] = distinct !{!"intel.optreport.rootnode", [[M2:!.*]]}
; CHECK: [[M2]] = distinct !{!"intel.optreport", [[M3:!.*]]}
; CHECK: [[M3]] = !{!"intel.optreport.first_child", [[M4:!.*]]}
; CHECK: [[M4]] = distinct !{!"intel.optreport.rootnode", [[M5:!.*]]}
; CHECK: [[M5]] = distinct !{!"intel.optreport", [[M6:!.*]], [[M8:!.*]]}
; CHECK: [[M6]] = !{!"intel.optreport.remarks", [[M7:!.*]]}
; CHECK: [[M7]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 10}
; CHECK: [[M8]] = !{!"intel.optreport.next_sibling", [[M9:!.*]]}
; CHECK: [[M9]] = distinct !{!"intel.optreport.rootnode", [[M11:!.*]]}
; CHECK: [[M10:!.*]] = distinct !{!"intel.optreport", [[M6]], [[M11:!.*]]}
; CHECK: [[M11]] = !{!"intel.optreport.next_sibling", [[M12:!.*]]}
; CHECK: [[M12]] = distinct !{!"intel.optreport.rootnode", [[M13:!.*]]}
; CHECK: [[M13]] = distinct !{!"intel.optreport", [[M6]]}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* noalias nocapture %B, i32* noalias nocapture %C, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp = icmp slt i32 %N, 10
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %if.then
  %indvars.iv18 = phi i64 [ 0, %if.then ], [ %indvars.iv.next19, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv18
  %0 = trunc i64 %indvars.iv18 to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next19 = add nuw nsw i64 %indvars.iv18, 1
  %exitcond20 = icmp eq i64 %indvars.iv.next19, 10
  br i1 %exitcond20, label %if.end.loopexit, label %for.body

if.else:                                          ; preds = %entry
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %if.else
  %indvars.iv21 = phi i64 [ 0, %if.else ], [ %indvars.iv.next22, %for.body6 ]
  %arrayidx8 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv21
  %1 = trunc i64 %indvars.iv21 to i32
  store i32 %1, i32* %arrayidx8, align 4, !tbaa !2
  %indvars.iv.next22 = add nuw nsw i64 %indvars.iv21, 1
  %exitcond23 = icmp eq i64 %indvars.iv.next22, 10
  br i1 %exitcond23, label %if.end.loopexit25, label %for.body6

if.end.loopexit:                                  ; preds = %for.body
  br label %if.end

if.end.loopexit25:                                ; preds = %for.body6
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit25, %if.end.loopexit
  br label %for.body16

for.cond.cleanup15:                               ; preds = %for.body16
  ret void

for.body16:                                       ; preds = %for.body16, %if.end
  %indvars.iv = phi i64 [ 0, %if.end ], [ %indvars.iv.next, %for.body16 ]
  %arrayidx18 = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  %2 = trunc i64 %indvars.iv to i32
  store i32 %2, i32* %arrayidx18, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %for.cond.cleanup15, label %for.body16
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
