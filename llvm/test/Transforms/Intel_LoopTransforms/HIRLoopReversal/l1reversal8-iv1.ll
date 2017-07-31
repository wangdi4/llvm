; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that CAN'T be reversed due to valid liveout
; 
; l1reversal8-iv1.ll:
; 1-level loop, sanity testcase, invalid reversal case, due to available liveout variable from loop.
; 
; [REASONS]
; - Applicalbe: Yes. There are 2 cases where const coeff is negative;  
; - Profitable: YES, both memory accesses accumulate values on negative IV const coeff 
; - Legal:      NO
;               Liveout variable(s) make the loop ILLEGAL for reversal!
; 
; A valid liveout case: 
; DO
; ..
;  t = A[i]+1;
; ..
; ENDDO
; ...{out-of-loop code}...
; . = t... //use of t1
;
; 
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;int foo(int *restrict A, int *restrict B) {
;  int t = 0;
;  int i = 0;
;
;  //loop i
;  for (i = 0; i <= 4; i++) {
;    t = A[100 - i] + 1;
;    B[50 - 2 * i] = t + 1;
;  }
;
;  //t is liveout of loop i
;  return A[1] + B[1] + 1 + t;
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
; Note: %add1 is actually the temp t
; 
;          BEGIN REGION { }
;<16>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          |   %1 = (%A)[-1 * i1 + 100];
;<5>          |   %add1 = %1  +  2;
;<9>          |   (%B)[-2 * i1 + 50] = %1 + 2;
;<16>         + END LOOP
;          END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   %1 = (%A)[-1 * i1 + 100];
; BEFORE:     |   %add1 = %1  +  2;
; BEFORE:     |   (%B)[-2 * i1 + 50] = %1 + 2;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
;     The loop will NOT be reversed!!
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal
; Note: %add1 is actually the temp t
; 
;          BEGIN REGION { }
;<16>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          |   %1 = (%A)[-1 * i1 + 100];
;<5>          |   %add1 = %1  +  2;
;<9>          |   (%B)[-2 * i1 + 50] = %1 + 2;
;<16>         + END LOOP
;          END REGION
;
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:     |   %1 = (%A)[-1 * i1 + 100];
; AFTER:     |   %add1 = %1  +  2;
; AFTER:     |   (%B)[-2 * i1 + 50] = %1 + 2;
; AFTER:     + END LOOP
; AFTER:  END REGION
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
  %0 = sub nuw nsw i64 100, %indvars.iv
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add1 = add i32 %1, 2
  %2 = shl nsw i64 %indvars.iv, 1
  %3 = sub nuw nsw i64 50, %2
  %arrayidx4 = getelementptr inbounds i32, i32* %B, i64 %3
  store i32 %add1, i32* %arrayidx4, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 1
  %4 = load i32, i32* %arrayidx5, align 4, !tbaa !1
  %arrayidx6 = getelementptr inbounds i32, i32* %B, i64 1
  %5 = load i32, i32* %arrayidx6, align 4, !tbaa !1
  %add8 = add i32 %add1, %4
  %add9 = add i32 %add8, %5
  ret i32 %add9
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
