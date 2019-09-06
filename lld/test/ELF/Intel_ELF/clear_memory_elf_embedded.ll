; REQUIRES: asserts

; This test checks that the memory wasn't freed when running
; lld without early exit.

; RUN: opt %s -o %t.bc
; RUN: ld.lld -intel-debug=mem -intel-embedded-linker -e main %t.bc 2>&1 | FileCheck %s

; CHECK: ld.lld: warning: Cleaning up LLD memory
; CHECK-NOT: ld.lld: warning: Cleaning up LLVM memory

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  ret i32 %call1
}
