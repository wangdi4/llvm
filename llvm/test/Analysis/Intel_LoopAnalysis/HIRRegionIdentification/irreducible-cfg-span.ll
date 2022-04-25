; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification -disable-hir-create-fusion-regions=0 | FileCheck %s
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-hir-create-fusion-regions=0 2>&1 | FileCheck %s

; Verify that two top loops are placed in the different regions bacause of irreducible CFG.

; CHECK: Region 1
; CHECK: EntryBB: %for.body
; CHECK: Member BBlocks: %for.body

; CHECK: Region 2
; CHECK: EntryBB: %for.body13
; CHECK: Member BBlocks: %for.body13

source_filename = "irr.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = common dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i32 %n, i32 %k, i32 %m) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  %tobool = icmp eq i32 %n, 0
  br i1 %tobool, label %L2, label %L1

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv32 = phi i64 [ 0, %entry ], [ %indvars.iv.next33, %for.body ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv32
  %0 = trunc i64 %indvars.iv32 to i32
  store i32 %0, i32* %arrayidx, align 4
  %indvars.iv.next33 = add nuw nsw i64 %indvars.iv32, 1
  %exitcond34 = icmp eq i64 %indvars.iv.next33, 50
  br i1 %exitcond34, label %for.cond.cleanup, label %for.body

L1:                                               ; preds = %L2, %for.cond.cleanup
  %1 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 0), align 16
  %inc1 = add nsw i32 %1, 1
  store i32 %inc1, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 0), align 16
  %tobool2 = icmp eq i32 %k, 0
  br i1 %tobool2, label %if.end8, label %L2

L2:                                               ; preds = %L1, %for.cond.cleanup
  %2 = load i32, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 1), align 4
  %inc4 = add nsw i32 %2, 1
  store i32 %inc4, i32* getelementptr inbounds ([100 x i32], [100 x i32]* @A, i64 0, i64 1), align 4
  %tobool5 = icmp eq i32 %m, 0
  br i1 %tobool5, label %if.end8, label %L1

if.end8:                                          ; preds = %L1, %L2
  br label %for.body13

for.cond.cleanup12:                               ; preds = %for.body13
  ret void

for.body13:                                       ; preds = %for.body13, %if.end8
  %indvars.iv = phi i64 [ 0, %if.end8 ], [ %indvars.iv.next, %for.body13 ]
  %arrayidx15 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx15, align 4
  %4 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %3, %4
  %arrayidx17 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  store i32 %add, i32* %arrayidx17, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.cond.cleanup12, label %for.body13
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }


