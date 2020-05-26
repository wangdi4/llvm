; REQUIRES: asserts
; This test checks if the whole program seen wasn't achieved
; since all symbols weren't internalized.

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -shared -plugin %llvmshlibdir/LLVMgold%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=-debug-only=whole-program-analysis \
; RUN:    -plugin-opt=-whole-program-assume-executable %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; CHECK:   WHOLE-PROGRAM-ANALYSIS: SIMPLE ANALYSIS
; CHECK:   VISIBLE OUTSIDE LTO: 2
; CHECK-NEXT:     add
; CHECK-NEXT:     sub
; CHECK: WHOLE PROGRAM SEEN:  NOT DETECTED

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
