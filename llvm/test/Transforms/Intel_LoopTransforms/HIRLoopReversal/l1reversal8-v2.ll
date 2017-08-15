; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that CAN be reversed because of no liveout variables.
; 
; l1reversal8-v2.ll:
; 1-level loop, sanity testcase, valid reversal case.
; There is NO liveout variable from the 2 level loop nests.
; Note:
; t is defined in loop i, and used in loop i. It is NOT considered a liveout w.r.t. loopi 
; even though we only have just 1 level of loop.
; 
; [REASONS]
; - Applicalbe: Yes. There are 2 cases where const coeff is negative;  
; - Profitable: YES, both memory accesses accumulate values on negative IV const coeff 
; - Legal:      YES
;               NO Liveout variable(s) make the loop LEGAL for reversal!
; 
;DO i
;  t = A[.i.]+1;
;  A[.i.] = t +1;
;  ..
;ENDDO i
;
; t is not live out of loop i
;
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;int foo(int *restrict A, int *restrict B) {
;  int t = 0;
;  int i = 0, j = 0;
;
;  //loop i
;  for (i = 0; i <= 4; i++) {
;    t = A[10 - 2 * i];        //t is defined in loop
;    B[50 - 2 * i] = A[t] + 1; // t is used in loop
;  }
;
;  //t is not live out of j loop
;  return A[1] + B[0] + 1;
;}
;
; [AFTER LOOP REVERSAL: NO REVERSAL WILL HAPPEN!]
; 
;{...}
; 
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal, DOESN'T REVERSE anything ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-after=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=AFTER 
;
;
; === -------------------------------------- ===
; *** Tests0: W/O HIR Loop Reversal Output ***
; === -------------------------------------- ===
; Expected output before Loop Reversal
; 
;          BEGIN REGION { }
;<19>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<5>          |   %2 = (%A)[-2 * i1 + 10];
;<8>          |   %3 = (%A)[%2];
;<12>         |   (%B)[-2 * i1 + 50] = %3 + 1;
;<19>         + END LOOP
;          END REGION
; 
; BEFORE:  BEGIN REGION { }
; BEFORE:      + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   %2 = (%A)[-2 * i1 + 10];
; BEFORE:     |   %3 = (%A)[%2];
; BEFORE:     |   (%B)[-2 * i1 + 50] = %3 + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
;     The loop will NOT be reversed!!
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal
; 
;          BEGIN REGION { modified }
;<19>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<5>          |   %2 = (%A)[2 * i1 + 2];
;<8>          |   %3 = (%A)[%2];
;<12>         |   (%B)[2 * i1 + 42] = %3 + 1;
;<19>         + END LOOP
;          END REGION
;
; AFTER:   BEGIN REGION { modified }
; AFTER:      + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:      |   %2 = (%A)[2 * i1 + 2];
; AFTER:      |   %3 = (%A)[%2];
; AFTER:      |   (%B)[2 * i1 + 42] = %3 + 1;
; AFTER:      + END LOOP
; AFTER:   END REGION
;
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32* noalias nocapture readonly %A, i32* noalias nocapture %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %0 = shl nsw i64 %indvars.iv, 1
  %1 = sub nuw nsw i64 10, %0
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %idxprom1 = sext i32 %2 to i64
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %idxprom1
  %3 = load i32, i32* %arrayidx2, align 4, !tbaa !1
  %add = add nsw i32 %3, 1
  %4 = sub nuw nsw i64 50, %0
  %arrayidx6 = getelementptr inbounds i32, i32* %B, i64 %4
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx7 = getelementptr inbounds i32, i32* %A, i64 1
  %5 = load i32, i32* %arrayidx7, align 4, !tbaa !1
  %6 = load i32, i32* %B, align 4, !tbaa !1
  %add9 = add i32 %5, 1
  %add10 = add i32 %add9, %6
  ret i32 %add10
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 9725) (llvm/branches/loopopt 9729)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
