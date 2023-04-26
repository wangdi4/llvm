; REQUIRES: asserts
; This test checks that whole program was detected since the alias @aliassub
; was resolved. This is the same test as
; test/Analysis/Intel_WP/whole_program_alias_01.ll, but rather than check with
; opt, it checks the whole program analysis through the linker.
;
; This is the same test as lld/test/ELF/Intel_ELF/whole_program_alias_01.ll,
; but rather than use lld linker, it uses the gold linker.

; RUN: llvm-as %s -o %t.bc
; RUN: %gold -shared -plugin %llvmshlibdir/icx-lto%shlibext \
; RUN:    -plugin-opt=O3 \
; RUN:    -plugin-opt=new-pass-manager \
; RUN:    -plugin-opt=-debug-only=whole-program-analysis \
; RUN:    -plugin-opt=-whole-program-assume-executable %t.bc -o %t \
; RUN:    2>&1 | FileCheck %s

; CHECK:   UNRESOLVED CALLSITES: 0
; CHECK:   ALIASES UNRESOLVED: 0
; CHECK:   LIBFUNCS NOT FOUND: 0
; CHECK:   VISIBLE OUTSIDE LTO: 0
; CHECK:   WHOLE PROGRAM RESULT:
; CHECK:   MAIN DEFINITION:  DETECTED
; CHECK:   LINKING AN EXECUTABLE:  DETECTED
; CHECK:   WHOLE PROGRAM READ:  DETECTED
; CHECK:   WHOLE PROGRAM SEEN:  DETECTED
; CHECK:   WHOLE PROGRAM SAFE:  DETECTED

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@aliassub = unnamed_addr alias i32 (i32), ptr @sub

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

define i32 @main(i32 %argc, ptr nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @aliassub(i32 %call1)
  ret i32 %call2
}
