; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s -check-prefix=KNC

target datalayout = "e-p:64:64"

declare void @llvm.x86.mic.setvxcsr(i32)

define void @f_setvxcsr(i32 %arg0) {
; KNF: f_setvxcsr:
; KNF: setvxcsr
entry:
  call void @llvm.x86.mic.setvxcsr(i32 %arg0)

 ret void 
}

