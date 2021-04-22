; This test checks that the option /ASSEMBLYMODULE: causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /ASSEMBLYMODULE: flag is ignored
; RUN: not lld-link /subsystem:console /ASSEMBLYMODULE:module.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKAM -allow-empty %s
; CHECKAM:     warning: ignoring unknown argument '/assemblymodule:'
; CHECKAM-NOT: could not open '/ASSEMBLYMODULE:'

; Check that the /assemblymodule: flag is ignored
; RUN: not lld-link /subsystem:console /assemblymodule:module.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKAM2 -allow-empty %s
; CHECKAM2:     warning: ignoring unknown argument '/assemblymodule:'
; CHECKAM2-NOT: could not open '/assemblymodule:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}