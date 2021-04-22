; This test checks that the option /CLRSUPPORTLASTERROR causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /CLRSUPPORTLASTERROR flag is ignored
; RUN: not lld-link /subsystem:console /CLRSUPPORTLASTERROR %t.obj 2>&1 | FileCheck -check-prefix=CHECKCLR -allow-empty %s
; CHECKCLR:     warning: ignoring unknown argument '/clrsupportlasterror'
; CHECKCLR-NOT: could not open '/CLRSUPPORTLASTERROR'

; Check that the /clrsupportlasterror flag is ignored
; RUN: not lld-link /subsystem:console /clrsupportlasterror %t.obj 2>&1 | FileCheck -check-prefix=CHECKCLR2 -allow-empty %s
; CHECKCLR2:     warning: ignoring unknown argument '/clrsupportlasterror'
; CHECKCLR2-NOT: could not open '/clrsupportlasterror'

; Check that the /CLRSUPPORTLASTERROR: flag is ignored
; RUN: not lld-link /subsystem:console /CLRSUPPORTLASTERROR:NO %t.obj 2>&1 | FileCheck -check-prefix=CHECKCLR3 -allow-empty %s
; CHECKCLR3:     warning: ignoring unknown argument '/clrsupportlasterror:'
; CHECKCLR3-NOT: could not open '/CLRSUPPORTLASTERROR:'

; Check that the /clrsupportlasterror: flag is ignored
; RUN: not lld-link /subsystem:console /clrsupportlasterror:no %t.obj 2>&1 | FileCheck -check-prefix=CHECKCLR4 -allow-empty %s
; CHECKCLR4:     warning: ignoring unknown argument '/clrsupportlasterror:'
; CHECKCLR4-NOT: could not open '/clrsupportlasterror:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}