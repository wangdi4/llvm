; RUN: opt < %s -hir-ssa-deconstruction -hir-temp-cleanup -hir-opt-predicate -print-after=hir-opt-predicate -analyze -enable-new-pm=0 -hir-dd-analysis -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,print<hir>,print<hir-dd-analysis>" -hir-dd-analysis-verify=Region 2>&1 | FileCheck %s

; Verify that all edges are omitted between (%A)[i1] and (%B)[i1] and %val.012's
; def and use after predicate optimization as they lie on mutually exclusive
; paths in the region.

; Incoming HIR-
; + DO i1 = 0, 39, 1   <DO_LOOP>
; |   if (%t > 5)
; |   {
; |      (%A)[i1] = %val.012;
; |   }
; |   else
; |   {
; |      %val.012 = (%B)[i1];
; |   }
; + END LOOP

; CHECK: if (%t > 5)
; CHECK: {
; CHECK:    + DO i1 = 0, 39, 1   <DO_LOOP>
; CHECK:    |   (%A)[i1] = %val.012;
; CHECK:    + END LOOP
; CHECK: }
; CHECK: else
; CHECK: {
; CHECK:    + DO i1 = 0, 39, 1   <DO_LOOP>
; CHECK:    |   %val.012 = (%B)[i1];
; CHECK:    + END LOOP
; CHECK: }

; CHECK: DD graph for function foo
; CHECK-NOT: -->


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @foo(i32* nocapture %A, i32* nocapture readonly %B, i32 %t) {
entry:
  %cmp1 = icmp sgt i32 %t, 5
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.inc ]
  %val.012 = phi i32 [ undef, %entry ], [ %val.1, %for.inc ]
  br i1 %cmp1, label %if.then, label %if.else

if.then:                                          ; preds = %for.body
  %ptridx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  store i32 %val.012, i32* %ptridx, align 4
  br label %for.inc

if.else:                                          ; preds = %for.body
  %ptridx3 = getelementptr inbounds i32, i32* %B, i64 %indvars.iv
  %0 = load i32, i32* %ptridx3, align 4
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %val.1 = phi i32 [ %val.012, %if.then ], [ %0, %if.else ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 40
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  %val.1.lcssa = phi i32 [ %val.1, %for.inc ]
  ret i32 %val.1.lcssa
}

