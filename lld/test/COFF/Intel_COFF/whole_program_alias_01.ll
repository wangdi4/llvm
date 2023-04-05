; REQUIRES: asserts
; This test checks that whole program was detected since the alias @aliassub
; was resolved. This test case checks that the whole program analysis produces
; the correct result when it is invoked from LLD (COFF driver).

; RUN: llvm-as -o %t_wp1alias.bc %s
; RUN: lld-link /out:%t_wp1alias.exe /entry:main %t_wp1alias.bc /subsystem:console  \
; RUN:     /mllvm:-debug-only=whole-program-analysis \
; RUN:     /opt:ltonewpassmanager \
; RUN:     2>&1 | FileCheck %s

; CHECK:   UNRESOLVED CALLSITES: 0
; CHECK:   WHOLE PROGRAM RESULT:
; CHECK:   MAIN DEFINITION:  DETECTED
; CHECK:   LINKING AN EXECUTABLE:  DETECTED
; CHECK:   WHOLE PROGRAM READ:  DETECTED
; CHECK:   WHOLE PROGRAM SEEN:  DETECTED
; CHECK:   WHOLE PROGRAM SAFE:  DETECTED

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
