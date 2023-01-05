; REQUIRES: asserts
; This test checks that whole program was detected since the alias @aliassub
; was resolved. The goal of this test case is to check that the whole program
; analysis produces the correct output when it runs through LLD (ELF driver).

; RUN: opt %s -o %t_wp1alias.bc
; RUN: ld.lld -e main --lto-O2 \
; RUN:     -plugin-opt=new-pass-manager  \
; RUN:     -mllvm -debug-only=whole-program-analysis \
; RUN:     -mllvm -whole-program-assume-executable %t_wp1alias.bc -o %t_wp1alias \
; RUN:     2>&1 | FileCheck %s

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

@aliassub = unnamed_addr alias i32 (i32), i32 (i32)* @sub

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

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @aliassub(i32 %call1)
  ret i32 %call2
}
