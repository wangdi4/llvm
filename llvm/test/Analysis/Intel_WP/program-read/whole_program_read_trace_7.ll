; REQUIRES: asserts
; This test checks that symbol add was marked as "DYNAMIC EXPORT SYMBOL"
; and the trace was printed correctly.

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -m elf_x86_64 -plugin %llvmshlibdir/icx-lto%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=new-pass-manager \
; RUN:    -plugin-opt=-debug-only=whole-program-analysis \
; RUN:    -plugin-opt=-whole-program-read-trace %t.bc -o %t \
; RUN:    --export-dynamic-symbol=add \
; RUN:    2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ TRACE
; CHECK: SYMBOL NAME: add
; CHECK: RESULT: DYNAMIC EXPORT SYMBOL | RESOLVED BY LINKER

; CHECK: SYMBOL NAME: main
; CHECK: RESULT: MAIN | RESOLVED BY LINKER

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @main(i32 %argc, ptr nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  ret i32 %call1
}
