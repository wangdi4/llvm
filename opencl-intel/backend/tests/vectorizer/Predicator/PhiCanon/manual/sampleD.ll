; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleD.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.end26.loopexit, %for.end26.loopexit9
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %call4 = tail call i32 @get_local_id(i32 0) nounwind
  %cmp7 = icmp eq i32 %call4, 0
  br i1 %cmp7, label %for.end26, label %for.cond3.preheader.lr.ph

for.cond3.preheader.lr.ph:                        ; preds = %entry
  %div = sdiv i64 %n, 10
  %cmp62 = icmp sgt i64 %n, 9
  br i1 %cmp62, label %for.body8.lr.ph.us.preheader, label %for.end.preheader

for.body8.lr.ph.us.preheader:                     ; preds = %for.cond3.preheader.lr.ph
  br label %for.body8.lr.ph.us

for.end.preheader:                                ; preds = %for.cond3.preheader.lr.ph
  br label %for.end

for.end.us:                                       ; preds = %for.body8.us
  %call16.us = tail call i32 @get_local_id(i32 0) nounwind
  %conv17.us = zext i32 %call16.us to i64
  %arrayidx20.us = getelementptr inbounds i64* %B, i64 %storemerge8.us
  %tmp21.us = load i64* %arrayidx20.us, align 8
  %xor22.us = xor i64 %tmp21.us, %conv17.us
  store i64 %xor22.us, i64* %arrayidx20.us, align 8
  %inc25.us = add nsw i64 %storemerge8.us, 1
  %call.us = tail call i32 @get_local_id(i32 0) nounwind
  %conv.us = zext i32 %call.us to i64
  %cmp.us = icmp slt i64 %inc25.us, %conv.us
  br i1 %cmp.us, label %for.body8.lr.ph.us, label %for.end26.loopexit9

for.body8.us:                                     ; preds = %for.body8.lr.ph.us, %for.body8.us
  %storemerge13.us = phi i64 [ 0, %for.body8.lr.ph.us ], [ %inc.us, %for.body8.us ]
  %tmp11.us = load i64* %arrayidx.us, align 8
  store i64 %tmp11.us, i64* %arrayidx14.us, align 8
  %inc.us = add nsw i64 %storemerge13.us, 1
  %cmp6.us = icmp slt i64 %inc.us, %div
  br i1 %cmp6.us, label %for.body8.us, label %for.end.us

for.body8.lr.ph.us:                               ; preds = %for.body8.lr.ph.us.preheader, %for.end.us
  %storemerge8.us = phi i64 [ %inc25.us, %for.end.us ], [ 0, %for.body8.lr.ph.us.preheader ]
  %add.us = add nsw i64 %storemerge8.us, 10
  %arrayidx.us = getelementptr inbounds i64* %A, i64 %add.us
  %xor.us = xor i64 %storemerge8.us, 90
  %arrayidx14.us = getelementptr inbounds i64* %A, i64 %xor.us
  br label %for.body8.us

for.end:                                          ; preds = %for.end.preheader, %for.end
  %storemerge8 = phi i64 [ %inc25, %for.end ], [ 0, %for.end.preheader ]
  %call16 = tail call i32 @get_local_id(i32 0) nounwind
  %conv17 = zext i32 %call16 to i64
  %arrayidx20 = getelementptr inbounds i64* %B, i64 %storemerge8
  %tmp21 = load i64* %arrayidx20, align 8
  %xor22 = xor i64 %tmp21, %conv17
  store i64 %xor22, i64* %arrayidx20, align 8
  %inc25 = add nsw i64 %storemerge8, 1
  %call = tail call i32 @get_local_id(i32 0) nounwind
  %conv = zext i32 %call to i64
  %cmp = icmp slt i64 %inc25, %conv
  br i1 %cmp, label %for.end, label %for.end26.loopexit

for.end26.loopexit:                               ; preds = %for.end
  br label %for.end26

for.end26.loopexit9:                              ; preds = %for.end.us
  br label %for.end26

for.end26:                                        ; preds = %for.end26.loopexit9, %for.end26.loopexit, %entry
  %call27 = tail call i32 @get_local_id(i32 2) nounwind
  ret void
}

declare i32 @get_local_id(i32)
