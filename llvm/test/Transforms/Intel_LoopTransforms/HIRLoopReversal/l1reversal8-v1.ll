; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that CAN be reversed because of no liveout variables.
; 
; l1reversal8-v1.ll:
; 2-level loop2, sanity testcase, valid reversal case.
; There is NO liveout variable from the 2 level loop nests.
; Note:
; t is defined in loop j, and used in loop i. It is NOT considered a liveout w.r.t. loop j (outer loop)
; 
; [REASONS]
; - Applicalbe: Yes. There are 2 cases where const coeff is negative;  
; - Profitable: YES, both memory accesses accumulate values on negative IV const coeff 
; - Legal:      YES
;               NO Liveout variable(s) make the loop LEGAL for reversal!
; 
; A valid liveout case:
; 
; DO j 
;  t = A[.j.]+1;
;
;  DO i
;  A[.i.] = t +1;
;  ..
;  ENDDO i
;
; ENDDO j
;
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;int foo(int *restrict A, int *restrict B) {
;  int t = 0;
;  int i = 0, j = 0;
;
;  //loop j
;  for (j = 0; j <= 4; ++j) {
;    t = A[10 - j]; //t is defined in j's level
;    //loop i
;    for (i = 0; i <= 4; i++) {
;      B[50 - 2 * i] = t + 1; // t is used in i's level
;    }
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
; Note: %add1 is actually the temp t
; 
;          BEGIN REGION { }
;<26>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          |   %1 = (%A)[-1 * i1 + 10];
;<27>         |   + DO i2 = 0, 4, 1   <DO_LOOP>
;<12>         |   |   (%B)[-2 * i2 + 50] = %1 + 1;
;<27>         |   + END LOOP
;<26>         + END LOOP
;          END REGION
; 
;BEFORE:   BEGIN REGION { }
;BEFORE:      + DO i1 = 0, 4, 1   <DO_LOOP>
;BEFORE:      |   %1 = (%A)[-1 * i1 + 10];
;BEFORE:      |   + DO i2 = 0, 4, 1   <DO_LOOP>
;BEFORE:      |   |   (%B)[-2 * i2 + 50] = %1 + 1;
;BEFORE:      |   + END LOOP
;BEFORE:      + END LOOP
;BEFORE:   END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
;     The loop will NOT be reversed!!
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal
; Note: %add1 is actually the temp t
; 
;          BEGIN REGION { modified }
;<26>         + DO i1 = 0, 4, 1   <DO_LOOP>
;<4>          |   %1 = (%A)[-1 * i1 + 10];
;<27>         |   + DO i2 = 0, 4, 1   <DO_LOOP>
;<12>         |   |   (%B)[2 * i2 + 42] = %1 + 1;
;<27>         |   + END LOOP
;<26>         + END LOOP
;          END REGION
;
; AFTER:   BEGIN REGION { modified }
; AFTER:      + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:      |   %1 = (%A)[-1 * i1 + 10];
; AFTER:      |   + DO i2 = 0, 4, 1   <DO_LOOP>
; AFTER:      |   |   (%B)[2 * i2 + 42] = %1 + 1;
; AFTER:      |   + END LOOP
; AFTER:      + END LOOP
; AFTER:   END REGION
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

for.body:                                         ; preds = %for.inc7, %entry
  %indvars.iv27 = phi i64 [ 0, %entry ], [ %indvars.iv.next28, %for.inc7 ]
  %0 = sub nuw nsw i64 10, %indvars.iv27
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %1, 1
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.body
  %indvars.iv = phi i64 [ 0, %for.body ], [ %indvars.iv.next, %for.body3 ]
  %2 = shl nsw i64 %indvars.iv, 1
  %3 = sub nuw nsw i64 50, %2
  %arrayidx6 = getelementptr inbounds i32, i32* %B, i64 %3
  store i32 %add, i32* %arrayidx6, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %indvars.iv.next28 = add nuw nsw i64 %indvars.iv27, 1
  %exitcond30 = icmp eq i64 %indvars.iv.next28, 5
  br i1 %exitcond30, label %for.end9, label %for.body

for.end9:                                         ; preds = %for.inc7
  %arrayidx10 = getelementptr inbounds i32, i32* %A, i64 1
  %4 = load i32, i32* %arrayidx10, align 4, !tbaa !1
  %5 = load i32, i32* %B, align 4, !tbaa !1
  %add12 = add i32 %4, 1
  %add13 = add i32 %add12, %5
  ret i32 %add13
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
