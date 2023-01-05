; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup" -disable-output -print-before=hir-temp-cleanup -print-after=hir-temp-cleanup 2>&1 < %s  | FileCheck %s

; Verify that temp cleanup does not incorrectly eliminate liveout load temp %0
; by forwarding it in %phi = %0. %0 cannot be replaced by %phi as the liveout
; value because there are multiple definitions of %phi.

 
; CHECK: Dump Before

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1] = 10;
; CHECK: |   %0 = (@B)[0][i1];
; CHECK: |   if (%t == 0)
; CHECK: |   {
; CHECK: |      %phi = 5;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %phi = %0;
; CHECK: |   }
; CHECK: |   (%C)[i1] = %phi;
; CHECK: + END LOOP


; CHECK: Dump After 

; CHECK: + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK: |   (@A)[0][i1] = 10;
; CHECK: |   %0 = (@B)[0][i1];
; CHECK: |   if (%t == 0)
; CHECK: |   {
; CHECK: |      %phi = 5;
; CHECK: |   }
; CHECK: |   else
; CHECK: |   {
; CHECK: |      %phi = %0;
; CHECK: |   }
; CHECK: |   (%C)[i1] = %phi;
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

define dso_local void @foo(i32 %t, i32* %C) {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %latch
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %latch ]
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv
  store i32 10, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv
  %0 = load i32, i32* %arrayidx2, align 4
  %cmp = icmp eq i32 %t, 0
  br i1 %cmp, label %if, label %else

if:
  br label %latch

else:
  br label %latch

latch:
  %phi = phi i32 [ %0, %else], [ 5, %if ]
  %arrayidx3 = getelementptr inbounds i32, i32* %C, i64 %indvars.iv
  store i32 %phi, i32* %arrayidx3, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %t.lcssa = phi i32 [ %0, %latch ]
  ret void
}
