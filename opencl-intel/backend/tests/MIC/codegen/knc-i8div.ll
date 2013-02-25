; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc
;

target datalayout = "e-p:64:64"

define i8 @i8div(i8* %arg0, i8* %arg1, i32 %num) {
; KNC: i8div:
; KNC: andl $255, %{{[a-z]+}}
; KNC: andl $255, %{{[a-z]+}}
; KNC: divl %{{[a-z]+}}

entry:
  %loc0 = getelementptr i8* %arg0, i32 %num
  %loc1 = getelementptr i8* %arg1, i32 %num

  %s0 = load i8* %loc0
  %s1 = load i8* %loc1 

  %res = udiv i8 %s0, %s1

  ret i8 %res
}

