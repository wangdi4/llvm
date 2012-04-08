; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"

define i32 @test1(i32 %a) nounwind readnone ssp {
entry:
; KNF: test1:
; KNF: countbitsl

; KNC: test1:
; KNC: popcntl
  %conv = call i32 @llvm.x86.popcnt.u32(i32 %a)
  ret i32 %conv
}

define i64 @test2(i64 %a) nounwind readnone ssp {
entry:
; KNF: test2:
; KNF: countbitsq

; KNC: test2:
; KNC: popcntq
  %conv = call i64 @llvm.x86.popcnt.u64(i64 %a)
  ret i64 %conv
}

declare i32 @llvm.x86.popcnt.u32(i32)
declare i64 @llvm.x86.popcnt.u64(i64)
