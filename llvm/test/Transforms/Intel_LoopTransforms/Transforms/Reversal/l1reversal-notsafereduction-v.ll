; l1reversal-notsafereduction-v.ll
; 1-level loop, testcase for liveout without safe reduction.
; This loop is NOT allowed for reversal! 
; 
; [REASONS]
; - PreliminaryTests: YES  
; - Applicable: NO
; - Profitable: YES
; - Legal:      YES 
; -  
;
; 
; *** Source Code ***
;
;[BEFORE LOOP REVERSAL]
;
;int foo(int *restrict A, int n) {
;  int s = 0;
;  for (int i = 0; i <= 10; i++) {
;    s = A[n - i] - s;
;    A[2 * n - i] = 2 * s;
;  }
; return s;
;}
;
;[AFTER LOOP REVERSAL]
;
;int foo(int *restrict A, int n) {
;  int s = 0;
;  for (int i = 0; i <= 10; i++) {
;    s = A[n - i] - s;
;    A[2 * n - i] = 2 * s;
;  }
; return s;
;}
; 
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
;
; ===-----------------------------------===
; *** Run1: AFTER HIR Loop Reversal ***
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
;<16>            + DO i1 = 0, 10, 1   <DO_LOOP>
;<4>             |   %3 = (%A)[-1 * i1 + sext.i32.i64(%n)];
;<5>             |   %s.015 = %3  -  %s.015;
;<9>             |   (%A)[-1 * i1 + sext.i32.i64((2 * %n))] = 2 * %s.015;
;<16>            + END LOOP
;          END REGION
;
; BEFORE:          BEGIN REGION { }
; BEFORE:         + DO i1 = 0, 10, 1   <DO_LOOP>
; BEFORE:        |   %3 = (%A)[-1 * i1 + sext.i32.i64(%n)];
; BEFORE:        |   %s.015 = %3  -  %s.015;
; BEFORE:        |   (%A)[-1 * i1 + sext.i32.i64((2 * %n))] = 2 * %s.015;
; BEFORE:        + END LOOP
; BEFORE:  END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Output ***
; === -------------------------------------- ===
;
; Expected HIR output after Loop-Reversal is enabled:
;
; AFTER:          BEGIN REGION { }
; AFTER:         + DO i1 = 0, 10, 1   <DO_LOOP>
; AFTER:        |   %3 = (%A)[-1 * i1 + sext.i32.i64(%n)];
; AFTER:        |   %s.015 = %3  -  %s.015;
; AFTER:        |   (%A)[-1 * i1 + sext.i32.i64((2 * %n))] = 2 * %s.015;
; AFTER:        + END LOOP
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
define i32 @foo(i32* noalias nocapture %A, i32 %n) local_unnamed_addr #0 {
entry:
  %mul2 = shl nsw i32 %n, 1
  %0 = sext i32 %mul2 to i64
  %1 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i32 %sub1

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %s.015 = phi i32 [ 0, %entry ], [ %sub1, %for.body ]
  %2 = sub nsw i64 %1, %indvars.iv
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %2
  %3 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %sub1 = sub nsw i32 %3, %s.015
  %mul = shl nsw i32 %sub1, 1
  %4 = sub nsw i64 %0, %indvars.iv
  %arrayidx5 = getelementptr inbounds i32, i32* %A, i64 %4
  store i32 %mul, i32* %arrayidx5, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 15884) (llvm/branches/loopopt 17273)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
