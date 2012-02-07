; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample4.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %call = tail call i32 @get_local_id(i32 0) nounwind
  %conv = zext i32 %call to i64
  %cmp3 = icmp slt i64 %conv, %n
  %cmp54 = icmp sgt i64 %n, 0
  %or.cond5 = and i1 %cmp3, %cmp54
  br i1 %or.cond5, label %while.body.preheader, label %while.end

while.body.preheader:                             ; preds = %entry
  br label %while.body

while.body:                                       ; preds = %while.body.preheader, %while.body
  %tmp1617 = phi i64 [ %add, %while.body ], [ %conv, %while.body.preheader ]
  %tmp1326 = phi i64 [ %inc, %while.body ], [ 0, %while.body.preheader ]
  %inc = add nsw i64 %tmp1326, 1
  %arrayidx = getelementptr inbounds i64* %A, i64 %inc
  %tmp9 = load i64* %arrayidx, align 8
  %add = add nsw i64 %tmp9, %tmp1617
  %call11 = tail call i32 @get_local_id(i32 0) nounwind
  %conv12 = zext i32 %call11 to i64
  %arrayidx15 = getelementptr inbounds i64* %B, i64 %inc
  store i64 %conv12, i64* %arrayidx15, align 8
  %cmp = icmp slt i64 %add, %n
  %cmp5 = icmp slt i64 %inc, %n
  %or.cond = and i1 %cmp, %cmp5
  br i1 %or.cond, label %while.body, label %while.end.loopexit

while.end.loopexit:                               ; preds = %while.body
  br label %while.end

while.end:                                        ; preds = %while.end.loopexit, %entry
  %tmp161.lcssa = phi i64 [ %conv, %entry ], [ %add, %while.end.loopexit ]
  %div = sdiv i64 %n, 2
  %arrayidx19 = getelementptr inbounds i64* %B, i64 %div
  store i64 %tmp161.lcssa, i64* %arrayidx19, align 8
  ret void
}

declare i32 @get_local_id(i32)
