; XFAIL: win32

; RUN: echo
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.spflt.64(i64)

define void @f_spflt_64(i64 %arg0) {
; KNF: f_spflt_64:
; KNF: spflt
entry:
  call void @llvm.x86.mic.spflt.64(i64 %arg0)

 ret void 
}

