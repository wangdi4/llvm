; This test checks that the option /KEYCONTAINER causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /KEYCONTAINER: flag is ignored
; RUN: not lld-link /subsystem:console /KEYCONTAINER:container %t.obj 2>&1 | FileCheck -check-prefix=CHECKKC -allow-empty %s
; CHECKKC:     warning: ignoring unknown argument '/keycontainer:'
; CHECKKC-NOT: could not open '/KEYCONTAINER:'

; Check that the /keycontainer: flag is ignored
; RUN: not lld-link /subsystem:console /keycontainer:container %t.obj 2>&1 | FileCheck -check-prefix=CHECKKC2 -allow-empty %s
; CHECKKC2:     warning: ignoring unknown argument '/keycontainer:'
; CHECKKC2-NOT: could not open '/keycontainer:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}