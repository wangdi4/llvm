; REQUIRES: assert
; This test checks that whole program read was achieved even if the
; definition of @sub is in another compilation unit.

; RUN: opt %s -o %t.bc
; RUN: llc %p/Inputs/whole_program_read_2_sub.ll -o %t2.o \
; RUN:          -filetype=obj
; RUN: ld.lld -e main --lto-O2 \
; RUN:     -mllvm -debug-only=whole-program-analysis \
; RUN:     -mllvm -whole-program-read-trace %t.bc %t2.o -o %t \
; RUN:     2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ
; CHECK: SYMBOL NAME: main
; CHECK:  RESULT: MAIN | RESOLVED BY LINKER

; CHECK: SYMBOL NAME: sub
; CHECK:  RESULT: RESOLVED BY LINKER

; CHECK: SYMBOLS RESOLVED BY LINKER: 2
; CHECK: SYMBOLS NOT RESOLVED BY LINKER: 0
; CHECK-NOT: whole program not read

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}

declare i32 @sub(i32 %a);

define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %call1 = call i32 @add(i32 %argc)
  %call2 = call i32 @sub(i32 %call1)
  ret i32 %call2
}
