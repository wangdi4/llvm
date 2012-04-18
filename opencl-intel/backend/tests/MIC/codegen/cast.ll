; XFAIL: win32
; XFAIL: *
; RUN: llc < %p/knc-%G -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %p/knc-%G
