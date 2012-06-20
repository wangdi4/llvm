; XFAIL: win32
; RUN: llc < %p/knc-%G -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %p/knc-%G -check-prefix=KNF
; enable the test below for KNC
; RUNc: llc < %p/knc-%G -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knf -disable-excess-fp-precision \
; RUNc:     | FileCheck %p/knc-%G -check-prefix=KNFmpa
