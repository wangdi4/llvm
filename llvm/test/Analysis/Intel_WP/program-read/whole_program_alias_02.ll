; REQUIRES: asserts
; This test checks that whole program wasn't achieved due to the following
; issues:
;
;  1) @sub2 is defined outside the module (Inputs/whole_program_alias_sub2.ll)
;     which produces a missing libfunc
;
;  2) @sub won't be internalized because it is used in @sub2
;
;  3) @aliassub aliases @sub, which wasn't internalized (unresolved alias)
;
; The goal of this test case is to check that the whole program analysis
; produces the correct result when running through the linker.
;
; This is the same test as lld/test/ELF/Intel_ELF/whole_program_alias_04.ll,
; but rather than use lld linker, it uses the gold linker.

; RUN: llvm-as %s -o %t.bc
; RUN: llc %p/Inputs/whole_program_alias_sub2.ll -o %t7.o \
; RUN:          -filetype=obj
; RUN: %gold -shared -plugin %llvmshlibdir/icx-lto%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=new-pass-manager \
; RUN:    -plugin-opt=-debug-only=whole-program-analysis \
; RUN:    -plugin-opt=-whole-program-assume-executable %t.bc %t7.o -o %t \
; RUN:    2>&1 | FileCheck %s

; CHECK:   ALIASES UNRESOLVED: 1
; CHECK:       aliassub
; CHECK:   LIBFUNCS NOT FOUND: 1
; CHECK:       sub2
; CHECK:   VISIBLE OUTSIDE LTO: 1
; CHECK:       sub
; CHECK:   WHOLE PROGRAM RESULT:
; CHECK:    MAIN DEFINITION:  DETECTED
; CHECK:    LINKING AN EXECUTABLE:  DETECTED
; CHECK:    WHOLE PROGRAM READ:  DETECTED
; CHECK:    WHOLE PROGRAM SEEN:  NOT DETECTED
; CHECK:    WHOLE PROGRAM SAFE:  NOT DETECTED

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aliassub = unnamed_addr alias i32 (i32), ptr @sub

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

declare i32 @sub2(i32 %a)

define external i32 @sub(i32 %a) {
entry:
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define i32 @main(i32 %argc, ptr nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @aliassub(i32 %call1)
  %call3 = call i32 @sub2(i32 %call2)
  ret i32 %call2
}
