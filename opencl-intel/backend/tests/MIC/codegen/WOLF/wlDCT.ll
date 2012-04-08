; XFAIL: *
; XFAIL: win32
; RUN: llc < %p/knf-%b -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc
