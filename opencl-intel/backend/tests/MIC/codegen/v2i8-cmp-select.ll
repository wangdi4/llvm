; XFAIL: win32
; RUN: llc < %p/knc-%G -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %p/knc-%G

