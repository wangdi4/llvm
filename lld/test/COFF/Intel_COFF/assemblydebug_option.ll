; This test checks that the option /ASSEMBLYDEBUG causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /ASSEMBLYDEBUG flag is ignored
; RUN: not lld-link /subsystem:console /ASSEMBLYDEBUG %t.obj 2>&1 | FileCheck -check-prefix=CHECKAD -allow-empty %s
; CHECKAD:     warning: ignoring unknown argument '/assemblydebug'
; CHECKAD-NOT: could not open '/ASSEMBLYDEBUG'

; Check that the /assemblydebug flag is ignored
; RUN: not lld-link /subsystem:console /assemblydebug %t.obj 2>&1 | FileCheck -check-prefix=CHECKAD2 -allow-empty %s
; CHECKAD2:     warning: ignoring unknown argument '/assemblydebug'
; CHECKAD2-NOT: could not open '/assemblydebug'

; Check that the /ASSEMBLYDEBUG: flag is ignored
; RUN: not lld-link /subsystem:console /ASSEMBLYDEBUG:DISABLE %t.obj 2>&1 | FileCheck -check-prefix=CHECKAD3 -allow-empty %s
; CHECKAD3:     warning: ignoring unknown argument '/assemblydebug:'
; CHECKAD3-NOT: could not open '/ASSEMBLYDEBUG:'

; Check that the /assemblydebug: flag is ignored
; RUN: not lld-link /subsystem:console /assemblydebug:disable %t.obj 2>&1 | FileCheck -check-prefix=CHECKAD4 -allow-empty %s
; CHECKAD4:     warning: ignoring unknown argument '/assemblydebug:'
; CHECKAD4-NOT: could not open '/assemblydebug:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}