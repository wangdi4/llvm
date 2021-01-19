; This test checks that the option /KEYFILE causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /KEYFILE: flag is ignored
; RUN: not lld-link /subsystem:console /KEYFILE:file.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKKF -allow-empty %s
; CHECKKF:     warning: ignoring unknown argument '/keyfile:'
; CHECKKF-NOT: could not open '/KEYFILE:'

; Check that the /keyfile: flag is ignored
; RUN: not lld-link /subsystem:console /keyfile:file.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKKF2 -allow-empty %s
; CHECKKF2:     warning: ignoring unknown argument '/keyfile:'
; CHECKKF2-NOT: could not open '/keyfile:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}