; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core --sycl-kernel-replace-fdiv-fast-with-svml=true -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-inst-to-func-call -sycl-vector-variant-isa-encoding-override=AVX512Core --sycl-kernel-replace-fdiv-fast-with-svml=true -S %s | FileCheck %s

define float @test_fdiv_v1f32(float %x, float %y) nounwind {
; CHECK-LABEL: @test_fdiv_v1f32
; CHECK: call float @_Z9divide_rmff(float %x, float %y)
; CHECK: fdiv
; CHECK-NOT: fdiv fast
  %1 = fdiv fast float %x, %y
  %2 = fdiv float %x, %y
  ret float %1
}

define <2 x float> @test_fdiv_v2f32(<2 x float> %x, <2 x float> %y) nounwind {
; CHECK-LABEL: @test_fdiv_v2f32
; CHECK: call <2 x float> @_Z9divide_rmDv2_fS_(<2 x float> %x, <2 x float> %y)
; CHECK: fdiv
; CHECK-NOT: fdiv fast
  %1 = fdiv fast <2 x float> %x, %y
  %2 = fdiv <2 x float> %x, %y
  ret <2 x float> %1
}

define <3 x float> @test_fdiv_v3f32(<3 x float> %x, <3 x float> %y) nounwind {
; CHECK-LABEL: @test_fdiv_v3f32
; CHECK: call <3 x float> @_Z9divide_rmDv3_fS_(<3 x float> %x, <3 x float> %y)
; CHECK: fdiv
; CHECK-NOT: fdiv fast
  %1 = fdiv fast <3 x float> %x, %y
  %2 = fdiv <3 x float> %x, %y
  ret <3 x float> %1
}

define <4 x float> @test_fdiv_v4f32(<4 x float> %x, <4 x float> %y) nounwind {
; CHECK-LABEL: @test_fdiv_v4f32
; CHECK: call <4 x float> @_Z9divide_rmDv4_fS_(<4 x float> %x, <4 x float> %y)
; CHECK: fdiv
; CHECK-NOT: fdiv fast
  %1 = fdiv fast <4 x float> %x, %y
  %2 = fdiv <4 x float> %x, %y
  ret <4 x float> %1
}

define <8 x float> @test_fdiv_v8f32(<8 x float> %x, <8 x float> %y) nounwind {
; CHECK-LABEL: @test_fdiv_v8f32
; CHECK: call <8 x float> @_Z9divide_rmDv8_fS_(<8 x float> %x, <8 x float> %y)
; CHECK: fdiv
; CHECK-NOT: fdiv fast
  %1 = fdiv fast <8 x float> %x, %y
  %2 = fdiv <8 x float> %x, %y
  ret <8 x float> %1
}

define <16 x float> @test_fdiv_v16f32(<16 x float> %x, <16 x float> %y) nounwind {
; CHECK-LABEL: @test_fdiv_v16f32
; CHECK: call <16 x float> @_Z9divide_rmDv16_fS_(<16 x float> %x, <16 x float> %y)
; CHECK: fdiv
; CHECK-NOT: fdiv fast
  %1 = fdiv fast <16 x float> %x, %y
  %2 = fdiv <16 x float> %x, %y
  ret <16 x float> %1
}

; DEBUGIFY-NOT: WARNING
