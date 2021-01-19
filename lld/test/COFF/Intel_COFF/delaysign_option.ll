; This test checks that the option /DELAYSIGN causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /DELAYSIGN flag is ignored
; RUN: not lld-link /subsystem:console /DELAYSIGN %t.obj 2>&1 | FileCheck -check-prefix=CHECKDS -allow-empty %s
; CHECKDS:     warning: ignoring unknown argument '/delaysign'
; CHECKDS-NOT: could not open '/DELAYSIGN'

; Check that the /delaysign flag is ignored
; RUN: not lld-link /subsystem:console /delaysign %t.obj 2>&1 | FileCheck -check-prefix=CHECKDS2 -allow-empty %s
; CHECKDS2:     warning: ignoring unknown argument '/delaysign'
; CHECKDS2-NOT: could not open '/delaysign'

; Check that the /DELAYSIGN: flag is ignored
; RUN: not lld-link /subsystem:console /DELAYSIGN:NO %t.obj 2>&1 | FileCheck -check-prefix=CHECKDS3 -allow-empty %s
; CHECKDS3:     warning: ignoring unknown argument '/delaysign:'
; CHECKDS3-NOT: could not open '/DELAYSIGN:'

; Check that the /delaysign: flag is ignored
; RUN: not lld-link /subsystem:console /delaysign:no %t.obj 2>&1 | FileCheck -check-prefix=CHECKDS4 -allow-empty %s
; CHECKDS4:     warning: ignoring unknown argument '/delaysign:'
; CHECKDS4-NOT: could not open '/delaysign:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}