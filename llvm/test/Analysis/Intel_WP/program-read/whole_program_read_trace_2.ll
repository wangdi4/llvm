; REQUIRES: assert
; This test checks that the trace for whole program read prints
; the correct solution when the information is missing for one symbol.

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -shared -plugin %llvmshlibdir/LLVMgold%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=-debug-only=whole-program-analysis \
; RUN:    -plugin-opt=-whole-program-read-trace %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; Check that main is resolved by linker since it is not
; marked as internal
; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ
; CHECK: SYMBOL NAME: main
; CHECK: RESULT: MAIN | RESOLVED BY LINKER

; Check that sub is not a whole program valid symbol
; CHECK: SYMBOL NAME: sub
; CHECK: RESULT: NOT RESOLVED BY LINKER

; CHECK: SYMBOLS RESOLVED BY LINKER: 1
; CHECK: SYMBOLS NOT RESOLVED BY LINKER: 1
; CHECK: WHOLE PROGRAM NOT DETECTED

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

declare i32 @sub(i32 %a);

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
