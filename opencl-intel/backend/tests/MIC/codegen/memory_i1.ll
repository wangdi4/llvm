; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

; KNF:  movb      (%rdi), %al
; KNF:  movb      %al, (%rsi)
; KNC:  movb      (%rdi), %al
; KNC:  movb      %al, (%rsi)
define void @test(i1* nocapture %a, i1* nocapture %b) nounwind {
  %1 = load i1* %a, align 1
  store i1 %1, i1* %b, align 1
  ret void
}
