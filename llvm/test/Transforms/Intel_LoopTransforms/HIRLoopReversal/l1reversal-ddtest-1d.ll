; l1reversal-ddtest-1d.ll
; (1-level loop, sanity testcase, for 1D array dependence tests)
; 
; *** Source Code ***
;
;[BEFORE LOOP REVERSAL]
;
;//testing for data dependence for 1D array
;int foo(int A[100], int B[100]) {
;  int i = 0;
;
;  //Loop0: (=): valid for reversal
;  //Status: GOOD, found 2 (=)s
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i] + 1;
;  }
;
;  //Loop1: (<): invalid for reversal:
;  //Status: no out-going edge
;  //Suggestion: check any incoming edge?
;  for (i = 1; i <= 11; i++) {
;    A[i] = A[i - 1] + 1;
;  }
;
;  //Loop2: (>): invalid for reversal
;  //Status: DDAnalysis flips the DDEdge, so it becomes: (<)
;  //        Won't impact result though!
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i + 1] + 1;
;  }
;
;  //Loop3: (<=): invalid for reversal
;  //Status: many artificial DVs created, but won't change the result
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i - 1] + A[i] + 1;
;  }
;
;  //Loop4: (>=): invalid for reversal
;  //Status:> is turned into <, won't impact the result!
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i + 1] + A[i] + 1;
;  }
;
;  //Loop5: (><): invalid for reversal
;  //Status:> is turned into <, some artificial dependencies created, won't impact the result!
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i + 1] + A[i - 1] + 1;
;  }
;
;  //Loop6: (=,<,>: *): invalid for reversal
;  //Status: > is changed into <, won't impact analysis result
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i + 1] + A[i - 1] + A[i] + 1;
;  }
;
;  return A[1] + B[1] + 1;
;}
;
;[AFTER LOOP REVERSAL]{Expected!}
;
;//testing for data dependence for 1D array
;int foo(int A[100], int B[100]) {
;  int i = 0;
;
;  // Loop0: (=): valid for reversal
;  for (i = 0; i <= 10; i++) {
;
;    A[i] = A[i] + 1;
;  }
;
;  // Loop1: (<): invalid for reversal
; for (i = 0; i <= 10; i++) {
;    A[i + 1] = A[i] + 1;
;  }
;
;  // Loop2: (>): invalid for reversal
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i + 1] + 1;
;  }
;
;  // Loop3: (<=): invalid for reversal
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i - 1] + A[i] + 1;
;  }
;
;  // Loop4: (>=): invalid for reversal
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i + 1] + A[i] + 1;
;  }
;
;  // Loop5: (><): invalid for reversal
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i + 1] + A[i - 1] + 1;
;  }
;
;  // Loop6: (*:=,<,>): invalid for reversal
;  for (i = 0; i <= 10; i++) {
;    A[i] = A[i + 1] + A[i - 1] + A[i] + 1;
;  }
;
;  return A[1] + B[1] + 1;
;}
; 
; ===-----------------------------------===
; *** Run0: BEFORE HIR Loop Reversal ***
; ===-----------------------------------===
; RUN: opt -hir-ssa-deconstruction -hir-loop-reversal -print-before=hir-loop-reversal -S 2>&1	\
; RUN: < %s  |	FileCheck %s -check-prefix=BEFORE 
;
;; ===-----------------------------------===
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
;<73>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<3>          |   %0 = (%A)[i1];
;<5>          |   (%A)[i1] = %0 + 1;
;<73>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<74>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<14>         |   %hir.de.ssa.copy0.out = %3;
;<16>         |   %4 = (%A)[i1];
;<18>         |   %3 = %hir.de.ssa.copy0.out + 1  +  %4;
;<19>         |   (%A)[i1] = %3;
;<74>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<75>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<30>         |   %6 = (%A)[i1 + 1];
;<34>         |   (%A)[i1] = %6 + %5 + 1;
;<36>         |   %5 = %6;
;<75>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<76>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<45>         |   %8 = (%A)[i1 + 1];
;<47>         |   %7 = %8 + 1  +  %7;
;<49>         |   (%A)[i1] = %7;
;<76>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<77>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<57>         |   %hir.de.ssa.copy4.out = %10;
;<58>         |   %hir.de.ssa.copy3.out = %9;
;<61>         |   %11 = (%A)[i1 + 1];
;<65>         |   %10 = %11 + %hir.de.ssa.copy4.out + 1  +  %hir.de.ssa.copy3.out;
;<66>         |   (%A)[i1] = %10;
;<68>         |   %9 = %11;
;<77>         + END LOOP
;          END REGION
;
; ...-...-...
; 
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 10, 1   <DO_LOOP>
; BEFORE:     |   %0 = (%A)[i1];
; BEFORE:     |   (%A)[i1] = %0 + 1;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 10, 1   <DO_LOOP>
; BEFORE:     |   %hir.de.ssa.copy0.out = %3;
; BEFORE:     |   %4 = (%A)[i1];
; BEFORE:     |   %3 = %hir.de.ssa.copy0.out + 1  +  %4;
; BEFORE:     |   (%A)[i1] = %3;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 10, 1   <DO_LOOP>
; BEFORE:     |   %6 = (%A)[i1 + 1];
; BEFORE:     |   (%A)[i1] = %6 + %5 + 1;
; BEFORE:     |   %5 = %6;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 10, 1   <DO_LOOP>
; BEFORE:     |   %8 = (%A)[i1 + 1];
; BEFORE:     |   %7 = %8 + 1  +  %7;
; BEFORE:     |   (%A)[i1] = %7;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; BEFORE:  BEGIN REGION { }
; BEFORE:     + DO i1 = 0, 10, 1   <DO_LOOP>
; BEFORE:     |   %hir.de.ssa.copy4.out = %10;
; BEFORE:     |   %hir.de.ssa.copy3.out = %9;
; BEFORE:     |   %11 = (%A)[i1 + 1];
; BEFORE:     |   %10 = %11 + %hir.de.ssa.copy4.out + 1  +  %hir.de.ssa.copy3.out;
; BEFORE:     |   (%A)[i1] = %10;
; BEFORE:     |   %9 = %11;
; BEFORE:     + END LOOP
; BEFORE:  END REGION
;
; === -------------------------------------- ===
; *** Tests1: With HIR Loop Reversal Output ***
; === -------------------------------------- ===
;
; Expected HIR output after Loop-Reversal is enabled:
;
;          BEGIN REGION { }
;<73>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<3>          |   %0 = (%A)[i1];
;<5>          |   (%A)[i1] = %0 + 1;
;<73>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<74>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<14>         |   %hir.de.ssa.copy0.out = %3;
;<16>         |   %4 = (%A)[i1];
;<18>         |   %3 = %hir.de.ssa.copy0.out + 1  +  %4;
;<19>         |   (%A)[i1] = %3;
;<74>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<75>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<30>         |   %6 = (%A)[i1 + 1];
;<34>         |   (%A)[i1] = %6 + %5 + 1;
;<36>         |   %5 = %6;
;<75>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<76>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<45>         |   %8 = (%A)[i1 + 1];
;<47>         |   %7 = %8 + 1  +  %7;
;<49>         |   (%A)[i1] = %7;
;<76>         + END LOOP
;          END REGION
;
;          BEGIN REGION { }
;<77>         + DO i1 = 0, 10, 1   <DO_LOOP>
;<57>         |   %hir.de.ssa.copy4.out = %10;
;<58>         |   %hir.de.ssa.copy3.out = %9;
;<61>         |   %11 = (%A)[i1 + 1];
;<65>         |   %10 = %11 + %hir.de.ssa.copy4.out + 1  +  %hir.de.ssa.copy3.out;
;<66>         |   (%A)[i1] = %10;
;<68>         |   %9 = %11;
;<77>         + END LOOP
;          END REGION
;
; ...-^^^vvv^^^-...
; 
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 10, 1   <DO_LOOP>
; AFTER:     |   %0 = (%A)[i1];
; AFTER:     |   (%A)[i1] = %0 + 1;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 10, 1   <DO_LOOP>
; AFTER:     |   %hir.de.ssa.copy0.out = %3;
; AFTER:     |   %4 = (%A)[i1];
; AFTER:     |   %3 = %hir.de.ssa.copy0.out + 1  +  %4;
; AFTER:     |   (%A)[i1] = %3;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 10, 1   <DO_LOOP>
; AFTER:     |   %6 = (%A)[i1 + 1];
; AFTER:     |   (%A)[i1] = %6 + %5 + 1;
; AFTER:     |   %5 = %6;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 10, 1   <DO_LOOP>
; AFTER:     |   %8 = (%A)[i1 + 1];
; AFTER:     |   %7 = %8 + 1  +  %7;
; AFTER:     |   (%A)[i1] = %7;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; AFTER:  BEGIN REGION { }
; AFTER:     + DO i1 = 0, 10, 1   <DO_LOOP>
; AFTER:     |   %hir.de.ssa.copy4.out = %10;
; AFTER:     |   %hir.de.ssa.copy3.out = %9;
; AFTER:     |   %11 = (%A)[i1 + 1];
; AFTER:     |   %10 = %11 + %hir.de.ssa.copy4.out + 1  +  %hir.de.ssa.copy3.out;
; AFTER:     |   (%A)[i1] = %10;
; AFTER:     |   %9 = %11;
; AFTER:     + END LOOP
; AFTER:  END REGION
;
; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define i32 @foo(i32* nocapture %A, i32* nocapture readonly %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv172 = phi i64 [ 0, %entry ], [ %indvars.iv.next173, %for.body ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv172
  %0 = load i32, i32* %arrayidx, align 4, !tbaa !1
  %add = add nsw i32 %0, 1
  store i32 %add, i32* %arrayidx, align 4, !tbaa !1
  %indvars.iv.next173 = add nuw nsw i64 %indvars.iv172, 1
  %exitcond174 = icmp eq i64 %indvars.iv.next173, 11
  br i1 %exitcond174, label %for.body5.preheader, label %for.body

for.body5.preheader:                              ; preds = %for.body
  %.pre = load i32, i32* %A, align 4, !tbaa !1
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %for.body5.preheader
  %1 = phi i32 [ %.pre, %for.body5.preheader ], [ %add8, %for.body5 ]
  %indvars.iv169 = phi i64 [ 0, %for.body5.preheader ], [ %indvars.iv.next170, %for.body5 ]
  %add8 = add nsw i32 %1, 1
  %indvars.iv.next170 = add nuw nsw i64 %indvars.iv169, 1
  %arrayidx11 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv.next170
  store i32 %add8, i32* %arrayidx11, align 4, !tbaa !1
  %exitcond171 = icmp eq i64 %indvars.iv.next170, 11
  br i1 %exitcond171, label %for.body17, label %for.body5

for.body17:                                       ; preds = %for.body5, %for.body17
  %indvars.iv166 = phi i64 [ %indvars.iv.next167, %for.body17 ], [ 0, %for.body5 ]
  %indvars.iv.next167 = add nuw nsw i64 %indvars.iv166, 1
  %arrayidx20 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv.next167
  %2 = load i32, i32* %arrayidx20, align 4, !tbaa !1
  %add21 = add nsw i32 %2, 1
  %arrayidx23 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv166
  store i32 %add21, i32* %arrayidx23, align 4, !tbaa !1
  %exitcond168 = icmp eq i64 %indvars.iv.next167, 11
  br i1 %exitcond168, label %for.body29.preheader, label %for.body17

for.body29.preheader:                             ; preds = %for.body17
  %arrayidx31.phi.trans.insert = getelementptr inbounds i32, i32* %A, i64 -1
  %.pre175 = load i32, i32* %arrayidx31.phi.trans.insert, align 4, !tbaa !1
  br label %for.body29

for.body29:                                       ; preds = %for.body29, %for.body29.preheader
  %3 = phi i32 [ %.pre175, %for.body29.preheader ], [ %add35, %for.body29 ]
  %indvars.iv162 = phi i64 [ 0, %for.body29.preheader ], [ %indvars.iv.next163, %for.body29 ]
  %arrayidx33 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv162
  %4 = load i32, i32* %arrayidx33, align 4, !tbaa !1
  %add34 = add i32 %3, 1
  %add35 = add i32 %add34, %4
  store i32 %add35, i32* %arrayidx33, align 4, !tbaa !1
  %indvars.iv.next163 = add nuw nsw i64 %indvars.iv162, 1
  %exitcond165 = icmp eq i64 %indvars.iv.next163, 11
  br i1 %exitcond165, label %for.body43.preheader, label %for.body29

for.body43.preheader:                             ; preds = %for.body29
  %.pre176 = load i32, i32* %A, align 4, !tbaa !1
  br label %for.body43

for.body43:                                       ; preds = %for.body43, %for.body43.preheader
  %5 = phi i32 [ %.pre176, %for.body43.preheader ], [ %6, %for.body43 ]
  %indvars.iv159 = phi i64 [ 0, %for.body43.preheader ], [ %indvars.iv.next160, %for.body43 ]
  %indvars.iv.next160 = add nuw nsw i64 %indvars.iv159, 1
  %arrayidx46 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv.next160
  %6 = load i32, i32* %arrayidx46, align 4, !tbaa !1
  %arrayidx48 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv159
  %add49 = add i32 %6, 1
  %add50 = add i32 %add49, %5
  store i32 %add50, i32* %arrayidx48, align 4, !tbaa !1
  %exitcond161 = icmp eq i64 %indvars.iv.next160, 11
  br i1 %exitcond161, label %for.body58.preheader, label %for.body43

for.body58.preheader:                             ; preds = %for.body43
  %.pre177 = load i32, i32* %arrayidx31.phi.trans.insert, align 4, !tbaa !1
  br label %for.body58

for.body58:                                       ; preds = %for.body58, %for.body58.preheader
  %7 = phi i32 [ %.pre177, %for.body58.preheader ], [ %add66, %for.body58 ]
  %indvars.iv155 = phi i64 [ 0, %for.body58.preheader ], [ %indvars.iv.next156, %for.body58 ]
  %indvars.iv.next156 = add nuw nsw i64 %indvars.iv155, 1
  %arrayidx61 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv.next156
  %8 = load i32, i32* %arrayidx61, align 4, !tbaa !1
  %add65 = add i32 %8, 1
  %add66 = add i32 %add65, %7
  %arrayidx68 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv155
  store i32 %add66, i32* %arrayidx68, align 4, !tbaa !1
  %exitcond158 = icmp eq i64 %indvars.iv.next156, 11
  br i1 %exitcond158, label %for.body74.preheader, label %for.body58

for.body74.preheader:                             ; preds = %for.body58
  %.pre178 = load i32, i32* %arrayidx31.phi.trans.insert, align 4, !tbaa !1
  %.pre179 = load i32, i32* %A, align 4, !tbaa !1
  br label %for.body74

for.body74:                                       ; preds = %for.body74, %for.body74.preheader
  %9 = phi i32 [ %.pre179, %for.body74.preheader ], [ %11, %for.body74 ]
  %10 = phi i32 [ %.pre178, %for.body74.preheader ], [ %add85, %for.body74 ]
  %indvars.iv = phi i64 [ 0, %for.body74.preheader ], [ %indvars.iv.next, %for.body74 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx77 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv.next
  %11 = load i32, i32* %arrayidx77, align 4, !tbaa !1
  %arrayidx83 = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %add81 = add i32 %11, 1
  %add84 = add i32 %add81, %10
  %add85 = add i32 %add84, %9
  store i32 %add85, i32* %arrayidx83, align 4, !tbaa !1
  %exitcond = icmp eq i64 %indvars.iv.next, 11
  br i1 %exitcond, label %for.end90, label %for.body74

for.end90:                                        ; preds = %for.body74
  %arrayidx91 = getelementptr inbounds i32, i32* %A, i64 1
  %12 = load i32, i32* %arrayidx91, align 4, !tbaa !1
  %arrayidx92 = getelementptr inbounds i32, i32* %B, i64 1
  %13 = load i32, i32* %arrayidx92, align 4, !tbaa !1
  %add93 = add i32 %12, 1
  %add94 = add i32 %add93, %13
  ret i32 %add94
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2121) (llvm/branches/loopopt 3727)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
