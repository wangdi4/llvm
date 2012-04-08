; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

declare i32 @llvm.x86.mic.mask8.to.int(<8 x i1>)
declare i32 @llvm.x86.mic.mask16.to.int(<16 x i1>)

define i32 @movmsk1(<16 x i1> %mask) nounwind ssp {
entry:
; KNF: vkmov [[R1:%k[12]+]], {{%e[a-z0-9]+}}
  %imask = call i32 @llvm.x86.mic.mask16.to.int(<16 x i1> %mask)
  ret i32 %imask
}

define i32 @movmsk2(<8 x i1> %mask) nounwind ssp {
entry:
; KNF: vkmov [[R1:%k[12]+]], [[R2:%e[a-z0-9]+]]
; KNF: andl  $255, [[R2]]
  %imask = call i32 @llvm.x86.mic.mask8.to.int(<8 x i1> %mask)
  ret i32 %imask
}
