; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define <8 x i16> @sra(<8 x i16> %arg0) {
; KNC: sra:
; KNC: vpsrad $1

entry:

 %res = ashr <8 x i16> %arg0, <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>

  ret <8 x i16> %res
}

