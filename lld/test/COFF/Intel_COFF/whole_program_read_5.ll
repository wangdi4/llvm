; REQUIRES: asserts
; This test checks that whole program wasn't achieved since
; there is no main.

; RUN: llvm-as -o %t_wp5.bc %s
; RUN: lld-link /out:%t_wp5.exe /entry:foo %t_wp5.bc /subsystem:console  \
; RUN:     /mllvm:-debug-only=whole-program-analysis \
; RUN:     /opt:ltonewpassmanager \
; RUN:     2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: SIMPLE ANALYSIS

; CHECK:  UNRESOLVED CALLSITES: 0
; CHECK:  ALIASES UNRESOLVED: 0
; CHECK:  LIBFUNCS NOT FOUND: 0
; CHECK:  VISIBLE OUTSIDE LTO: 0
; CHECK:  WHOLE PROGRAM RESULT:
; CHECK:    MAIN DEFINITION:  NOT DETECTED
; CHECK:    LINKING AN EXECUTABLE:  DETECTED
; CHECK:    WHOLE PROGRAM READ:  NOT DETECTED
; CHECK:    WHOLE PROGRAM SEEN:  NOT DETECTED
; CHECK:    WHOLE PROGRAM SAFE:  NOT DETECTED

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

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

define i32 @foo(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
