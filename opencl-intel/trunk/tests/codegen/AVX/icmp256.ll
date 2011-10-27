; RUN: llc -mcpu=sandybridge < %s | FileCheck %s
; ModuleID = '<stdin>'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

define <8 x i32> @test1(<8 x i32> %a, <8 x i32> %b) nounwind {
; CHECK: test1
; CHECK: vpcmpgtd
; CHECK: vpcmpgtd
  %1 = icmp slt <8 x i32> %a, %b
  %2 = sext <8 x i1> %1 to <8 x i32>
  ret <8 x i32> %2
}

define <4 x i64> @test2(<4 x i64> %a, <4 x i64> %b) nounwind {
; CHECK: test2
; CHECK: vpcmpgtq
; CHECK: vpcmpgtq
  %1 = icmp slt <4 x i64> %a, %b
  %2 = sext <4 x i1> %1 to <4 x i64>
  ret <4 x i64> %2
}

define <16 x i16> @test3(<16 x i16> %a, <16 x i16> %b) nounwind {
; CHECK: test3
; CHECK: vpcmpgtw
; CHECK: vpcmpgtw
  %1 = icmp slt <16 x i16> %a, %b
  %2 = sext <16 x i1> %1 to <16 x i16>
  ret <16 x i16> %2
}

define <32 x i8> @test4(<32 x i8> %a, <32 x i8> %b) nounwind {
; CHECK: test4
; CHECK: vpcmpgtb 
; CHECK: vpcmpgtb 
  %1 = icmp slt <32 x i8> %a, %b
  %2 = sext <32 x i1> %1 to <32 x i8>
  ret <32 x i8> %2
}
