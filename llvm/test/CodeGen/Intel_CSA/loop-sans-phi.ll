; RUN: llc -mtriple=csa < %s | FileCheck %s
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

@do_lock.lock = internal global i32 0, align 4

; The check here is that we need to repeat the memory ordering operand for the
; loop, as this is a non-constant value.
; CHECK: repeat0
define void @do_lock() local_unnamed_addr {
entry:
  br label %while.cond

while.cond:                                       ; preds = %while.cond, %entry
  %0 = cmpxchg i32* @do_lock.lock, i32 0, i32 1 seq_cst seq_cst
  %1 = extractvalue { i32, i1 } %0, 1
  br i1 %1, label %while.cond, label %while.end

while.end:                                        ; preds = %while.cond
  ret void
}
