; REQUIRES: asserts
; This test checks that whole program was achieved.

; RUN: opt %s -o %t.bc
; RUN: ld.lld -e main --lto-O2 \
; RUN:     -plugin-opt=new-pass-manager  \
; RUN:     -mllvm -debug-only=whole-program-analysis \
; RUN:     -mllvm -whole-program-read-trace %t.bc -o %t \
; RUN:     2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: WHOLE PROGRAM READ TRACE
; CHECK: SYMBOL NAME: main
; CHECK:  RESULT: MAIN | RESOLVED BY LINKER

; CHECK: SYMBOL NAME: mycommon
; CHECK: RESULT: RESOLVED BY LINKER

; CHECK: SYMBOLS RESOLVED BY LINKER: 2
; CHECK: SYMBOLS NOT RESOLVED BY LINKER: 0
; CHECK:  WHOLE PROGRAM RESULT:
; CHECK:    MAIN DEFINITION:  DETECTED
; CHECK:    LINKING AN EXECUTABLE:  DETECTED
; CHECK:    WHOLE PROGRAM READ:  DETECTED
; CHECK:    WHOLE PROGRAM SEEN:  DETECTED
; CHECK:    WHOLE PROGRAM SAFE:  DETECTED


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@mycommon = common dso_local global i32 zeroinitializer, align 4

; Function Attrs: norecurse nounwind readonly uwtable
define dso_local i32 @main() #0 {
  %1 = load i32, i32* @mycommon, align 4
  ret i32 %1
}
