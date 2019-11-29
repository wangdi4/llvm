; REQUIRES: assert
; This test checks if the whole program seen was achieved
; since add is internal and sub is not used.

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -shared -plugin %llvmshlibdir/LLVMgold%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=-debug-only=whole-program-analysis \
; RUN:    -plugin-opt=-whole-program-assume-executable %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; CHECK:   WHOLE-PROGRAM-ANALYSIS: SIMPLE ANALYSIS
; CHECK:   VISIBLE OUTSIDE LTO: 0
; CHECK-NEXT:   WHOLE PROGRAM DETECTED
; CHECK-NEXT:   WHOLE PROGRAM SAFE is determined

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 @add(i32 %a) {
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
  ret i32 %call1
}
