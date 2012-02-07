; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sampleA.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK: phi-split-bb:                                     ; preds = %for.cond13.preheader, %if.end.loopexit14
; CHECK: phi-split-bb1:                                    ; preds = %for.cond.preheader, %if.end.loopexit
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %call = tail call i32 @get_local_id(i32 0) nounwind
  %rem = and i32 %call, 1
  %tobool = icmp eq i32 %rem, 0
  br i1 %tobool, label %for.cond13.preheader, label %for.cond.preheader

for.cond13.preheader:                             ; preds = %entry
  %add = add nsw i64 %n, 4
  %cmp176 = icmp sgt i64 %add, 0
  br i1 %cmp176, label %for.body19.preheader, label %if.end

for.body19.preheader:                             ; preds = %for.cond13.preheader
  br label %for.body19

for.cond.preheader:                               ; preds = %entry
  %cmp2 = icmp sgt i64 %n, 0
  br i1 %cmp2, label %for.body.preheader, label %if.end

for.body.preheader:                               ; preds = %for.cond.preheader
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.body
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i64* %B, i64 %indvars.iv
  %tmp5 = load i64* %arrayidx, align 8
  %arrayidx9 = getelementptr inbounds i64* %A, i64 %indvars.iv
  store i64 %tmp5, i64* %arrayidx9, align 8
  %indvars.iv.next = add i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %n
  br i1 %exitcond, label %if.end.loopexit, label %for.body

for.body19:                                       ; preds = %for.body19.preheader, %for.body19
  %indvars.iv10 = phi i64 [ %indvars.iv.next11, %for.body19 ], [ 0, %for.body19.preheader ]
  %arrayidx23 = getelementptr inbounds i64* %A, i64 %indvars.iv10
  %tmp24 = load i64* %arrayidx23, align 8
  %arrayidx28 = getelementptr inbounds i64* %B, i64 %indvars.iv10
  store i64 %tmp24, i64* %arrayidx28, align 8
  %indvars.iv.next11 = add i64 %indvars.iv10, 1
  %exitcond12 = icmp eq i64 %indvars.iv.next11, %add
  br i1 %exitcond12, label %if.end.loopexit14, label %for.body19

if.end.loopexit:                                  ; preds = %for.body
  br label %if.end

if.end.loopexit14:                                ; preds = %for.body19
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit14, %if.end.loopexit, %for.cond13.preheader, %for.cond.preheader
  %call33 = tail call i32 @get_global_id(i32 0) nounwind
  %conv34 = zext i32 %call33 to i64
  store i64 %conv34, i64* %B, align 8
  ret void
}

declare i32 @get_local_id(i32)

declare i32 @get_global_id(i32)
