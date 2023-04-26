; REQUIRES: asserts
; This test checks that the alias @aliassub was resolved since it is
; aliasing @sub, which was internalized.

; NOTE: This test case runs in opt rather than lld because we don't want
; to run the internalization in LTO.

; RUN: opt < %s -disable-output -passes='require<wholeprogram>' -debug-only=whole-program-analysis 2>&1  | FileCheck %s

; CHECK:   ALIASES UNRESOLVED: 0
; CHECK:   LIBFUNCS NOT FOUND: 0
; CHECK:   VISIBLE OUTSIDE LTO: 0
; CHECK:   WHOLE PROGRAM RESULT:
; CHECK:    MAIN DEFINITION:  DETECTED

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

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