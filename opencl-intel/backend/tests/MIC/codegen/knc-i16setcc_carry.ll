; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define i16 @i16setcc_carry(i16 %arg0, i16 %arg1) {
; KNC: i16setcc_carry:
; KNC: cmpw %si, %di
; KNC: movzbl %al, %eax

entry:
  %cmp = icmp ult i16 %arg0, %arg1
  %res = zext i1 %cmp to i16

  ret i16 %res
}

