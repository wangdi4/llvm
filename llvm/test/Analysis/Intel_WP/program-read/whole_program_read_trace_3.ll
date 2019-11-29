; REQUIRES: assert
; This test checks that the trace for whole program read prints
; the correct solution when all symbols aren't internal.

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -shared -plugin %llvmshlibdir/LLVMgold%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=-debug-only=whole-program-analysis \
; RUN:    -plugin-opt=-whole-program-read-trace %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; Check that main is a valid symbol
; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ
; CHECK: SYMBOL NAME: main
; CHECK: RESULT: MAIN | RESOLVED BY LINKER

; Check that add and sub are printed since they won't be internalized
; CHECK: SYMBOL NAME: add
; CHECK:  RESULT: RESOLVED BY LINKER

; CHECK: SYMBOL NAME: sub
; CHECK:  RESULT: RESOLVED BY LINKER

; CHECK: SYMBOLS RESOLVED BY LINKER: 3
; CHECK: SYMBOLS NOT RESOLVED BY LINKER: 0
; CHECK-NOT: whole program not read

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define i32 @sub(i32 %a) {
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
