; This test checks that the option /CLRUNMANAGEDCODECHECK causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /CLRUNMANAGEDCODECHECK flag is ignored
; RUN: not lld-link /subsystem:console /CLRUNMANAGEDCODECHECK %t.obj 2>&1 | FileCheck -check-prefix=CHECKCLRM -allow-empty %s
; CHECKCLRM:     warning: ignoring unknown argument '/clrunmanagedcodecheck'
; CHECKCLRM-NOT: could not open '/CLRUNMANAGEDCODECHECK'

; Check that the /clrunmanagedcodecheck flag is ignored
; RUN: not lld-link /subsystem:console /clrunmanagedcodecheck %t.obj 2>&1 | FileCheck -check-prefix=CHECKCLRM2 -allow-empty %s
; CHECKCLRM2:     warning: ignoring unknown argument '/clrunmanagedcodecheck'
; CHECKCLRM2-NOT: could not open '/clrunmanagedcodecheck'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}