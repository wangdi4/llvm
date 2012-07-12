; XFAIL: win32
;
; RUN: llc < knc-v16i16-load-store.ll -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck knc-%s -check-prefix=KNF
