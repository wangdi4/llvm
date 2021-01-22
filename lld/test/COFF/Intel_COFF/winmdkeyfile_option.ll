; This test checks that the option /WINMDKEYFILE causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /WINMDKEYFILE: flag is ignored
; RUN: not lld-link /subsystem:console /WINMDKEYFILE:file.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKWKF -allow-empty %s
; CHECKWKF:     warning: ignoring unknown argument '/winmdkeyfile:'
; CHECKWKF-NOT: could not open '/WINMDKEYFILE:'

; Check that the /winmdkeyfile: flag is ignored
; RUN: not lld-link /subsystem:console /winmdkeyfile:file.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKWKF2 -allow-empty %s
; CHECKWKF2:     warning: ignoring unknown argument '/winmdkeyfile:'
; CHECKWKF2-NOT: could not open '/winmdkeyfile:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}