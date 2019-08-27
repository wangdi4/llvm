; This test checks that whole program read wasn't achieved because the
; definition for @sub is missing in the IR.
; NOTE: lld will throw a exit error since the defintion of sub
; is not present during link time.

; RUN: llvm-as -o %T/wpt3.bc %s
; RUN: lld-link /out:%T/wpt3.exe /entry:main %T/wpt3.bc /subsystem:console  \
; RUN:     /mllvm:-whole-program-read-trace /force:unresolved \
; RUN:     2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ
; CHECK: SYMBOL NAME: main
; CHECK:  RESULT: MAIN | RESOLVED BY LINKER

; CHECK: SYMBOLS RESOLVED BY LINKER: 1
; CHECK: SYMBOLS NOT RESOLVED BY LINKER: 1
; CHECK: WHOLE PROGRAM READ NOT ACHIEVED

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
