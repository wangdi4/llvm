; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-details -hir-details-constants -hir-framework -hir-framework-debug=parser | FileCheck %s
; RUN: opt < %s -passes=hir-ssa-deconstruction | opt -passes="print<hir-framework>" -hir-details -hir-details-constants -hir-framework-debug=parser 2>&1 | FileCheck %s

; Verify that %tmp which is trunc.i64.i8(256) is simplified to 'i8 0' during parsing.
; Casts on constants are not expected in HIR so they can result in assertions.

; CHECK: + DO i64 i1 = 0, 99, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   if (0 == 0)
; CHECK: |   <RVAL-REG> i8 0 {sb:1}
; CHECK: |   <RVAL-REG> i8 0 {sb:1}
; CHECK: |   {
; CHECK: |      goto earlyexit;
; CHECK: |   }
; CHECK: + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define hidden void @foo() {
entry:
  %tmp = trunc i64 256 to i8
  %tmp1 = icmp ne i8 %tmp, 0
  br label %loop

loop:                                              ; preds = %latch, %entry
  %tmp4 = phi i64 [ %inc, %latch ], [ 0, %entry ]
  br i1 %tmp1, label %latch, label %earlyexit

earlyexit:                                              ; preds = %loop
  unreachable

latch:                                              ; preds = %loop
  %inc = add i64 %tmp4, 1
  %tmp7 = icmp slt i64 %inc, 100
  br i1 %tmp7, label %loop, label %exit

exit:                                              ; preds = %latch
  ret void
}

