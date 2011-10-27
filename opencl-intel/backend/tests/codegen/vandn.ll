; RUN: llc -mcpu=sandybridge < %s | FileCheck %s

define <8 x i32> @test_v8i32_rr_swapped(<8 x i32> %b, <8 x i32> %a) {
; CHECK: test_v8i32_rr_swapped
; CHECK: vandnps %ymm
    %a0 = bitcast <8 x i32> %a to <8 x float>
    %b0 = bitcast <8 x i32> %b to <8 x float>
    %a1 = fadd <8 x float> %a0, <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>
    %b1 = fadd <8 x float> %b0, <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>
    %a2 = bitcast <8 x float> %a1 to <8 x i32>
    %b2 = bitcast <8 x float> %b1 to <8 x i32>
    %t0 = xor <8 x i32> %a2, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
    %t1 = and <8 x i32> %t0, %b2
    ret <8 x i32> %t1
}

define <8 x i32> @test_v8i32_rm_swapped(<8 x i32> %b, <8 x i32>* %a) {
; CHECK1: test_v8i32_rm_swapped
; CHECK: vandnpd (
    %a0 = load <8 x i32>* %a
    %t0 = xor <8 x i32> %b, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
    %t1 = and <8 x i32> %t0, %a0
    ret <8 x i32> %t1
}

define <8 x i32> @test_v8i32_rr(<8 x i32> %a, <8 x i32> %b) {
; CHECK: test_v8i32_rr
; CHECK: vandnps %ymm
    %a0 = bitcast <8 x i32> %a to <8 x float>
    %b0 = bitcast <8 x i32> %b to <8 x float>
    %a1 = fadd <8 x float> %a0, <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>
    %b1 = fadd <8 x float> %b0, <float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0, float 1.0>
    %a2 = bitcast <8 x float> %a1 to <8 x i32>
    %b2 = bitcast <8 x float> %b1 to <8 x i32>
    %t0 = xor <8 x i32> %a2, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
    %t1 = and <8 x i32> %t0, %b2
    ret <8 x i32> %t1
}

define <8 x i32> @test_v8i32_rm(<8 x i32> %a, <8 x i32>* %b) {
; CHECK: test_v8i32_rm
; CHECK: vandnpd (
    %b0 = load <8 x i32>* %b
    %t0 = xor <8 x i32> %a, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
    %t1 = and <8 x i32> %t0, %b0
    ret <8 x i32> %t1
    }

define <8 x i32> @test_v8f32_rm(<8 x float> %b, <8 x float>* %a) {
; CHECK: test_v8f32_rm
; CHECK: vandnps (
    %a1 = load <8 x float>* %a
    %a0 = bitcast <8 x float> %a1 to <8 x i32>
    %b0 = bitcast <8 x float> %b to <8 x i32>
    %t0 = xor <8 x i32> %b0, <i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1, i32 -1>
    %t1 = and <8 x i32> %t0, %a0
    ret <8 x i32> %t1
}

define <4 x double> @test_v4f64_rm(<4 x double> %a, <4 x double>* %b) {
; CHECK: test_v4f64_rm
; CHECK: vandnpd (
    %b0 = load <4 x double>* %b
    %t0 = bitcast <4 x double> %a to <4 x i64>
    %t1 = bitcast <4 x double> %b0 to <4 x i64>
    %t2 = xor <4 x i64> %t0, <i64 -1, i64 -1, i64 -1, i64 -1>
    %t3 = and <4 x i64> %t2, %t1
    %t4 = bitcast <4 x i64> %t3 to <4 x double>
    ret <4 x double> %t4
}

define <4 x i64> @test_v4i64_rr(<4 x i64> %a, <4 x i64> %b) {
; CHECK: test_v4i64_rr
; CHECK: vandnpd %ymm
    %t0 = xor <4 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1>
    %t1 = and <4 x i64> %t0, %b
    ret <4 x i64> %t1
}

define <4 x i64> @test_v4i64_rm(<4 x i64> %a, <4 x i64>* %b) {
; CHECK: test_v4i64_rm
; CHECK: vandnpd (
    %b0 = load <4 x i64>* %b
    %t0 = xor <4 x i64> %a, <i64 -1, i64 -1, i64 -1, i64 -1>
    %t1 = and <4 x i64> %t0, %b0
    ret <4 x i64> %t1
}

define <16 x i16> @test_v16i16_rr(<16 x i16> %a, <16 x i16> %b) {
; CHECK: test_v16i16_rr
; CHECK: vandnpd %ymm
    %t0 = xor <16 x i16> %a, <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>
    %t1 = and <16 x i16> %t0, %b
    ret <16 x i16> %t1
}

define <16 x i16> @test_v16i16_rm(<16 x i16> %a, <16 x i16>* %b) {
; CHECK: test_v16i16_rm
; CHECK: vandnpd (
    %b0 = load <16 x i16>* %b
    %t0 = xor <16 x i16> %a, <i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1, i16 -1>
    %t1 = and <16 x i16> %t0, %b0
    ret <16 x i16> %t1
}
