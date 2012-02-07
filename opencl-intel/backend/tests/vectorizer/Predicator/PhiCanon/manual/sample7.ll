; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample7.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.cond8.preheader, %for.end22.loopexit
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %cmp7 = icmp sgt i64 %n, 0
  br i1 %cmp7, label %for.body.preheader, label %for.end22

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.cond8.preheader:                              ; preds = %for.body
  br i1 %cmp7, label %for.body12.preheader, label %for.end22

for.body12.preheader:                             ; preds = %for.cond8.preheader
  br label %for.body12

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge19 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %tmp2338 = phi i64 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %call = tail call i32 @get_local_id(i32 0) nounwind
  %idxprom = zext i32 %call to i64
  %arrayidx = getelementptr inbounds i64* %A, i64 %idxprom
  %tmp3 = load i64* %arrayidx, align 8
  %add = add nsw i64 %tmp3, %tmp2338
  %inc = add nsw i64 %storemerge19, 1
  %exitcond11 = icmp eq i64 %inc, %n
  br i1 %exitcond11, label %for.cond8.preheader, label %for.body

for.body12:                                       ; preds = %for.body12.preheader, %for.body12
  %storemerge6 = phi i64 [ %inc21, %for.body12 ], [ 0, %for.body12.preheader ]
  %tmp2325 = phi i64 [ %add18, %for.body12 ], [ %add, %for.body12.preheader ]
  %arrayidx15 = getelementptr inbounds i64* %B, i64 %storemerge6
  %tmp16 = load i64* %arrayidx15, align 8
  %add18 = add nsw i64 %tmp16, %tmp2325
  %inc21 = add nsw i64 %storemerge6, 1
  %exitcond = icmp eq i64 %inc21, %n
  br i1 %exitcond, label %for.end22.loopexit, label %for.body12

for.end22.loopexit:                               ; preds = %for.body12
  br label %for.end22

for.end22:                                        ; preds = %for.end22.loopexit, %entry, %for.cond8.preheader
  %tmp232.lcssa = phi i64 [ %add, %for.cond8.preheader ], [ 0, %entry ], [ %add18, %for.end22.loopexit ]
  store i64 %tmp232.lcssa, i64* %B, align 8
  ret void
}

declare i32 @get_local_id(i32)
