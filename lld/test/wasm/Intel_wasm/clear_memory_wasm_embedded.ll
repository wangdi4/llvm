; REQUIRES: asserts

; This test checks that the memory wasn't freed when running
; lld without early exit.

; RUN: opt %s -o %t.bc
; RUN: wasm-ld -intel-debug=mem -intel-embedded-linker -e main %t.bc -o $t.o 2>&1 | FileCheck %s

; CHECK: wasm-ld: warning: Cleaning up LLD memory
; CHECK-NOT: wasm-ld: warning: Cleaning up LLVM memory

target datalayout = "e-m:e-p:32:32-i64:64-n32:64-S128"
target triple = "wasm32-unknown-unknown"

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
