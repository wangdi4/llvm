; XFAIL: win32
; RUN: llc < %p/knc-%G -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf | FileCheck %p/knc-%G
