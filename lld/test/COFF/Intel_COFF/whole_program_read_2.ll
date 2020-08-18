; REQUIRES: asserts
; This test checks that whole program wasn't achieved because the
; definition for @sub is missing in the IR. Whole program read
; can still be achieved.

; RUN: llvm-as -o %t_wp2.bc %s
; RUN: llc %p/Inputs/whole_program_read_2_sub.ll -o %t_foo.obj \
; RUN:          -filetype=obj
; RUN: lld-link /out:%t_wp2.exe /entry:main %t_wp2.bc %t_foo.obj /subsystem:console  \
; RUN:     /mllvm:-debug-only=whole-program-analysis \
; RUN:     2>&1 | FileCheck %s

; CHECK:   LIBFUNCS NOT FOUND: 1
; CHECK:       sub
; CHECK:   WHOLE PROGRAM RESULT:
; CHECK:    MAIN DEFINITION:  DETECTED
; CHECK:    LINKING AN EXECUTABLE:  DETECTED
; CHECK:    WHOLE PROGRAM READ:  DETECTED
; CHECK:    WHOLE PROGRAM SEEN:  NOT DETECTED
; CHECK:    WHOLE PROGRAM SAFE:  NOT DETECTED

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
