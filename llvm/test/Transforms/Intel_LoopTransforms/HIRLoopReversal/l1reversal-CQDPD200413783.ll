; Sanity Test(s) on HIR Loop Reversal: testcase reported by CQ DPD200413783
; 
; [REASONS]
; - Applicable: YES
; - Profitable: YES
; - Legal:      YES
; - Decision:   Reverse the loop!
;
; Issue with the CQ DPD200413783:
; - there is a non-memory iv op that is not collected from the original reverser.
; - fixed to collect this iv;
; 
; *** Source Code ***
;int a1_lpa[192];
;unsigned a1_diwssz[192];
;int main_v_xk, main_v_me;
;int main() {
;  main_v_xk = 0;
;  for (; main_v_xk <= 35; main_v_xk++)
;  {
;    a1_diwssz[3 * main_v_xk] += main_v_xk;
;    a1_lpa[129 - 2 * main_v_xk] = 0;
;    a1_diwssz[2 * main_v_xk + main_v_xk] &= main_v_me + 1;  /*      *//*      */
;  }
;  return a1_diwssz[0];
;}
;
;
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -print-after=hir-loop-reversal < %s 2>&1 | FileCheck %s
;
; CHECK: IR Dump Before HIR Loop Reversal
;
; CHECK:  BEGIN REGION { }
; CHECK:        + DO i1 = 0, 35, 1   <DO_LOOP>
; CHECK:        |   %2 = (@a1_diwssz)[0][3 * i1];
; CHECK:        |   (@a1_lpa)[0][-2 * i1 + 129] = 0;
; CHECK:        |   %and = i1 + %2  &&  %0 + 1;
; CHECK:        |   (@a1_diwssz)[0][3 * i1] = %and;
; CHECK:        + END LOOP
; CHECK:  END REGION
;  
;
; CHECK: IR Dump After HIR Loop Reversal
;
; CHECK:  BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 35, 1   <DO_LOOP>
; CHECK:        |   %2 = (@a1_diwssz)[0][-3 * i1 + 105];
; CHECK:        |   (@a1_lpa)[0][2 * i1 + 59] = 0;
; CHECK:        |   %and = -1 * i1 + %2 + 35  &&  %0 + 1;
; CHECK:        |   (@a1_diwssz)[0][-3 * i1 + 105] = %and;
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "bug-DPD200413783.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@main_v_xk = common local_unnamed_addr global i32 0, align 4
@a1_diwssz = common local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@a1_lpa = common local_unnamed_addr global [192 x i32] zeroinitializer, align 16
@main_v_me = common local_unnamed_addr global i32 0, align 4

; Function Attrs: norecurse nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  store i32 0, i32* @main_v_xk, align 4, !tbaa !1
  %0 = load i32, i32* @main_v_me, align 4, !tbaa !1
  %add4 = add nsw i32 %0, 1
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %1 = mul nuw nsw i64 %indvars.iv, 3
  %arrayidx = getelementptr inbounds [192 x i32], [192 x i32]* @a1_diwssz, i64 0, i64 %1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %3 = trunc i64 %indvars.iv to i32
  %add = add i32 %2, %3
  %4 = shl nsw i64 %indvars.iv, 1
  %5 = sub nuw nsw i64 129, %4
  %arrayidx3 = getelementptr inbounds [192 x i32], [192 x i32]* @a1_lpa, i64 0, i64 %5
  store i32 0, i32* %arrayidx3, align 4, !tbaa !1
  %and = and i32 %add, %add4
  store i32 %and, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 36
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  store i32 36, i32* @main_v_xk, align 4, !tbaa !1
  %6 = load i32, i32* getelementptr inbounds ([192 x i32], [192 x i32]* @a1_diwssz, i64 0, i64 0), align 16, !tbaa !1
  ret i32 %6
}

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15884) (llvm/branches/loopopt 17927)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
