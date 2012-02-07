; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample8.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %cmp8 = icmp sgt i64 %n, 0
  br i1 %cmp8, label %for.body14.preheader, label %for.end48

for.body14.preheader:                             ; preds = %entry
  br label %for.body14

for.body14:                                       ; preds = %for.body14.preheader, %for.inc45
  %storemerge7 = phi i64 [ %inc47, %for.inc45 ], [ 0, %for.body14.preheader ]
  %arrayidx17 = getelementptr inbounds i64* %A, i64 %storemerge7
  %tmp18 = load i64* %arrayidx17, align 8
  %tobool19 = icmp eq i64 %tmp18, 0
  %arrayidx39.phi.trans.insert = getelementptr inbounds i64* %B, i64 %storemerge7
  %tmp40.pre = load i64* %arrayidx39.phi.trans.insert, align 8
  br i1 %tobool19, label %if.else, label %land.lhs.true

land.lhs.true:                                    ; preds = %for.body14
  %tobool24 = icmp eq i64 %tmp40.pre, 0
  %cmp27 = icmp eq i64 %storemerge7, 9
  %or.cond = or i1 %tobool24, %cmp27
  br i1 %or.cond, label %if.else, label %for.inc45

if.else:                                          ; preds = %for.body14, %land.lhs.true
  store i64 %tmp40.pre, i64* %arrayidx17, align 8
  br label %for.inc45

for.inc45:                                        ; preds = %land.lhs.true, %if.else
  %inc47 = add nsw i64 %storemerge7, 1
  %exitcond = icmp eq i64 %inc47, %n
  br i1 %exitcond, label %for.end48.loopexit, label %for.body14

for.end48.loopexit:                               ; preds = %for.inc45
  br label %for.end48

for.end48:                                        ; preds = %for.end48.loopexit, %entry
  ret void
}
