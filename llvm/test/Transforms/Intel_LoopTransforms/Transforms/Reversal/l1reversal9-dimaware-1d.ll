; Sanity Test(s) on HIR Loop Reversal: l1 that is dimension aware (1D case)
; 
; l1reversal9-dimaware-1d.ll: 
; 1-level loop, sanity testcase, that is dimension-aware on 1D array
; 
; Loop is GOOD for reversal,
; 
; [REASONS]
; - Prelimary:   _
; - Applicable:  _
; - Profitable:  _
; - Legal:       _
; 
; *** Source Code ***
;
; [BEFORE LOOP REVERSAL]
;
;// stride-aware 1D
;int foo(int *restrict A, char *restrict C) {
; int i;
;  //loop i
;  for (i = 0; i <= 4; i++) {
;    A[100 - i] = C[i];
;    C[50 - i] = A[2 * i];
;  }
;
;  return A[1] + C[0] + 1;
;}
;
; [AFTER LOOP REVERSAL: reversed]
; 
;int foo(int *restrict A, char *restrict C) {
; int i;
;  //loop i
;  for (i = 0; i <= 4; i++) {
;    A[96 + i] = C[4-i];
;    C[46 + i] = A[8 - 2 * i];
;  }
;
;  return A[1] + C[0] + 1;
;}
;
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
; 
;          BEGIN REGION { }
;<21>            + DO i1 = 0, 4, 1   <DO_LOOP>
;<3>             |   %0 = (%C)[i1];
;<7>             |   (%A)[-1 * i1 + 100] = %0;
;<10>            |   %3 = (%A)[2 * i1];
;<14>            |   (%C)[-1 * i1 + 50] = %3;
;<21>            + END LOOP
;          END REGION
; 
; BEFORE:  BEGIN REGION { }
; BEFORE:    + DO i1 = 0, 4, 1   <DO_LOOP>
; BEFORE:    |   %0 = (%C)[i1];
; BEFORE:    |   (%A)[-1 * i1 + 100] = %0;
; BEFORE:    |   %3 = (%A)[2 * i1];
; BEFORE:    |   (%C)[-1 * i1 + 50] = %3;
; BEFORE:    + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: AFTER HIR Loop Reversal Output ***
; *** NEED TO TUNE THE COST MODEL !!!        ***
; === -------------------------------------- ===
; Expected output AFTER	 Loop Reversal 
;
;          BEGIN REGION { modified }
;<21>            + DO i1 = 0, 4, 1   <DO_LOOP>
;<3>             |   %0 = (%C)[-1 * i1 + 4];
;<7>             |   (%A)[i1 + 96] = %0;
;<10>            |   %3 = (%A)[-2 * i1 + 8];
;<14>            |   (%C)[i1 + 46] = %3;
;<21>            + END LOOP
;          END REGION
; 
; AFTER:   BEGIN REGION { modified }
; AFTER:         + DO i1 = 0, 4, 1   <DO_LOOP>
; AFTER:         |   %0 = (%C)[-1 * i1 + 4];
; AFTER:         |   (%A)[i1 + 96] = %0;
; AFTER:         |   %3 = (%A)[-2 * i1 + 8];
; AFTER:         |   (%C)[i1 + 46] = %3;
; AFTER:         + END LOOP
; AFTER:   END REGION
;
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
define i32 @foo(i32* noalias nocapture %A, i8* noalias nocapture %C) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i8, i8* %C, i64 %indvars.iv
  %0 = load i8, i8* %arrayidx, align 1, !tbaa !1
  %conv = sext i8 %0 to i32
  %1 = sub nuw nsw i64 100, %indvars.iv
  %arrayidx2 = getelementptr inbounds i32, i32* %A, i64 %1
  store i32 %conv, i32* %arrayidx2, align 4, !tbaa !4
  %2 = shl nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds i32, i32* %A, i64 %2
  %3 = load i32, i32* %arrayidx4, align 4, !tbaa !4
  %conv5 = trunc i32 %3 to i8
  %4 = sub nuw nsw i64 50, %indvars.iv
  %arrayidx8 = getelementptr inbounds i8, i8* %C, i64 %4
  store i8 %conv5, i8* %arrayidx8, align 1, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 5
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx9 = getelementptr inbounds i32, i32* %A, i64 1
  %5 = load i32, i32* %arrayidx9, align 4, !tbaa !4
  %6 = load i8, i8* %C, align 1, !tbaa !1
  %conv11 = sext i8 %6 to i32
  %add = add i32 %5, 1
  %add12 = add i32 %add, %conv11
  ret i32 %add12
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 11232) (llvm/branches/loopopt 12266)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"omnipotent char", !3, i64 0}
!3 = !{!"Simple C/C++ TBAA"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !2, i64 0}
