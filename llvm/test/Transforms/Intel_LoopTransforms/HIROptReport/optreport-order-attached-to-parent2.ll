; Check that optreport order (structure and metadata) for deleted loops (Completely Unrolled) is attached to a parent loop (first_child field).
; Expected structure: first_child <- next_sibling <- next_sibling

;void foo(int *restrict A, int *restrict B, int *restrict C, int N) {
;
;  for (int j = 0; j < N; ++j) {
;      if ( j < 10 ) {
;        for (int i = 0; i < 10; ++i) {
;          A[i] = i;
;        }
;      } else {
;        for (int i = 0; i < 10; ++i) {
;          B[i] = i;
;        }
;      }
;      for (int i = 0; i < 10; ++i) {
;          C[i] = i;
;      }
;    }
;  return;
;}

; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-opt-report=low -simplifycfg -intel-ir-optreport-emitter 2>&1 < %s -S | FileCheck %s -check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:         remark #25436: Loop completely unrolled by 10
; OPTREPORT-NEXT:     LOOP END{{[[:space:]]}}
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
; CHECK: [[M6]] = distinct !{!"intel.optreport", [[M7:!.*]], [[M9:!.*]]}
; CHECK: [[M7]] = !{!"intel.optreport.remarks", [[M8:!.*]]}
; CHECK: [[M8]] = !{!"intel.optreport.remark", i32 25436, !"Loop completely unrolled by %d", i32 10}
; CHECK: [[M9]] = !{!"intel.optreport.next_sibling", [[M10:!.*]]}
; CHECK: [[M10]] = distinct !{!"intel.optreport.rootnode", [[M11:!.*]]}
; CHECK: [[M11]] = distinct !{!"intel.optreport", [[M7]], [[M12:!.*]]}
; CHECK: [[M12]] = !{!"intel.optreport.next_sibling", [[M13:!.*]]}
; CHECK: [[M13]] = distinct !{!"intel.optreport.rootnode", [[M14:!.*]]}
; CHECK: [[M14]] = distinct !{!"intel.optreport", [[M7]]}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* noalias nocapture %B, i32* noalias nocapture %C, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp20 = icmp sgt i32 %N, 0
  br i1 %cmp20, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup19
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

for.body:                                         ; preds = %for.cond.cleanup19, %for.body.lr.ph
  %j.021 = phi i32 [ 0, %for.body.lr.ph ], [ %inc27, %for.cond.cleanup19 ]
  %cmp1 = icmp ult i32 %j.021, 10
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %if.then
  %indvars.iv23 = phi i64 [ 0, %if.then ], [ %indvars.iv.next24, %for.body5 ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv23
  %0 = trunc i64 %indvars.iv23 to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next24 = add nuw nsw i64 %indvars.iv23, 1
  %exitcond25 = icmp eq i64 %indvars.iv.next24, 10
  br i1 %exitcond25, label %if.end.loopexit, label %for.body5

if.else:                                          ; preds = %for.body
  br label %for.body10

for.body10:                                       ; preds = %for.body10, %if.else
  %indvars.iv = phi i64 [ 0, %if.else ], [ %indvars.iv.next, %for.body10 ]
  %arrayidx12 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %1 = trunc i64 %indvars.iv to i32
  store i32 %1, i32* %arrayidx12, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %if.end.loopexit31, label %for.body10

if.end.loopexit:                                  ; preds = %for.body5
  br label %if.end

if.end.loopexit31:                                ; preds = %for.body10
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit31, %if.end.loopexit
  br label %for.body20

for.cond.cleanup19:                               ; preds = %for.body20
  %inc27 = add nuw nsw i32 %j.021, 1
  %exitcond29 = icmp eq i32 %inc27, %N
  br i1 %exitcond29, label %for.cond.cleanup.loopexit, label %for.body

for.body20:                                       ; preds = %for.body20, %if.end
  %indvars.iv26 = phi i64 [ 0, %if.end ], [ %indvars.iv.next27, %for.body20 ]
  %arrayidx22 = getelementptr inbounds i32, i32* %C, i64 %indvars.iv26
  %2 = trunc i64 %indvars.iv26 to i32
  store i32 %2, i32* %arrayidx22, align 4, !tbaa !2
  %indvars.iv.next27 = add nuw nsw i64 %indvars.iv26, 1
  %exitcond28 = icmp eq i64 %indvars.iv.next27, 10
  br i1 %exitcond28, label %for.cond.cleanup19, label %for.body20
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
