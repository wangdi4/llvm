; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleE.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %cmp7 = icmp sgt i64 %n, 0
  br i1 %cmp7, label %for.body.preheader, label %for.end39

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc36
  %storemerge8 = phi i64 [ %inc38, %for.inc36 ], [ 0, %for.body.preheader ]
  %rem1 = and i64 %storemerge8, 1
  %tobool = icmp eq i64 %rem1, 0
  %arrayidx26 = getelementptr inbounds i64* %A, i64 %storemerge8
  br i1 %tobool, label %for.cond19.preheader, label %for.cond5.preheader

for.cond19.preheader:                             ; preds = %for.body
  %arrayidx31 = getelementptr inbounds i64* %B, i64 %storemerge8
  br label %for.body23

for.cond5.preheader:                              ; preds = %for.body
  %call = tail call i32 @get_local_id(i32 0) nounwind
  %conv = zext i32 %call to i64
  %tmp14 = load i64* %arrayidx26, align 8
  %add11 = add i64 %tmp14, %n
  %add15 = add i64 %add11, %conv
  store i64 %add15, i64* %arrayidx26, align 8
  %call.1 = tail call i32 @get_local_id(i32 0) nounwind
  %conv.1 = zext i32 %call.1 to i64
  %tmp14.1 = load i64* %arrayidx26, align 8
  %add11.1 = add i64 %tmp14.1, %n
  %add15.1 = add i64 %add11.1, %conv.1
  store i64 %add15.1, i64* %arrayidx26, align 8
  br label %for.inc36

for.body23:                                       ; preds = %for.cond19.preheader, %for.body23
  %storemerge26 = phi i64 [ 0, %for.cond19.preheader ], [ %add34, %for.body23 ]
  %tmp27 = load i64* %arrayidx26, align 8
  %add28 = add nsw i64 %tmp27, 1
  store i64 %add28, i64* %arrayidx31, align 8
  %add34 = add nsw i64 %storemerge26, 2
  %cmp21 = icmp slt i64 %add34, 600
  br i1 %cmp21, label %for.body23, label %for.inc36.loopexit

for.inc36.loopexit:                               ; preds = %for.body23
  br label %for.inc36

for.inc36:                                        ; preds = %for.inc36.loopexit, %for.cond5.preheader
  %inc38 = add nsw i64 %storemerge8, 1
  %exitcond = icmp eq i64 %inc38, %n
  br i1 %exitcond, label %for.end39.loopexit, label %for.body

for.end39.loopexit:                               ; preds = %for.inc36
  br label %for.end39

for.end39:                                        ; preds = %for.end39.loopexit, %entry
  ret void
}

declare i32 @get_local_id(i32)
