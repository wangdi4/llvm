; XFAIL: win32
; XFAIL: *
; RUN: llc < %p/knc-%G -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %p/knc-%G -check-prefix=KNF
; RUN: llc < %p/knc-%G -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf -disable-excess-fp-precision \
; RUN:     | FileCheck %p/knc-%G -check-prefix=KNFmpa 
