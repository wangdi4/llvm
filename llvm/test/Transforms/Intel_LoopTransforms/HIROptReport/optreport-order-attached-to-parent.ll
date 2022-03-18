; Check that proper optreport order (structure and metadata) for deleted loops (Completely Unrolled) is attached to parent loop.
; Expected structure: first_child <- next_sibling

;void foo(int *restrict A, int *restrict B, int N) {
;
;  int number = 100;
;  for (int j = 0; j < N; ++j)
;    {
;      if(number > 80)
;      {
;        for (int i = 0; i < 10; ++i) {
;          A[i] = i;
;        }
;      }
;      for (int i = 0; i < 10; ++i) {
;        B[i] = i;
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
; CHECK: [[M11]] = distinct !{!"intel.optreport", [[M7]]}

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i32* noalias nocapture %A, i32* noalias nocapture %B, i32 %N) local_unnamed_addr #0 {
entry:
  %cmp14 = icmp sgt i32 %N, 0
  br i1 %cmp14, label %if.then.lr.ph, label %for.cond.cleanup

if.then.lr.ph:                                    ; preds = %entry
  br label %if.then

for.cond.cleanup.loopexit:                        ; preds = %for.cond.cleanup9
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %entry
  ret void

if.then:                                          ; preds = %for.cond.cleanup9, %if.then.lr.ph
  %j.015 = phi i32 [ 0, %if.then.lr.ph ], [ %inc17, %for.cond.cleanup9 ]
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %if.then
  %indvars.iv = phi i64 [ 0, %if.then ], [ %indvars.iv.next, %for.body5 ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %0 = trunc i64 %indvars.iv to i32
  store i32 %0, i32* %arrayidx, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond, label %if.end, label %for.body5

if.end:                                           ; preds = %for.body5
  br label %for.body10

for.cond.cleanup9:                                ; preds = %for.body10
  %inc17 = add nuw nsw i32 %j.015, 1
  %exitcond19 = icmp eq i32 %inc17, %N
  br i1 %exitcond19, label %for.cond.cleanup.loopexit, label %if.then

for.body10:                                       ; preds = %for.body10, %if.end
  %indvars.iv16 = phi i64 [ 0, %if.end ], [ %indvars.iv.next17, %for.body10 ]
  %arrayidx12 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv16
  %1 = trunc i64 %indvars.iv16 to i32
  store i32 %1, i32* %arrayidx12, align 4, !tbaa !2
  %indvars.iv.next17 = add nuw nsw i64 %indvars.iv16, 1
  %exitcond18 = icmp eq i64 %indvars.iv.next17, 10
  br i1 %exitcond18, label %for.cond.cleanup9, label %for.body10
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
