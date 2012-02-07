; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleC.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.cond22.preheader, %if.end.loopexit13
; CHECK: phi-split-bb1:                                    ; preds = %for.cond.preheader, %if.end.loopexit
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %call = tail call i32 @get_global_id(i32 2) nounwind
  %tobool = icmp eq i32 %call, 0
  %div25 = sdiv i64 %n, 30
  %cmp2610 = icmp sgt i64 %n, 29
  br i1 %tobool, label %for.cond22.preheader, label %for.cond.preheader

for.cond22.preheader:                             ; preds = %entry
  br i1 %cmp2610, label %for.body28.preheader, label %if.end

for.body28.preheader:                             ; preds = %for.cond22.preheader
  br label %for.body28

for.cond.preheader:                               ; preds = %entry
  br i1 %cmp2610, label %for.cond3.preheader.preheader, label %if.end

for.cond3.preheader.preheader:                    ; preds = %for.cond.preheader
  br label %for.cond3.preheader

for.cond3.preheader:                              ; preds = %for.cond3.preheader.preheader, %for.inc16
  %storemerge18 = phi i64 [ %inc18, %for.inc16 ], [ 0, %for.cond3.preheader.preheader ]
  %call53 = tail call i32 @get_local_id(i32 0) nounwind
  %cmp65 = icmp eq i32 %call53, 0
  br i1 %cmp65, label %for.inc16, label %for.body8.lr.ph

for.body8.lr.ph:                                  ; preds = %for.cond3.preheader
  %arrayidx = getelementptr inbounds i64* %B, i64 %storemerge18
  br label %for.body8

for.body8:                                        ; preds = %for.body8.lr.ph, %for.body8
  %storemerge26 = phi i64 [ 0, %for.body8.lr.ph ], [ %inc, %for.body8 ]
  %tmp11 = load i64* %arrayidx, align 8
  %arrayidx14 = getelementptr inbounds i64* %A, i64 %storemerge26
  store i64 %tmp11, i64* %arrayidx14, align 8
  %inc = add nsw i64 %storemerge26, 1
  %call5 = tail call i32 @get_local_id(i32 0) nounwind
  %conv = zext i32 %call5 to i64
  %cmp6 = icmp slt i64 %inc, %conv
  br i1 %cmp6, label %for.body8, label %for.inc16.loopexit

for.inc16.loopexit:                               ; preds = %for.body8
  br label %for.inc16

for.inc16:                                        ; preds = %for.inc16.loopexit, %for.cond3.preheader
  %inc18 = add nsw i64 %storemerge18, 1
  %cmp = icmp slt i64 %inc18, %div25
  br i1 %cmp, label %for.cond3.preheader, label %if.end.loopexit

for.body28:                                       ; preds = %for.body28.preheader, %for.body28
  %storemerge11 = phi i64 [ %inc38, %for.body28 ], [ 0, %for.body28.preheader ]
  %arrayidx31 = getelementptr inbounds i64* %B, i64 %storemerge11
  %tmp32 = load i64* %arrayidx31, align 8
  %arrayidx35 = getelementptr inbounds i64* %A, i64 %storemerge11
  store i64 %tmp32, i64* %arrayidx35, align 8
  %inc38 = add nsw i64 %storemerge11, 1
  %cmp26 = icmp slt i64 %inc38, %div25
  br i1 %cmp26, label %for.body28, label %if.end.loopexit13

if.end.loopexit:                                  ; preds = %for.inc16
  br label %if.end

if.end.loopexit13:                                ; preds = %for.body28
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit13, %if.end.loopexit, %for.cond22.preheader, %for.cond.preheader
  ret void
}

declare i32 @get_global_id(i32)

declare i32 @get_local_id(i32)
