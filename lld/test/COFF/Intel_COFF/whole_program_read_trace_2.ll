; REQUIRES: asserts
; This test checks that whole program read was achieved even if the
; definition of @sub is in another compilation unit.

; RUN: llvm-as -o %t_wpt2.bc %s
; RUN: llc %p/Inputs/whole_program_read_2_sub.ll -o %t_wpt2_sub.obj \
; RUN:          -filetype=obj
; RUN: lld-link /out:%t_wpt2.exe /entry:main %t_wpt2.bc %t_wpt2_sub.obj /subsystem:console  \
; RUN:     /mllvm:-debug-only=whole-program-analysis \
; RUN:     /mllvm:-whole-program-read-trace \
; RUN:     /opt:ltonewpassmanager \
; RUN:     2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ TRACE
; CHECK: SYMBOL NAME: main
; CHECK:  RESULT: MAIN | RESOLVED BY LINKER

; CHECK: SYMBOL NAME: sub
; CHECK:  RESULT: RESOLVED BY LINKER

; CHECK: SYMBOLS RESOLVED BY LINKER: 2
; CHECK: SYMBOLS NOT RESOLVED BY LINKER: 0
; CHECK: WHOLE PROGRAM READ:  DETECTED
; CHECK: WHOLE PROGRAM SAFE:  NOT DETECTED

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
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
