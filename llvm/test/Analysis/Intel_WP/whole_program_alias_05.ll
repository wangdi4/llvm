; REQUIRES: asserts
; This test checks that @aliassub isn't in the unresolved aliases list since
; there is no use for it in the IR.

; RUN: opt < %s -disable-output -passes='require<wholeprogram>' -debug-only=whole-program-analysis 2>&1  | FileCheck %s

; CHECK:   ALIASES UNRESOLVED: 0
; CHECK:   LIBFUNCS NOT FOUND: 0
; CHECK:   VISIBLE OUTSIDE LTO: 0
; CHECK:   WHOLE PROGRAM RESULT:
; CHECK:    MAIN DEFINITION:  DETECTED

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@aliassub = unnamed_addr alias i32 (i32), i32 (i32)* @sub

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

define external i32 @sub(i32 %a) {
entry:
  %sub = add nsw i32 %a, -2
  ret i32 %sub
}

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  ret i32 %call1
}