; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.spflt.16(i16)

define void @f_spflt_16(i16 %arg0) {
; KNF: f_spflt_16:
; KNF: spflt
entry:
  call void @llvm.x86.mic.spflt.16(i16 %arg0)

 ret void 
}

