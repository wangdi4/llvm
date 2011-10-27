; RUN: llc < %s -mcpu=sandybridge | FileCheck %s

define <2 x double> @test1(double* nocapture %f) nounwind readonly {
; CHECK: test1
; CHECK: vmovddup
  %1 = load double* %f
  %2 = insertelement <2 x double> undef, double %1, i32 1
  %3 = insertelement <2 x double> %2, double %1, i32 0
  ret <2 x double> %3
}

define <4 x float> @test2(float* nocapture %f) nounwind readonly {
; CHECK: test2
; CHECK: vbroadcastss
  %1 = load float* %f
  %2 = insertelement <4 x float> undef, float %1, i32 3
  %3 = insertelement <4 x float> %2, float %1, i32 2
  %4 = insertelement <4 x float> %3, float %1, i32 1
  %5 = insertelement <4 x float> %4, float %1, i32 0
  ret <4 x float> %5
}

define <4 x i32> @test5(i32* nocapture %f) nounwind readonly {
; CHECK: test5
; CHECK: vbroadcastss
  %1 = load i32* %f
  %2 = insertelement <4 x i32> undef, i32 %1, i32 3
  %3 = insertelement <4 x i32> %2, i32 %1, i32 2
  %4 = insertelement <4 x i32> %3, i32 %1, i32 1
  %5 = insertelement <4 x i32> %4, i32 %1, i32 0
  ret <4 x i32> %5
}

;i64 is expanded?
define <2 x i64> @test4(i64* nocapture %f) nounwind readonly {
; CHECK: test4
  %1 = load i64* %f
  %2 = insertelement <2 x i64> undef, i64 %1, i32 1
  %3 = insertelement <2 x i64> %2, i64 %1, i32 0
  ret <2 x i64> %3
}

define <8 x i32> @test6(i32* nocapture %f) nounwind readonly {
; CHECK: test6
; CHECK: vbroadcastss
  %1 = load i32* %f
  %2 = insertelement <8 x i32> undef, i32 %1, i32 7
  %3 = insertelement <8 x i32> %2, i32 %1, i32 6
  %4 = insertelement <8 x i32> %3, i32 %1, i32 5
  %5 = insertelement <8 x i32> %4, i32 %1, i32 4
  %6 = insertelement <8 x i32> %5, i32 %1, i32 3
  %7 = insertelement <8 x i32> %6, i32 %1, i32 2
  %8 = insertelement <8 x i32> %7, i32 %1, i32 1
  %9 = insertelement <8 x i32> %8, i32 %1, i32 0
  ret <8 x i32> %9
}

; splat mixed with undef
define <8 x i32> @test7(i32* nocapture %f) nounwind readonly {
; CHECK: test7
; CHECK: vbroadcastss
  %1 = load i32* %f
  %2 = insertelement <8 x i32> undef, i32 %1, i32 7
  %3 = insertelement <8 x i32> %2, i32 %1, i32 6
  %4 = insertelement <8 x i32> %3, i32 %1, i32 5
  %5 = insertelement <8 x i32> %4, i32 %1, i32 4
  %6 = insertelement <8 x i32> %5, i32 %1, i32 2
  %7 = insertelement <8 x i32> %6, i32 %1, i32 1
  %8 = insertelement <8 x i32> %7, i32 %1, i32 0
  ret <8 x i32> %8
}

define <4 x double> @test3(double* nocapture %f) nounwind readonly {
; CHECK: test3
; CHECK: vbroadcastsd
  %1 = load double* %f
  %2 = insertelement <4 x double> undef, double %1, i32 3
  %3 = insertelement <4 x double> %2, double %1, i32 2
  %4 = insertelement <4 x double> %3, double %1, i32 1
  %5 = insertelement <4 x double> %4, double %1, i32 0
  ret <4 x double> %5
}


; splat mixed with undef
define <8 x i32> @test8(i32* nocapture %f) nounwind readonly {
; CHECK: test8
; CHECK: vbroadcastss
  %1 = load i32* %f
  %2 = insertelement <8 x i32> undef, i32 %1, i32 7
  %3 = insertelement <8 x i32> %2, i32 %1, i32 0
  ret <8 x i32> %3
}

; splat mixed with undef
define <8 x i32> @test9(i32* nocapture %f) nounwind readonly {
; CHECK: test9
; CHECK: vbroadcastss
  %1 = load i32* %f
  %2 = insertelement <8 x i32> undef, i32 %1, i32 7
  ret <8 x i32> %2
}

define  <8 x float> @shufflevector_float8(float* %ARG0) nounwind {
; CHECK: shufflevector_float8
; CHECK: vbroadcastss
  %a = load float* %ARG0
  %1 = insertelement <8 x float> undef, float %a, i32 3
  %2 = shufflevector <8 x float> %1, <8 x float> undef, <8 x i32> <i32 8, i32 9, i32 10, i32 3, i32 11, i32 13, i32 14, i32 12>
  ret <8 x float> %2
}

; This test crashes if we allow loads with chains to be selected into VBROADCAST
define  void @sample_test2(i64* %ptr, <8 x i64>* %out, <8 x i64> %x) nounwind {
  %t2 = load i64* %ptr, align 8
  %t3 = insertelement <8 x i64> undef, i64 %t2, i32 7
  store <8 x i64> %t3, <8 x i64>* %out, align 64
  ret void
}



; CHECK: vector_code
define <8 x float> @vector_code()  {
   ret <8 x float> <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>
}

; CHECK: vector_code8
; CHECK-NOT: vbroadcast
define <8 x i8> @vector_code8( )  {
   ret <8 x i8> <i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1, i8 1>
} 

; CHECK: vector_code16
; CHECK-NOT: vbroadcast
define <8 x i16> @vector_code16()  {
   ret <8 x i16> <i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
} 

; CHECK: vector_code32
define <8 x i32> @vector_code32()  {
   ret <8 x i32> <i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
} 

; CHECK: vector_code64
define <8 x i64> @vector_code64()  {
   ret <8 x i64> <i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1, i64 1>
} 


; CHECK: vector_codef64
define <8 x double> @vector_codef64()  {
   ret <8 x double> <double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0, double 1.0>
} 


; CHECK: vector4_code
define <4 x float> @vector4_code(<4 x i64> %A, <4 x i64> %B, <4 x float> %R0, <4 x float> %R1 )  {
   ret <4 x float> <float 1.0, float 1.0, float 1.0, float 1.0>
} 


; CHECK: vector4_code32
define <4 x i32> @vector4_code32()  {
   ret <4 x i32> <i32 1, i32 1, i32 1, i32 1>
} 

; CHECK: vector4_code64
define <4 x i64> @vector4_code64()  {
   ret <4 x i64> <i64 1, i64 1, i64 1, i64 1>
} 

; CHECK: vector4_codef64
define <4 x double> @vector4_codef64()  {
   ret <4 x double> <double 1.0, double 1.0, double 1.0, double 1.0>
} 

define  x86_ocl_kernelcc void @__Vectorized_CryptThread(i32 %blockLen) nounwind {
.preheader14:
  %temp = insertelement <8 x i32> undef, i32 %blockLen, i32 0
  %vector = shufflevector <8 x i32> %temp, <8 x i32> undef, <8 x i32> zeroinitializer
  %0 = icmp ugt i32 %blockLen, 63
  br i1 %0, label %bb.nph17, label %._crit_edge18

bb.nph17:                                         ; preds = %.preheader14
  %tmp562 = mul <8 x i32> undef, %vector
  unreachable

._crit_edge18:                                    ; preds = %.preheader14
  unreachable
}

define <4 x float> @float4_splat1(float * %a) {
; CHECK: float4_splat1
; CHECK: vbroadcastss
  %b13 = load float* %a, align 4
  %temp = insertelement <4 x float> undef, float %b13, i32 1
  %vector = shufflevector <4 x float> %temp, <4 x float> undef, <4 x i32> <i32 1, i32 1, i32 1, i32 1>
  ret <4 x float> %vector
}

define <4 x float> @float4_splat0(float * %a) {
; CHECK: float4_splat0
; CHECK: vbroadcastss
  %b13 = load float* %a, align 4
  %temp = insertelement <4 x float> undef, float %b13, i32 0
  %vector = shufflevector <4 x float> %temp, <4 x float> undef, <4 x i32> zeroinitializer
  ret <4 x float> %vector
}

define <8 x float> @float8_splat0(float * %a) {
; CHECK: float8_splat0
; CHECK: vbroadcastss
  %b13 = load float* %a, align 4
  %temp = insertelement <8 x float> undef, float %b13, i32 0
  %vector = shufflevector <8 x float> %temp, <8 x float> undef, <8 x i32> zeroinitializer
  ret <8 x float> %vector
}

