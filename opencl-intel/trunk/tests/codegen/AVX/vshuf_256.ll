; RUN: llc -mcpu=sandybridge < %s | FileCheck %s


target triple="x86_64-pc-win32"

define <4 x i64> @test5(<4 x i64> %A, <4 x i64> %B) nounwind {
; CHECK: test5
; CHECK: vshufpd $1, (%rdx), %ymm0, %ymm0
 %C = shufflevector <4 x i64> %A, <4 x i64> %B, <4 x i32> < i32 1, i32 4, i32 2, i32 6>
  ret <4 x i64> %C
}

define <4 x double> @test1(<4 x double> %A, <4 x double> %B) nounwind {
; CHECK: test1
; CHECK: vshufpd $1, (%rdx), %ymm0, %ymm0
 %C = shufflevector <4 x double> %A, <4 x double> %B, <4 x i32> < i32 1, i32 4, i32 2, i32 6>
  ret <4 x double> %C
}

define <8 x float> @test2(<8 x float> %A, <8 x float> %B) nounwind {
; CHECK: test2
; CHECK: vshufps $57, (%rdx), %ymm0, %ymm0
  %C = shufflevector <8 x float> %A, <8 x float> %B, <8 x i32> < i32 1, i32 2, i32 11, i32 8, i32 5, i32 6, i32 15, i32 12 >
  ret <8 x float> %C
}

 define <8 x float> @test3(<8 x float> %A, <8 x float> %B) nounwind {
; CHECK: test3
; CHECK:  vshufps $9, %ymm0, %ymm0, %ymm0
   %C = shufflevector <8 x float> %A, <8 x float> undef, <8 x i32> < i32 1, i32 2, i32 11, i32 8, i32 5, i32 6, i32 15, i32 12 >
	ret <8 x float> %C
 }

 define <8 x i32> @test4(<8 x i32> %A, <8 x i32> %B) nounwind {
; CHECK: test4
; CHECK: vshufps $57, (%rdx), %ymm0, %ymm0
  %C = shufflevector <8 x i32> %A, <8 x i32> %B, <8 x i32> < i32 1, i32 2, i32 11, i32 8, i32 5, i32 6, i32 15, i32 12 >
  ret <8 x i32> %C
}

define <8 x float> @test_vshufps(<8 x float> %v1) nounwind readonly {
; CHECK: test_vshufps
; CHECK: vshufps $9, %ymm0, %ymm0, %ymm0
  %t1 = shufflevector <8 x float> %v1, <8 x float> undef, <8 x i32> <i32 1, i32 2, i32 8, i32 11, i32 5, i32 6, i32 12, i32 15>
  ret <8 x float> %t1
}

define <8 x float> @test_vshufps2(<8 x float> %v1, <8 x float> %v2) nounwind readonly {
; CHECK: test_vshufps2
; CHECK: vshufps $-55, (%rcx), %ymm0, %ymm0
  %t1 = shufflevector <8 x float> %v1, <8 x float> %v2, <8 x i32> <i32 9, i32 10, i32 0, i32 3, i32 13, i32 14, i32 4, i32 7>
  ret <8 x float> %t1
}

define <4 x double> @test_vshufpd(<4 x double> %v1) nounwind readonly {
; CHECK: test_vshufpd
; CHECK: vshufpd $1, %ymm0, %ymm0, %ymm0
  %t1 = shufflevector <4 x double> %v1, <4 x double> undef, <4 x i32> <i32 1, i32 5, i32 2, i32 6>
  ret <4 x double> %t1
}

define <4 x double> @test_vshufpd2(<4 x double> %v1, <4 x double> %v2) nounwind readonly {
; CHECK: test_vshufpd2
; CHECK: vshufpd $8, (%rdx), %ymm0, %ymm0
  %t1 = shufflevector <4 x double> %v1, <4 x double> %v2, <4 x i32> <i32 0, i32 4, i32 2, i32 7>
  ret <4 x double> %t1
}
