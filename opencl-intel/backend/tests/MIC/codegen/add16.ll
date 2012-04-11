; XFAIL: *
; XFAIL: win32
; RUN: llc < %p/knc-%b -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf | FileCheck %p/knc-%b
