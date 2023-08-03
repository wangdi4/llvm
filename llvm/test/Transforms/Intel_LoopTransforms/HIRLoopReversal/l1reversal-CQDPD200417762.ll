; Sanity Test(s) on HIR Loop Reversal: testcase reported by CQ DPD200413783
;
; [REASONS]
; - Applicable: YES
; - Profitable: YES
; - Legal:      NO
; - Decision:   NOT Reverse the loop
;
; Explain:
; %add1233826:22 (%t)[0][-1 * i1 + 42] --> (%t)[0][-1 * i1 + 44] ANTI (<) fails reversal's legal test.

; Issue with the CQ DPD200417762: same as CQDPD200415561
;
; *** Source Code ***
; from ATG Tests, reported by Youcef/John.
;
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reversal,print<hir>" -aa-pipeline="basic-aa" < %s 2>&1 | FileCheck %s
;
; CHECK: Function
;
; CHECK:  + DO i1 = 0, 41, 1   <DO_LOOP>
; CHECK:  |   (%t)[0][-1 * i1 + 44] = -1 * i1 + %nv.promoted + -1;
; CHECK:  |   %add12338 = %add12338  +  (%t)[0][-1 * i1 + 42];
; CHECK:  + END LOOP
;
;
; CHECK: Function
;
; CHECK:  + DO i1 = 0, 41, 1   <DO_LOOP>
; CHECK:  |   (%t)[0][-1 * i1 + 44] = -1 * i1 + %nv.promoted + -1;
; CHECK:  |   %add12338 = %add12338  +  (%t)[0][-1 * i1 + 42];
; CHECK:  + END LOOP
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;

define void @foo(i32 %s5.promoted, i32 %nv.promoted) {
entry:
  %t = alloca [100 x i32], align 16
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv377 = phi i64 [ 43, %entry ], [ %indvars.iv.next378, %for.body ]
  %add12338 = phi i32 [ %s5.promoted, %entry ], [ %add12, %for.body ]
  %dec337 = phi i32 [ %nv.promoted, %entry ], [ %dec, %for.body ]
  %dec = add i32 %dec337, -1
  %0 = add nuw nsw i64 %indvars.iv377, 1
  %arrayidx9 = getelementptr inbounds [100 x i32], ptr %t, i64 0, i64 %0
  store i32 %dec, ptr %arrayidx9, align 4, !tbaa !1
  %indvars.iv.next378 = add nsw i64 %indvars.iv377, -1
  %arrayidx11 = getelementptr inbounds [100 x i32], ptr %t, i64 0, i64 %indvars.iv.next378
  %1 = load i32, ptr %arrayidx11, align 4, !tbaa !1
  %add12 = add i32 %add12338, %1
  %cmp = icmp ugt i64 %indvars.iv.next378, 1
  br i1 %cmp, label %for.body, label %exit

exit:
  %add12.lcssa = phi i32 [ %add12, %for.body ]
  ret void
}

!0 = !{!"clang version 4.0.0 (trunk 20746) (llvm/branches/loopopt 20781)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}

