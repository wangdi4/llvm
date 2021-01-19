; This test checks that the option /clrthreadattribute causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /CLRTHREADATTRIBUTE: flag is ignored
; RUN: not lld-link /subsystem:console /CLRTHREADATTRIBUTE:STA %t.obj 2>&1 | FileCheck -check-prefix=CHECKCLRT -allow-empty %s
; CHECKCLRT:     warning: ignoring unknown argument '/clrthreadattribute:'
; CHECKCLRT-NOT: could not open '/CLRTHREADATTRIBUTE:'

; Check that the /clrthreadattribute: flag is ignored
; RUN: not lld-link /subsystem:console /clrthreadattribute:sta %t.obj 2>&1 | FileCheck -check-prefix=CHECKCLRT2 -allow-empty %s
; CHECKCLRT2:     warning: ignoring unknown argument '/clrthreadattribute:'
; CHECKCLRT2-NOT: could not open '/clrthreadattribute:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}