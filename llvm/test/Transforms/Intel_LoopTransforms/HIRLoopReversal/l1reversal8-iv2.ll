; Sanity Test(s) on HIR Loop Reversal: simple l1 loop that CAN'T be reversed due to valid liveout
; 
; l1reversal8-iv2.ll:
; 1-level loop, sanity testcase, invalid reversal case, due to 1 available liveout variable.
; 
; [REASONS]
; - Applicalbe: Yes. There are 2 cases where const coeff is negative;  
; - Profitable: YES, both memory accesses accumulate values on negative IV const coeff 
; - Legal:      NO
;               Liveout variable(s) make the loop ILLEGAL for reversal!
; 
; A valid liveout case:
; DO j 
; DO i
; ..
;  t = A[i]+1;
; ..
; ENDDO i
; ...{out-of-loop i, but in loop j code}...
; . = t... //use of t1
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
;    //loop i
;    for (i = 0; i <= 4; i++) {
;      t = t + A[100 - i] + 1;
;      B[50 - 2 * i] = t + 1;
;    }
;    A[10 - j] = t; //t is live out of i, live in j, but not out of region
;  }
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
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   + DO i2 = 0, 4, 1   <DO_LOOP>
; BEFORE:     |   |   %1 = (%A)[-1 * i2 + 100];
; BEFORE:     |   |   %t.033 = %1  +  %t.033;
; BEFORE:     |   |   %t.033.out1 = %t.033;
; BEFORE:     |   |   %t.033 = %t.033  +  1;
; BEFORE:     |   |   %t.033.out = %t.033;
; BEFORE:     |   |   (%B)[-2 * i2 + 50] = %t.033.out1 + 2;
; BEFORE:     |   + END LOOP
; BEFORE:     |   (%A)[-1 * i1 + 10] = %t.033.out;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
;
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
;     The loop will NOT be reversed!!
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal
; Note: %add1 is actually the temp t
;
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:     |   + DO i2 = 0, 4, 1   <DO_LOOP>
; AFTER:     |   |   %1 = (%A)[-1 * i2 + 100];
; AFTER:     |   |   %t.033 = %1  +  %t.033;
; AFTER:     |   |   %t.033.out1 = %t.033;
; AFTER:     |   |   %t.033 = %t.033  +  1;
; AFTER:     |   |   %t.033.out = %t.033;
; AFTER:     |   |   (%B)[-2 * i2 + 50] = %t.033.out1 + 2;
; AFTER:     |   + END LOOP
; AFTER:     |   (%A)[-1 * i1 + 10] = %t.033.out;
; AFTER:     + END LOOP
; AFTER:  END REGION
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
define i32 @foo(i32* noalias nocapture %A, i32* noalias nocapture %B) #0 {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.end, %entry
  %indvars.iv38 = phi i64 [ 0, %entry ], [ %indvars.iv.next39, %for.end ]
  %t.033 = phi i32 [ 0, %entry ], [ %add4, %for.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %t.131 = phi i32 [ %t.033, %for.cond1.preheader ], [ %add4, %for.body3 ]
  %0 = sub nuw nsw i64 100, %indvars.iv
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %0
  %1 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %1, %t.131
  %add4 = add nsw i32 %add, 1
  %add5 = add nsw i32 %add, 2
  %2 = shl nsw i64 %indvars.iv, 1
  %3 = sub nuw nsw i64 50, %2
  %arrayidx8 = getelementptr inbounds i32, i32* %B, i64 %3
  store i32 %add5, i32* %arrayidx8, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body3

for.end:                                          ; preds = %for.body3
  %4 = sub nuw nsw i64 10, %indvars.iv38
  %arrayidx11 = getelementptr inbounds i32, i32* %A, i64 %4
  store i32 %add4, i32* %arrayidx11, align 4, !tbaa !1
  %indvars.iv.next39 = add nuw nsw i64 %indvars.iv38, 1
  %exitcond41 = icmp eq i64 %indvars.iv.next39, 5
  br i1 %exitcond41, label %for.end14, label %for.cond1.preheader

for.end14:                                        ; preds = %for.end
  %arrayidx15 = getelementptr inbounds i32, i32* %A, i64 1
  %5 = load i32, i32* %arrayidx15, align 4, !tbaa !1
  %6 = load i32, i32* %B, align 4, !tbaa !1
  %add17 = add i32 %5, 1
  %add18 = add i32 %add17, %6
  ret i32 %add18
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
