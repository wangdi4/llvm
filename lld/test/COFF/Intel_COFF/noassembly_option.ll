; This test checks that the option /NOASSEMBLY causes a warning to be
; printed and is then ignored.

; RUN: llc %s -o %t.obj -filetype=obj

; Check that the /NOASSEMBLY flag is ignored
; RUN: not lld-link /subsystem:console /NOASSEMBLY %t.obj 2>&1 | FileCheck -check-prefix=CHECKNA -allow-empty %s
; CHECKNA:     warning: ignoring unknown argument '/noassembly'
; CHECKNA-NOT: could not open '/NOASSEMBLY'

; Check that the /noassembly flag is ignored
; RUN: not lld-link /subsystem:console /noassembly %t.obj 2>&1 | FileCheck -check-prefix=CHECKNA2 -allow-empty %s
; CHECKNA2:     warning: ignoring unknown argument '/noassembly'
; CHECKNA2-NOT: could not open '/noassembly'

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}