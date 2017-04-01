; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-interchange -hir-post-vec-complete-unroll -hir-complete-unroll-loop-trip-threshold=30 -hir-lmm -print-before=hir-lmm -print-after=hir-lmm < %s 2>&1 | FileCheck %s
;
; (This test is based on CompleteUnroll/triinner.ll)
;
; INPUT HIR:
;
;          BEGIN REGION { modified }
;<32>            + DO i1 = 0, 39, 1   <DO_LOOP>
;<2>             |   %5 = (%1)[1];
;<6>             |   (%0)[0] = i1 + zext.i32.i64(%5);
;<34>            |   %6 = (%B)[0];
;<35>            |   %7 = (%6)[2];
;<36>            |   (%2)[0] = i1 + zext.i32.i64(%7);
;<37>            |   %6 = (%B)[1];
;<38>            |   %7 = (%6)[2];
;<39>            |   (%2)[1] = i1 + zext.i32.i64(%7);
;<32>            + END LOOP
;          END REGION
;
;
; [LIMM Analysis]
;MemRefCollection, entries: 4
;  (%B)[0] {  R  R  } 0W : 2R  legal 
;  (%A)[1] {  R  } 0W : 1R  legal 
;  (%A)[2] {  R  R  } 0W : 2R  legal 
;  (%B)[1] {  R  } 0W : 1R  legal 
;
;
; LIMM's Opportunities:
; - LILH:  (4)
; - LISS:  (0)
; - LILHSS:(0)
;
;  
; CHECK: IR Dump Before HIR Loop Memory Motion
;
; CHECK: BEGIN REGION { modified }
; CHECK:        + DO i1 = 0, 39, 1   <DO_LOOP>
; CHECK:        |   %0 = (%B)[0];
; CHECK:        |   %1 = (%0)[1];
; CHECK:        |   %2 = (%A)[1];
; CHECK:        |   (%2)[0] = i1 + sext.i32.i64(%1);
; CHECK:        |   %0 = (%B)[0];
; CHECK:        |   %1 = (%0)[2];
; CHECK:        |   %2 = (%A)[2];
; CHECK:        |   (%2)[0] = i1 + sext.i32.i64(%1);
; CHECK:        |   %0 = (%B)[1];
; CHECK:        |   %1 = (%0)[2];
; CHECK:        |   %2 = (%A)[2];
; CHECK:        |   (%2)[1] = i1 + sext.i32.i64(%1);
; CHECK:        + END LOOP
; CHECK: END REGION

; *** *** 
;          
; CHECK: IR Dump After HIR Loop Memory Motion
;
; CHECK:  BEGIN REGION { modified }
; CHECK:           %limm = (%B)[0];
; CHECK:           %limm4 = (%A)[1];
; CHECK:           %limm6 = (%A)[2];
; CHECK:           %limm9 = (%B)[1];
; CHECK:        + DO i1 = 0, 39, 1   <DO_LOOP>
; CHECK:        |   %0 = %limm;
; CHECK:        |   %1 = (%0)[1];
; CHECK:        |   %2 = %limm4;
; CHECK:        |   (%2)[0] = i1 + sext.i32.i64(%1);
; CHECK:        |   %0 = %limm;
; CHECK:        |   %1 = (%0)[2];
; CHECK:        |   %2 = %limm6;
; CHECK:        |   (%2)[0] = i1 + sext.i32.i64(%1);
; CHECK:        |   %0 = %limm9;
; CHECK:        |   %1 = (%0)[2];
; CHECK:        |   %2 = %limm6;
; CHECK:        |   (%2)[1] = i1 + sext.i32.i64(%1);
; CHECK:        + END LOOP
; CHECK:  END REGION
;
;
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
; noalias nocapture 
define i32 @_Z3fooPPiS0_(i32** noalias %A, i32** noalias %B) #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc14
  %k.04 = phi i64 [ 0, %entry ], [ %inc15, %for.inc14 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body, %for.inc11
  %i.03 = phi i64 [ 0, %for.body ], [ %inc12, %for.inc11 ]
  %cmp51 = icmp slt i64 0, %i.03
  br i1 %cmp51, label %for.body6.lr.ph, label %for.end

for.body6.lr.ph:                                  ; preds = %for.body3
  br label %for.body6

for.body6:                                        ; preds = %for.body6.lr.ph, %for.inc
  %j.02 = phi i64 [ 0, %for.body6.lr.ph ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds i32*, i32** %B, i64 %j.02
  %0 = load i32*, i32** %arrayidx, align 8
  %arrayidx7 = getelementptr inbounds i32, i32* %0, i64 %i.03
  %1 = load i32, i32* %arrayidx7, align 4
  %conv = sext i32 %1 to i64
  %add = add nsw i64 %conv, %k.04
  %conv8 = trunc i64 %add to i32
  %arrayidx9 = getelementptr inbounds i32*, i32** %A, i64 %i.03
  %2 = load i32*, i32** %arrayidx9, align 8
  %arrayidx10 = getelementptr inbounds i32, i32* %2, i64 %j.02
  store i32 %conv8, i32* %arrayidx10, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body6
  %inc = add nsw i64 %j.02, 1
  %cmp5 = icmp slt i64 %inc, %i.03
  br i1 %cmp5, label %for.body6, label %for.cond4.for.end_crit_edge

for.cond4.for.end_crit_edge:                      ; preds = %for.inc
  br label %for.end

for.end:                                          ; preds = %for.cond4.for.end_crit_edge, %for.body3
  br label %for.inc11

for.inc11:                                        ; preds = %for.end
  %inc12 = add nsw i64 %i.03, 1
  %cmp2 = icmp slt i64 %inc12, 3
  br i1 %cmp2, label %for.body3, label %for.end13

for.end13:                                        ; preds = %for.inc11
  br label %for.inc14

for.inc14:                                        ; preds = %for.end13
  %inc15 = add nsw i64 %k.04, 1
  %cmp = icmp slt i64 %inc15, 40
  br i1 %cmp, label %for.body, label %for.end16

for.end16:                                        ; preds = %for.inc14
  %arrayidx17 = getelementptr inbounds i32*, i32** %A, i64 2
  %3 = load i32*, i32** %arrayidx17, align 8
  %arrayidx18 = getelementptr inbounds i32, i32* %3, i64 3
  %4 = load i32, i32* %arrayidx18, align 4
  ret i32 %4
}


attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 2121) (llvm/branches/loopopt 3770)"}
