; This test checks that whole program assume won't internalize
; @sub since it doesn't have IR but it should internalize @add,
; while using the new pass manager. The new pass manager won't
; REQUIRES: assert
; print IR for an analysis pass, therefore we are going to print
; the whole program trace and check that @add is visible externally
; first. Then, @add shouldn't be marked as visible externally
; because it was internalized. The test also checks that the
; internalization pass runs after the whole program analysis and
; before the inline report setup pass.

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -shared -plugin %llvmshlibdir/LLVMgold%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=-debug-only=whole-program-analysis \
; RUN:    -plugin-opt=-whole-program-assume \
; RUN:    -plugin-opt=new-pass-manager \
; RUN:    -plugin-opt=debug-pass-manager \
; RUN:    %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; Check that whole program analysis runs
; CHECK: Running analysis: WholeProgramAnalysis

; Check that main, add and sub aren't internal
; CHECK: VISIBLE OUTSIDE LTO: 2
; CHECK: add
; CHECK: sub

; Check that whole-program-assume is enabled
; CHECK: whole-program-assume is enabled

; Check that the internalization pass runs
; CHECK: Running pass: InternalizePass

; Check that add is not visible externally
; CHECK: VISIBLE OUTSIDE LTO: 1
; CHECK-NEXT: sub

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

declare i32 @sub(i32 %a)

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
