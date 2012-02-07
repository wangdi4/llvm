; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample6.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %call = tail call i32 @get_local_id(i32 0) nounwind
  %cmp = icmp ugt i32 %call, 100
  br i1 %cmp, label %for.body.preheader, label %if.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge1 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64* %B, i64 %storemerge1
  %tmp4 = load i64* %arrayidx, align 8
  %mul = mul i64 %tmp4, %n
  %arrayidx8 = getelementptr inbounds i64* %A, i64 %storemerge1
  %tmp9 = load i64* %arrayidx8, align 8
  %add = add nsw i64 %tmp9, %mul
  store i64 %add, i64* %arrayidx8, align 8
  %inc = add nsw i64 %storemerge1, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %if.end.loopexit, label %for.body

if.end.loopexit:                                  ; preds = %for.body
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit, %entry
  ret void
}

declare i32 @get_local_id(i32)
