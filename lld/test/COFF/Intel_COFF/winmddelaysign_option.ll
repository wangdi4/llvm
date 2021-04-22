; This test checks that the option /WINMDDELAYSIGN causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /WINMDDELAYSIGN flag is ignored
; RUN: not lld-link /subsystem:console /WINMDDELAYSIGN %t.obj 2>&1 | FileCheck -check-prefix=CHECKWD -allow-empty %s
; CHECKWD:     warning: ignoring unknown argument '/winmddelaysign'
; CHECKWD-NOT: could not open '/WINMDDELAYSIGN'

; Check that the /winmddelaysign flag is ignored
; RUN: not lld-link /subsystem:console /winmddelaysign %t.obj 2>&1 | FileCheck -check-prefix=CHECKWD2 -allow-empty %s
; CHECKWD2:     warning: ignoring unknown argument '/winmddelaysign'
; CHECKWD2-NOT: could not open '/winmddelaysign'

; Check that the /WINMDDELAYSIGN: flag is ignored
; RUN: not lld-link /subsystem:console /WINMDDELAYSIGN:NO %t.obj 2>&1 | FileCheck -check-prefix=CHECKWD3 -allow-empty %s
; CHECKWD3:     warning: ignoring unknown argument '/winmddelaysign:'
; CHECKWD3-NOT: could not open '/WINMDDELAYSIGN:'

; Check that the /winmddelaysign: flag is ignored
; RUN: not lld-link /subsystem:console /winmddelaysign:no %t.obj 2>&1 | FileCheck -check-prefix=CHECKWD4 -allow-empty %s
; CHECKWD4:     warning: ignoring unknown argument '/winmddelaysign:'
; CHECKWD4-NOT: could not open '/winmddelaysign:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}