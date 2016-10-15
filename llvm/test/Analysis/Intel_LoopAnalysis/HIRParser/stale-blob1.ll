; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser -hir-details | FileCheck %s

; Check that the DDRef representing %mul does not have any stale blobs.
 
; CHECK: |   |   %5 = %5  +  ((-86 + (2 * (%init + %init1 + %0)) + (-2 * %add70) + (-1 * %8)) * %add70);
; CHECK: |   |   <RVAL-REG> NON-LINEAR i32 ((-86 + (2 * (%init + %init1 + %0)) + (-2 * %add70) + (-1 * %8)) * %add70)
; CHECK-NEXT: |   |      <BLOB> NON-LINEAR i32 %8
; CHECK-NEXT: |   |      <BLOB> LINEAR i32 %init1
; CHECK-NEXT: |   |      <BLOB> LINEAR i32 %0{def@1}
; CHECK-NEXT: |   |      <BLOB> NON-LINEAR i32 %add70
; CHECK-NEXT: |   |      <BLOB> LINEAR i32 %init
; CHECK-NEXT: |   |
; CHECK-NEXT: |   + END LOOP


;Module Before HIR; ModuleID = 't9123.c'
source_filename = "t9123.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define i32 @foo(i32 %add42.lcssa212.ph, i64 %idxprom32, i32 %init, i32 %init1, i64 %n) {
entry:
  %xp = alloca [100 x i32], align 16
  %i = alloca [100 x [100 x i32]], align 16
  %ip = alloca [100 x [100 x [100 x i32]]], align 16
  %e9 = alloca [100 x i32], align 16
  %arrayidx38 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %ip, i64 0, i64 6, i64 7, i64 %idxprom32
  br label %for.body20

for.body20:                                       ; preds = %entry, %for.cond77.preheader
  %indvars.iv222 = phi i64 [ 15, %entry ], [ %indvars.iv.next223, %for.cond77.preheader ]
  %dec69.lcssa205 = phi i32 [ %init, %entry ], [ %4, %for.cond77.preheader ]
  %inc.lcssa202 = phi i32 [ %init1, %entry ], [ %9, %for.cond77.preheader ]
  %0 = phi i32 [ %add42.lcssa212.ph, %entry ], [ %add42, %for.cond77.preheader ]
  %1 = add nuw nsw i64 %indvars.iv222, 1
  %arrayidx29 = getelementptr inbounds [100 x [100 x [100 x i32]]], [100 x [100 x [100 x i32]]]* %ip, i64 0, i64 6, i64 8, i64 %1
  %2 = load i32, i32* %arrayidx29, align 4
  %add30 = add i32 %2, 7
  store i32 %add30, i32* %arrayidx38, align 4
  %arrayidx41 = getelementptr inbounds [100 x i32], [100 x i32]* %xp, i64 0, i64 %1
  %3 = load i32, i32* %arrayidx41, align 4
  %add42 = add i32 %0, %3
  %arrayidx74 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %i, i64 0, i64 %indvars.iv222, i64 %indvars.iv222
  %arrayidx74.promoted = load i32, i32* %arrayidx74, align 4
  %4 = add i32 %dec69.lcssa205, -9
  br label %for.body45

for.body45:                                       ; preds = %for.body45, %for.body20
  %indvars.iv = phi i64 [ 1, %for.body20 ], [ %indvars.iv.next, %for.body45 ]
  %5 = phi i32 [ %arrayidx74.promoted, %for.body20 ], [ %add75, %for.body45 ]
  %dec69198 = phi i32 [ %dec69.lcssa205, %for.body20 ], [ %dec69, %for.body45 ]
  %6 = phi i32 [ %inc.lcssa202, %for.body20 ], [ %inc, %for.body45 ]
  %7 = add nsw i64 %indvars.iv, -1
  %arrayidx52 = getelementptr inbounds [100 x i32], [100 x i32]* %e9, i64 0, i64 %7
  %inc = add i32 %6, 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx57 = getelementptr inbounds [100 x i32], [100 x i32]* %xp, i64 0, i64 %indvars.iv.next
  %8 = load i32, i32* %arrayidx57, align 4
  %sub54.neg = sub i32 87, %6
  %sub58 = add i32 %sub54.neg, %8
  store i32 %sub58, i32* %arrayidx57, align 4
  store i32 %sub58, i32* %arrayidx52, align 4
  %sub68 = sub i32 %inc, %sub58
  %dec69 = add i32 %dec69198, -1
  %add70 = add i32 %add42, %dec69198
  %mul = mul i32 %add70, %sub68
  %add75 = add i32 %5, %mul
  %exitcond221 = icmp eq i64 %indvars.iv.next, 10
  br i1 %exitcond221, label %for.cond77.preheader, label %for.body45

for.cond77.preheader:                             ; preds = %for.body45
  %add75.lcssa = phi i32 [ %add75, %for.body45 ]
  %9 = add i32 %inc.lcssa202, 9
  store i32 %add75.lcssa, i32* %arrayidx74, align 4
  %indvars.iv.next223 = add nsw i64 %indvars.iv222, -1
  %cmp19 = icmp ugt i64 %indvars.iv.next223, %n
  br i1 %cmp19, label %for.body20, label %checkSum.exit

checkSum.exit:                                    ; preds = %for.body.i123
  ret i32 0
}


