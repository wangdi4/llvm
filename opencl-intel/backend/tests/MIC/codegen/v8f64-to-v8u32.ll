; XFAIL: win32
; RUNc: llc < %p/knf-%b -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %p/knf-%b -check-prefix=KNC
