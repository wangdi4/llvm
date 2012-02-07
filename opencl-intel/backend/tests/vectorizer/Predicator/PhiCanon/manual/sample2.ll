; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample2.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* nocapture %B) nounwind {
entry:
  %cmp2 = icmp sgt i64 %n, 0
  br i1 %cmp2, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %storemerge4 = phi i64 [ %inc, %for.body ], [ 0, %for.body.preheader ]
  %tmp713 = phi i64 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64* %A, i64 %storemerge4
  %tmp4 = load i64* %arrayidx, align 8
  %add = add nsw i64 %tmp4, %tmp713
  %inc = add nsw i64 %storemerge4, 1
  %exitcond = icmp eq i64 %inc, %n
  br i1 %exitcond, label %for.end.loopexit, label %for.body

for.end.loopexit:                                 ; preds = %for.body
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %tmp71.lcssa = phi i64 [ 0, %entry ], [ %add, %for.end.loopexit ]
  store i64 %tmp71.lcssa, i64* %A, align 8
  ret void
}
