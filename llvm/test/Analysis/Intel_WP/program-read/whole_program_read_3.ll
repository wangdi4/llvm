; REQUIRES: assert
; This test checks if the __dso_handle was treated as linker added symbol
; during whole program read.

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -shared -plugin %llvmshlibdir/LLVMgold%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=-debug-only=whole-program-analysis \
; RUN:    -plugin-opt=-whole-program-read-trace %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ
; CHECK: SYMBOL NAME: __dso_handle
; CHECK:  LINKER ADDED SYMBOL


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@__dso_handle = external hidden global i8

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
