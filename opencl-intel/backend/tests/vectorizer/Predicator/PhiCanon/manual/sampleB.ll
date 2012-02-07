; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleB.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.cond.preheader, %if.end42.loopexit
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %tmp1 = load i64* %B, align 8
  %tobool = icmp eq i64 %tmp1, 0
  br i1 %tobool, label %for.cond17.preheader, label %for.cond.preheader

for.cond17.preheader:                             ; preds = %entry
  %add = add nsw i64 %n, 4
  br label %for.cond17

for.cond.preheader.loopexit:                      ; preds = %for.body23
  br label %for.cond.preheader

for.cond.preheader:                               ; preds = %for.cond.preheader.loopexit, %entry
  %cmp5 = icmp sgt i64 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %if.end42

for.body.preheader:                               ; preds = %for.cond.preheader
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx8 = getelementptr inbounds i64* %B, i64 %indvars.iv
  %tmp9 = load i64* %arrayidx8, align 8
  %arrayidx13 = getelementptr inbounds i64* %A, i64 %indvars.iv
  store i64 %tmp9, i64* %arrayidx13, align 8
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %if.end42.loopexit, label %for.body

for.cond17:                                       ; preds = %for.cond17.preheader, %if.end
  %indvars.iv10 = phi i64 [ 0, %for.cond17.preheader ], [ %indvars.iv.next11, %if.end ]
  %cmp21 = icmp slt i64 %indvars.iv10, %add
  br i1 %cmp21, label %for.body23, label %if.end42.loopexit14

for.body23:                                       ; preds = %for.cond17
  %0 = trunc i64 %indvars.iv10 to i32
  %cmp25 = icmp eq i32 %0, 80
  br i1 %cmp25, label %for.cond.preheader.loopexit, label %if.end

if.end:                                           ; preds = %for.body23
  %1 = shl i64 %indvars.iv10, 1
  %arrayidx31 = getelementptr inbounds i64* %B, i64 %1
  %tmp32 = load i64* %arrayidx31, align 8
  %2 = add nsw i64 %indvars.iv10, 4
  %arrayidx37 = getelementptr inbounds i64* %A, i64 %2
  store i64 %tmp32, i64* %arrayidx37, align 8
  %indvars.iv.next11 = add i64 %indvars.iv10, 1
  br label %for.cond17

if.end42.loopexit:                                ; preds = %for.body
  br label %if.end42

if.end42.loopexit14:                              ; preds = %for.cond17
  br label %if.end42

if.end42:                                         ; preds = %if.end42.loopexit14, %if.end42.loopexit, %for.cond.preheader
  ret void
}
