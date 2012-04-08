; XFAIL: win32
; RUN: llc < %p/knf-%b -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %p/knf-%b
