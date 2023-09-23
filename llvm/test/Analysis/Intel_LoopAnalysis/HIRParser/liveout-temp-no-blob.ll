; RUN: opt -passes="hir-ssa-deconstruction,print<hir>" -hir-details -disable-output < %s 2>&1 | FileCheck %s

; Verify that %t is marked as liveout out of i3 loop. We skipped livein/liveout
; processing logic in this special case where %t is never parsed as a blob.
; Both its definitions have a canonical expr which is a constant.

; CHECK: + DO i32 i1 = 0, 2, 1   <DO_LOOP>
; CHECK: |   %ld = (%aa)[0];
; CHECK: |
; CHECK: |   + DO i16 i2 = 0, 1, 1   <DO_LOOP>

; CHECK: |   |   + LiveOut symbases: [[LIVEOUTSB:[0-9]+]]
; CHECK: |   |   + DO i8 i3 = 0, 4, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   |   |   %t = 0;
; CHECK: |   |   |   <LVAL-REG> i8 0 {sb:[[LIVEOUTSB]]}

; CHECK: |   |   |   if (%ld != 0)
; CHECK: |   |   |   {
; CHECK: |   |   |      goto for.inc15;
; CHECK: |   |   |   }
; CHECK: |   |   |   %t = 5;
; CHECK: |   |   + END LOOP
; CHECK: |   |
; CHECK: |   |   for.inc15:
; CHECK: |   + END LOOP
; CHECK: + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @v(ptr nocapture noundef readonly %aa) {
entry:
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.end17, %for.end
  %storemerge6 = phi i32 [ -25, %entry ], [ %add.i, %for.end17 ]
  %ld = load i32, ptr %aa, align 4
  %tobool13.not = icmp eq i32 %ld, 0
  br label %for.cond8.preheader

for.cond8.preheader:                              ; preds = %for.inc15, %for.cond4.preheader
  %storemerge383 = phi i16 [ 0, %for.cond4.preheader ], [ %inc16, %for.inc15 ]
  br label %for.body12

for.body12:                                       ; preds = %for.inc, %for.cond8.preheader
  %storemerge391 = phi i8 [ 0, %for.cond8.preheader ], [ %inc, %for.inc ]
  br i1 %tobool13.not, label %for.inc, label %for.inc15

for.inc:                                          ; preds = %for.body12
  %inc = add nuw nsw i8 %storemerge391, 1
  %exitcond.not = icmp eq i8 %inc, 5
  br i1 %exitcond.not, label %for.inc15, label %for.body12

for.inc15:                                        ; preds = %for.inc, %for.body12
  %t = phi i8 [ 5, %for.inc ], [ 0, %for.body12 ]
  %inc16 = add nuw nsw i16 %storemerge383, 1
  %exitcond7.not = icmp eq i16 %inc16, 2
  br i1 %exitcond7.not, label %for.end17, label %for.cond8.preheader

for.end17:                                        ; preds = %for.inc15
  %t.lcssa = phi i8 [ %t, %for.inc15 ]
  %add.i = add nuw nsw i32 %storemerge6, 8
  %cmp.not = icmp eq i32 %add.i, -1
  br i1 %cmp.not, label %for.end24, label %for.cond4.preheader

for.end24:                                        ; preds = %for.end17
  %t.lcssa.lcssa = phi i8 [ %t.lcssa, %for.end17 ]
  ret void
}

