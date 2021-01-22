; This test checks that the option /ASSEMBLYRESOURCE: causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /ASSEMBLYRESOURCE: flag is ignored
; RUN: not lld-link /subsystem:console /ASSEMBLYRESOURCE:resource.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKAR -allow-empty %s
; CHECKAR:     warning: ignoring unknown argument '/assemblyresource:'
; CHECKAR-NOT: could not open '/ASSEMBLYRESOURCE:'

; Check that the /assemblyresource: flag is ignored
; RUN: not lld-link /subsystem:console /assemblyresource:module.txt %t.obj 2>&1 | FileCheck -check-prefix=CHECKAR2 -allow-empty %s
; CHECKAR2:     warning: ignoring unknown argument '/assemblyresource:'
; CHECKAR2-NOT: could not open '/assemblyresource:'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}