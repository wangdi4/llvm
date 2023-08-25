; RUN: opt < %s -disable-output -passes='require<wholeprogram>' -debug-only=whole-program-analysis 2>&1  | FileCheck %s

; Check that atexit is not an unresolved function on Linux.

; CHECK: WHOLE-PROGRAM-ANALYSIS
; CHECK: LIBFUNCS NOT FOUND: 0

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @exitfunc() {
entry:
  ret void
}

define i32 @main() {
entry:
  %call = tail call i32 @atexit(ptr noundef nonnull @exitfunc)
  ret i32 0
}

declare i32 @atexit(ptr noundef)
