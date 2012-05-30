; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

define <2 x i8> @sample_test(<2 x i8> %a0, <2 x i8> %b0, <2 x i8> %arg) {
  %a = icmp eq <2 x i8> %a0, zeroinitializer
  %b = icmp eq <2 x i8> %b0, zeroinitializer
  %sel = or <2 x i1> %a, %b  
  %res = select <2 x i1> %sel, <2 x i8> <i8 1, i8 1>, <2 x i8> %arg
  ret <2 x i8> %res
}
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

define <2 x i8> @sample_test(<2 x i8> %a0, <2 x i8> %b0, <2 x i8> %arg) {
  %a = icmp eq <2 x i8> %a0, zeroinitializer
  %b = icmp eq <2 x i8> %b0, zeroinitializer
  %sel = or <2 x i1> %a, %b  
  %res = select <2 x i1> %sel, <2 x i8> <i8 1, i8 1>, <2 x i8> %arg
  ret <2 x i8> %res
}
