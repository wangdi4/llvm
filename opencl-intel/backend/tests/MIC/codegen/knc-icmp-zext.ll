; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

define <8 x i32> @test_uint8(<8 x float> %a, <8 x float> %b) {

  %cmpvector_func.i = fcmp oeq <8 x float> %a, %b
  %res = zext <8 x i1> %cmpvector_func.i to <8 x i32>

  ret <8 x i32> %res
}

define <16 x i32> @test_uint16(<16 x i16> %a, <16 x i16> %b) {

  %cmpvector_func.i = icmp ne <16 x i16> %a, %b
  %res = zext <16 x i1> %cmpvector_func.i to <16 x i32>

  ret <16 x i32> %res
}

