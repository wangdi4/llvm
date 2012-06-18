; XFAIL: win32
; XFAIL: *
;
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knc \
; RUN:     | FileCheck %s
;

target datalayout = "e-p:64:64"

define i16 @bitcast16(<16 x i1> %m) nounwind ssp {
; CHECK: bitcast16:
; CHECK: vkmov %k1, %edx
; CHECK: movswq    %dx, %rax
  %mask = bitcast <16 x i1> %m to i16
  ret i16 %mask
}

define i8 @bitcast8(<8 x i1> %m) nounwind ssp {
; CHECK: bitcast8:
; CHECK: vkmov %k1, %eax
  %mask = bitcast <8 x i1> %m to i8
  ret i8 %mask
}

