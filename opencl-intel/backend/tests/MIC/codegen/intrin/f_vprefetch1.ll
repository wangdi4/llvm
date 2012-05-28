; XFAIL: win32
; RUN: echo
; RUNc: llc < %p/knc-%G -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knf \
; RUNc:     | FileCheck %p/knc-%G -check-prefix=KNF
