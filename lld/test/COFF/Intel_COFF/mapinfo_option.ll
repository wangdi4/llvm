; This test checks that the option /MAPINFO causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /MAPINFO: flag is ignored
; RUN: not lld-link /subsystem:console /MAPINFO:EXPORTS %t.obj 2>&1 | FileCheck -check-prefix=CHECKMI -allow-empty %s
; CHECKMI:     warning: ignoring unknown argument '/mapinfo:'
; CHECKMI-NOT: could not open '/MAPINFO:'

; Check that the /mapinfo: flag is ignored
; RUN: not lld-link /subsystem:console /mapinfo:exports %t.obj 2>&1 | FileCheck -check-prefix=CHECKMI2 -allow-empty %s
; CHECKMI2:     warning: ignoring unknown argument '/mapinfo:'
; CHECKMI2-NOT: could not open '/mapinfo:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}