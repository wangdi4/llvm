; This test checks that the assert in whole program analysis triggers
; since whole program wasn't achieved. This is the same test as
; whole_program_read_2.ll.

; RUN: llvm-as -o %T/wp2.bc %s
; RUN: llc %p/Inputs/whole_program_read_2_sub.ll -o %T/foo.obj \
; RUN:          -filetype=obj
; RUN: not lld-link /out:%T/wp2.exe /entry:main %T/wp2.bc %T/foo.obj /subsystem:console  \
; RUN:     /mllvm:-whole-program-assert \
; RUN:     2>&1 | FileCheck %s

; CHECK: Whole-Program-Analysis: Did not detect whole program

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
