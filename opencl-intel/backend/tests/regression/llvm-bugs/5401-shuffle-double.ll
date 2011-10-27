;RUN: llc < %s | FileCheck %s
; CHECK: unpcklpd
; CHECK: ret

 define  <2 x double> @test_even_double4(<4 x double>* %srcA, <2 x double>* %dst) {
  %i5 = getelementptr inbounds <4 x double>* %srcA, i32 3
  %i6 = load <4 x double>* %i5, align 32
  %i7 = shufflevector <4 x double> %i6, <4 x double> undef, <2 x i32> <i32 0, i32 2>
  ret <2 x double> %i7
}


