; This test checks that the option /WINMDFILE causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /WINMDFILE: flag is ignored
; RUN: not lld-link /subsystem:console /WINMDFILE:file.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKWF -allow-empty %s
; CHECKWF:     warning: ignoring unknown argument '/winmdfile:'
; CHECKWF-NOT: could not open '/WINMDFILE:'

; Check that the /winmdfile: flag is ignored
; RUN: not lld-link /subsystem:console /winmdfile:file.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKWF2 -allow-empty %s
; CHECKWF2:     warning: ignoring unknown argument '/winmdfile:'
; CHECKWF2-NOT: could not open '/winmdfile:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}