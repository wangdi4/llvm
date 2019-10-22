; REQUIRES: assert
; This test checks that whole program was achieved.

; RUN: opt %s -o %t.bc
; RUN: ld.lld -e main --lto-O2 \
; RUN:     -mllvm -debug-only=whole-program-analysis \
; RUN:     -mllvm -whole-program-assume-executable %t.bc -o %t \
; RUN:     2>&1 | FileCheck %s

; CHECK:   Main definition seen
; CHECK:   UNRESOLVED CALLSITES: 0
; CHECK:   WHOLE PROGRAM DETECTED
; CHECK:   WHOLE PROGRAM SAFE is determined

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define internal i32 @sub(i32 %a) {
entry:
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
