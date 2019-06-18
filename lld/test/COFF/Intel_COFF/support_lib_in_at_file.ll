; This test checks that the function to load /lib works with
; response files (@).

; Without bitcode
; RUN: llc %s -o %t.obj -filetype=obj
; RUN: echo "/lib /OUT:%t.lib %t.obj" > %t.lnk
; RUN: lld-link @%t.lnk
; RUN: llvm-readobj -symbols %t.lib | FileCheck %s

; CHECK: Format: COFF-x86
; CHECK: Arch: x86
; CHECK: Symbol
; CHECK:   Name: add

target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

define internal i32 @add(i32 %a) {
entry:
  %add = add nsw i32 %a, 2
  ret i32 %add
}
