; REQUIRES: asserts
; This test checks that whole program read was achieved with wmain.

; RUN: llvm-as -o %t_wpt4.bc %s
; RUN: lld-link /out:%t_wpt4.exe /entry:wmain %t_wpt4.bc /subsystem:console  \
; RUN:     /mllvm:-debug-only=whole-program-analysis \
; RUN:     /mllvm:-whole-program-read-trace \
; RUN:     /opt:ltonewpassmanager \
; RUN:     2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ TRACE
; CHECK: SYMBOL NAME: wmain
; CHECK:  RESULT: MAIN | RESOLVED BY LINKER

; CHECK: SYMBOLS RESOLVED BY LINKER: 1
; CHECK: SYMBOLS NOT RESOLVED BY LINKER: 0
; CHECK: MAIN DEFINITION:  DETECTED
; CHECK: WHOLE PROGRAM READ:  DETECTED

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

define i32 @wmain(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
