; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-dead-store-elimination" -print-before=hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output 2>&1 < %s | FileCheck %s

; Verify that unused region local alloca store (%A)[0][i1] is eliminated by DSE
; and then the load is eliminated by single load use propagation utility and
; finally the empty if and parent loop is eliminated by redundant node removal
; utility.

; This test case compfails with a verifier assertion about empty loop if
; redundant node removal is performed before load propagation.

; CHECK: Dump Before

; CHECK: + DO i1 = 0, 4, 1   <DO_LOOP>
; CHECK: |   %ld1 = (%B)[i1];
; CHECK: |   if (%ld1 != 0)
; CHECK: |   {
; CHECK: |      (%A)[0][i1] = %ld1 + 1;
; CHECK: |   }
; CHECK: + END LOOP


; CHECK: Dump After

; CHECK: BEGIN REGION { modified }
; CHECK-NEXT: END REGION


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(ptr %B) {
entry:
  %A = alloca [10 x i32], align 16
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry], [ %iv.inc, %latch]
  %iv.inc = add i64 %iv, 1
  %gepb = getelementptr inbounds i32, ptr %B, i64 %iv
  %ld1 = load i32, ptr %gepb, align 4
  %ld.cmp = icmp ne i32 %ld1, 0
  br i1 %ld.cmp, label %then, label %latch 

then:
  %add = add nsw i32 %ld1, 1
  %gepa = getelementptr inbounds [10 x i32], ptr %A, i64 0, i64 %iv
  store i32 %add, ptr %gepa, align 4
  br label %latch
  
latch:
  %cmp = icmp eq i64 %iv.inc, 5
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}


