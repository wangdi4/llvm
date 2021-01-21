; This test checks that the option /RELEASE causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /RELEASE flag is ignored
; RUN: not lld-link /subsystem:console /RELEASE %t.obj 2>&1 | FileCheck -check-prefix=CHECKREL -allow-empty %s
; CHECKREL:     warning: ignoring unknown argument '/release'
; CHECKREL-NOT: could not open '/RELEASE'

; Check that the /release flag is ignored
; RUN: not lld-link /subsystem:console /release %t.obj 2>&1 | FileCheck -check-prefix=CHECKREL2 -allow-empty %s
; CHECKREL2:     warning: ignoring unknown argument '/release'
; CHECKREL2-NOT: could not open '/release'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}