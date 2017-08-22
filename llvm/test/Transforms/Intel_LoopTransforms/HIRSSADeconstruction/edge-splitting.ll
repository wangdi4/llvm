; RUN: opt < %s -hir-ssa-deconstruction -S | FileCheck %s

; Check that we split edges (if.then.128 -> if.end.150) and (if.else.139 -> if.end.150) when deconstructing the SCC consisting of %0 and %2.

; CHECK: if.then.128.if.end.150_crit_edge:
; CHECK-NEXT: hir.de.ssa.copy0.in
; CHECK-NEXT: br label %if.end.150

; CHECK: if.else.139.if.end.150_crit_edge:
; CHECK-NEXT: hir.de.ssa.copy0.in
; CHECK-NEXT: br label %if.end.150

; ModuleID = 'swap.ll'
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

define i32 @Swap() {
entry:
  %swap_list = alloca [32 x i32], align 4
  br label %while.body.126

while.body.126:                                   ; preds = %entry, %if.end.150
  %0 = phi i32 [ 0, %entry ], [ %2, %if.end.150 ]
  %storemerge.149 = phi i32 [ 0, %entry ], [ %add152, %if.end.150 ]
  %next_capture.248 = phi i32 [ 0, %entry ], [ %sub130, %if.end.150 ]
  %cmp = icmp slt i32 %storemerge.149, 0
  %sub130 = add nsw i32 %next_capture.248, -1
  %arrayidx131 = getelementptr inbounds [32 x i32], [32 x i32]* %swap_list, i32 0, i32 %sub130
  %1 = load i32, i32* %arrayidx131, align 4
  br i1 %cmp, label %if.then.128, label %if.else.139

if.then.128:                                      ; preds = %while.body.126
  %cmp132 = icmp sgt i32 %0, %1
  br i1 %cmp132, label %if.end.150, label %if.then.134

if.then.134:                                      ; preds = %if.then.128
  store i32 %0, i32* %arrayidx131, align 4
  br label %if.end.150

if.else.139:                                      ; preds = %while.body.126
  %cmp143 = icmp slt i32 %0, %1
  br i1 %cmp143, label %if.end.150, label %if.then.145

if.then.145:                                      ; preds = %if.else.139
  store i32 %0, i32* %arrayidx131, align 4
  br label %if.end.150

if.end.150:                                       ; preds = %if.then.145, %if.else.139, %if.then.134, %if.then.128
  %2 = phi i32 [ %0, %if.then.145 ], [ %1, %if.else.139 ], [ %0, %if.then.134 ], [ %1, %if.then.128 ]
  %add152 = add nsw i32 1, %storemerge.149
  %tobool125 = icmp eq i32 %sub130, 1
  br i1 %tobool125, label %while.end.153.loopexit, label %while.body.126

while.end.153.loopexit:                           ; preds = %if.end.150
  br label %while.end.153

while.end.153:                                    ; preds = %while.end.153.loopexit, %while.cond.preheader, %while.end
  ret i32 %0
}

