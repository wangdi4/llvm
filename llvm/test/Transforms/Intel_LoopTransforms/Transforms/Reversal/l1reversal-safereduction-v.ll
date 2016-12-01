; l1reversal-safereduction-v.ll
; 1-level loop, testcase for safe reduction.
; The loop has a temp liveout, which is also a safe reduction.
; This loop is allowed for reversal.
; 
; [REASONS]
; - PreliminaryTests: YES (Revised: allow any loop's temp liveout to pass, collect it/them);  
; - Applicable: YES (Revised: allow test to return true if the liveOut is always a reduction)
; - Profitable: YES
; - Legal:      YES 
; 
; *** Source Code ***
;
;[BEFORE LOOP REVERSAL]
;
;int foo(int *restrict A, int n) {
;  int s = 0;
;  for (int i = 0; i <= 10; i++) {
;    s += A[n - i]; //safe reduction on s
;  }
; return s;
;}
;
;[AFTER LOOP REVERSAL] {Expect the reversal to happen}
;
;int foo(int *restrict A, int n) {
;  int s = 0;
;  for (int i = 0; i <= 10; i++) {
;    s += A[n - 10 + i];
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
; BEFORE:  BEGIN REGION { }
; BEFORE:       + DO i1 = 0, 10, 1   <DO_LOOP>
; BEFORE        |   %2 = (%A)[-1 * i1 + sext.i32.i64(%n)];
; BEFORE        |   %s.06 = %2  +  %s.06;
; BEFORE        + END LOOP
; BEFORE   END REGION
;
;
; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Output ***
; === -------------------------------------- ===
;
; Expected HIR output after Loop-Reversal is enabled:
;
; AFTER:   BEGIN REGION { modified }
; AFTER:         + DO i1 = 0, 10, 1   <DO_LOOP>
; AFTER:         |   %2 = (%A)[i1 + sext.i32.i64(%n) + -10];
; AFTER:         |   %s.06 = %2  +  %s.06;
; AFTER:         + END LOOP
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

; Function Attrs: norecurse nounwind readonly uwtable
define i32 @foo(i32* noalias nocapture readonly %A, i32 %n) #0 {
entry:
  %0 = sext i32 %n to i64
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret i32 %add

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %s.06 = phi i32 [ 0, %entry ], [ %add, %for.body ]
  %1 = sub nsw i64 %0, %indvars.iv
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %1
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %2, %s.06
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %for.cond.cleanup, label %for.body
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind readonly uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 12506) (llvm/branches/loopopt 12537)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
