; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

define <2 x i8> @A(<2 x i8> %a0, <2 x i8> %b0, <2 x i8> %arg) {
  %a = icmp eq <2 x i8> %a0, zeroinitializer
  %b = icmp eq <2 x i8> %b0, zeroinitializer
  %sel = or <2 x i1> %a, %b  
  %res = select <2 x i1> %sel, <2 x i8> <i8 1, i8 1>, <2 x i8> %arg
  ret <2 x i8> %res
}

define <2 x i8> @B(<2 x i8> %a0, <2 x i8> %b0, <2 x i8> %arg) {
  %a = icmp eq <2 x i8> %a0, zeroinitializer
  %b = icmp eq <2 x i8> %b0, zeroinitializer
  %sel = xor <2 x i1> %a, %b  
  %res = select <2 x i1> %sel, <2 x i8> <i8 1, i8 1>, <2 x i8> %arg
  ret <2 x i8> %res
}

define <2 x i8> @C(<2 x i8> %a0, <2 x i8> %b0, <2 x i8> %arg) {
  %a = icmp eq <2 x i8> %a0, zeroinitializer
  %b = icmp eq <2 x i8> %b0, zeroinitializer
  %sel = add <2 x i1> %a, %b  
  %res = select <2 x i1> %sel, <2 x i8> <i8 1, i8 1>, <2 x i8> %arg
  ret <2 x i8> %res
}

define <2 x i8> @D(<2 x i8> %a0, <2 x i8> %b0, <2 x i8> %arg) {
  %a = icmp eq <2 x i8> %a0, zeroinitializer
  %b = icmp eq <2 x i8> %b0, zeroinitializer
  %sel = sub <2 x i1> %a, %b  
  %res = select <2 x i1> %sel, <2 x i8> <i8 1, i8 1>, <2 x i8> %arg
  ret <2 x i8> %res
}

