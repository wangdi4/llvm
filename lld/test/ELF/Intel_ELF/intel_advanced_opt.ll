; This test case checks that -plugin-opt=fintel-advanced-optim
; is in the driver.

; REQUIRES: x86

; RUN: opt %s -o %t.o
; RUN: ld.lld --plugin-opt=fintel-advanced-optim --plugin-opt=new-pass-manager -o /dev/null %t.o 2>&1 | FileCheck %s

; CHECK-NOT: error: --plugin-opt: ld.lld: Unknown command line argument 'fintel-advanced-optim'

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @main() {
  ret void
}
