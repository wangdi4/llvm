; This test checks that the option /WINMDKEYCONTAINER causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /WINMDKEYCONTAINER: flag is ignored
; RUN: not lld-link /subsystem:console /WINMDKEYCONTAINER:container %t.obj 2>&1 | FileCheck -check-prefix=CHECKWC -allow-empty %s
; CHECKWC:     warning: ignoring unknown argument '/winmdkeycontainer:'
; CHECKWC-NOT: could not open '/WINMDKEYCONTAINER:'

; Check that the /winmdkeycontainer: flag is ignored
; RUN: not lld-link /subsystem:console /winmdkeycontainer:container %t.obj 2>&1 | FileCheck -check-prefix=CHECKWC2 -allow-empty %s
; CHECKWC2:     warning: ignoring unknown argument '/winmdkeycontainer:'
; CHECKWC2-NOT: could not open '/winmdkeycontainer:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}