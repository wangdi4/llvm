; RUN: opt < %s -passes='require<anders-aa>' -disable-output 2>/dev/null
; Test Andersens analysis that caused crash when vectorType instructions
; were used. These Vector type patterns appear when Andersens analysis
; is enabled at LTO.

define i32 @test_extractelement(<4 x i32> %V) {
        %R1 = extractelement <4 x i32> %V, i32 1         ; <i32> [#uses=1]
        ret i32 %R1
}

define <4 x i32> @test_insertelement(<4 x i32> %V) {
        %R2 = insertelement <4 x i32> %V, i32 0, i32 0           ; <<4 x i32>> [#uses=1]
        ret <4 x i32> %R2
}

define <4 x float> @test_shufflevector_f(<4 x float> %V) {
        %R3 = shufflevector <4 x float> %V, <4 x float> undef, <4 x i32> < i32 1, i32 undef, i32 7, i32 2 >      ; <<4 x float>> [#uses=1]
        ret <4 x float> %R3
}

