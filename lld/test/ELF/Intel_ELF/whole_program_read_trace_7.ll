; REQUIRES: asserts
; This test checks that the trace for dynamic exported symbol is printed
; correctly. The symbol "add" must be marked as "DYNAMIC EXPORT SYMBOL".

; RUN: opt %s -o %t.bc
; RUN: ld.lld -e main --lto-O2 \
; RUN:     -plugin-opt=new-pass-manager  \
; RUN:     -mllvm -debug-only=whole-program-analysis \
; RUN:     -mllvm -whole-program-read-trace %t.bc -o %t \
; RUN:     --export-dynamic-symbol=add \
; RUN:     2>&1 | FileCheck %s

; CHECK: SYMBOL NAME: main
; CHECK:  RESULT: MAIN | RESOLVED BY LINKER

; CHECK: SYMBOL NAME: add
; CHECK:  RESULT: DYNAMIC EXPORT SYMBOL | RESOLVED BY LINKER

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  ret i32 %call1
}
