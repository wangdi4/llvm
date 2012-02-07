; RUN: llvm-as %s -o %t.bc
; RUN: opt -phicanon -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; ModuleID = 'sample5.bc'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: @func
; CHECK-NOT: %{{[a-z\.0-9]}} %{{[a-z\.0-9]}} %{{[a-z\.0-9]}}
; CHECK-NOT: phi-split-bb
; CHECK: ret

define void @func(i64 %n, i64* %A, i64* %B) nounwind {
entry:
  %div = sdiv i64 %n, 10
  %cmp7 = icmp sgt i64 %n, 9
  br i1 %cmp7, label %for.body12.lr.ph.us.preheader, label %for.end23

for.body12.lr.ph.us.preheader:                    ; preds = %entry
  br label %for.body12.lr.ph.us

for.inc20.us:                                     ; preds = %for.body12.us
  %inc22.us = add nsw i64 %storemerge9.us, 1
  %cmp.us = icmp slt i64 %inc22.us, %div
  br i1 %cmp.us, label %for.body12.lr.ph.us, label %for.end23.loopexit

for.body12.us:                                    ; preds = %for.body12.lr.ph.us, %for.body12.us
  %storemerge16.us = phi i64 [ 0, %for.body12.lr.ph.us ], [ %inc.us, %for.body12.us ]
  %tmp2435.us = phi i64 [ %add.us, %for.body12.lr.ph.us ], [ %add18.us, %for.body12.us ]
  %arrayidx15.us = getelementptr inbounds i64* %A, i64 %storemerge16.us
  %tmp16.us = load i64* %arrayidx15.us, align 8
  %add18.us = add nsw i64 %tmp16.us, %tmp2435.us
  %inc.us = add nsw i64 %storemerge16.us, 1
  %cmp11.us = icmp slt i64 %inc.us, %div
  br i1 %cmp11.us, label %for.body12.us, label %for.inc20.us

for.body12.lr.ph.us:                              ; preds = %for.body12.lr.ph.us.preheader, %for.inc20.us
  %storemerge9.us = phi i64 [ %inc22.us, %for.inc20.us ], [ 0, %for.body12.lr.ph.us.preheader ]
  %tmp2428.us = phi i64 [ %add18.us, %for.inc20.us ], [ 0, %for.body12.lr.ph.us.preheader ]
  %arrayidx.us = getelementptr inbounds i64* %B, i64 %storemerge9.us
  %tmp4.us = load i64* %arrayidx.us, align 8
  %add.us = add nsw i64 %tmp4.us, %tmp2428.us
  br label %for.body12.us

for.end23.loopexit:                               ; preds = %for.inc20.us
  br label %for.end23

for.end23:                                        ; preds = %for.end23.loopexit, %entry
  %tmp242.lcssa = phi i64 [ 0, %entry ], [ %add18.us, %for.end23.loopexit ]
  %arrayidx26 = getelementptr inbounds i64* %A, i64 5
  store i64 %tmp242.lcssa, i64* %arrayidx26, align 8
  ret void
}
