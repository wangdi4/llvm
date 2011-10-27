; RUN: llc -mcpu=sandybridge < %s | FileCheck %s

target triple="x86_64-pc-win32"

define <8 x float> @test_vpermilps(<8 x float> %v1) nounwind readonly {
; CHECK: test_vpermilps
; CHECK: vpermilps       $-24
  %t1 = shufflevector <8 x float> %v1, <8 x float> undef, <8 x i32> <i32 0, i32 2, i32 2, i32 3, i32 4, i32 6, i32 6, i32 7>
  ret <8 x float> %t1
}
define <8 x float> @test_vpermilps2(<8 x float> %v1) nounwind readonly {
; CHECK: test_vpermilps2
; CHECK: vpermilps       $-24
  %t1 = shufflevector <8 x float> %v1, <8 x float> undef, <8 x i32> <i32 0, i32 undef, i32 2, i32 3, i32 4, i32 6, i32 6, i32 undef>
  ret <8 x float> %t1
}

define <8 x i32> @test_vpermilps3(<8 x i32> %v1) nounwind readonly {
; CHECK: test_vpermilps3
; CHECK: vpermilps       $104
  %t1 = shufflevector <8 x i32> %v1, <8 x i32> undef, <8 x i32> <i32 0, i32 undef, i32 2, i32 1, i32 4, i32 6, i32 6, i32 undef>
  ret <8 x i32> %t1
}

define <8 x float> @test_vpermilps4(<8 x float> %v1) nounwind readonly {
; CHECK: test_vpermilps4
; CHECK: vpermilps       $-8
  %t1 = shufflevector <8 x float> %v1, <8 x float> undef, <8 x i32> <i32 0, i32 2, i32 3, i32 3, i32 4, i32 6, i32 7, i32 7>
  ret <8 x float> %t1
}

define <4 x double> @test_vpermilpd(<4 x double> addrspace(1)* nocapture %source) nounwind readonly {
; CHECK: test_vpermilpd
; CHECK: vpermilpd       $7
  %v1 = load <4 x double> addrspace(1)* %source, align 64
  %t1 = shufflevector <4 x double> %v1, <4 x double> undef, <4 x i32> <i32 1, i32 1, i32 3, i32 2>
  ret <4 x double> %t1
}
define <4 x double> @test_vpermilpd2(<4 x double> addrspace(1)* nocapture %source) nounwind readonly {
; CHECK: test_vpermilpd2
; CHECK: vpermilpd       $13
  %v1 = load <4 x double> addrspace(1)* %source, align 64
  %t1 = shufflevector <4 x double> %v1, <4 x double> undef, <4 x i32> <i32 1, i32 undef, i32 3, i32 3>
  ret <4 x double> %t1
}

