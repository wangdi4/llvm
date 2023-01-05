; REQUIRES: asserts
; This test checks that whole program wasn't achieved since
; there is no main.

; RUN: opt %s -o %t.bc
; RUN: ld.lld -e main --lto-O2 \
; RUN:     -plugin-opt=new-pass-manager  \
; RUN:     -mllvm -debug-only=whole-program-analysis \
; RUN:     -mllvm -whole-program-read-trace %t.bc -o %t \
; RUN:     2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ TRACE
; CHECK: SYMBOL NAME: foo
; CHECK:  RESULT: RESOLVED BY LINKER

; CHECK: SYMBOLS RESOLVED BY LINKER: 1
; CHECK: SYMBOLS NOT RESOLVED BY LINKER: 0
; CHECK: MAIN DEFINITION:  NOT DETECTED
; CHECK: WHOLE PROGRAM READ:  NOT DETECTED

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

define i32 @foo(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
