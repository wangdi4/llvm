; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s
;

target datalayout = "e-p:64:64"

define i64 @func(i32 %x) nounwind readnone {
; <label>:0
; CHECK:  movslq  %edi, %rax
  %.0 = sext i32 %x to i64
  ret i64 %.0
}


define i64 @func1(i32 %x) nounwind readnone {
; <label>:0
; CHECK:  movl  %edi, %rax
  %.0 = zext i32 %x to i64
  ret i64 %.0
}

