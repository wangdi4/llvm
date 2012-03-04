; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define i32 @func32(i32 %x) nounwind readnone {
;KNF: movl      $1431655766
;KNF: imull
  %1 = srem i32 %x, 3
  ret i32 %1
}

define i64 @func64(i64 %x) nounwind readnone {
;KNF: idivq
  %1 = srem i64 %x, 3
  ret i64 %1
}

