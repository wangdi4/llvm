; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.spflt.32(i32)

define void @f_spflt_32(i32 %arg0) {
; KNF: f_spflt_32:
; KNF: spflt
entry:
  call void @llvm.x86.mic.spflt.32(i32 %arg0)

 ret void 
}

