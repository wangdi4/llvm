; INTEL_FEATURE_CPU_RYL
; REQUIRES: intel_feature_cpu_ryl
; Test that we can compile avx512 code in case of multiversioning.
; ZMM registers must be presented.
; RUN: llc -mtriple=x86_64-unknown-unknown -mcpu=royal -O2 < %s | FileCheck %s

define dso_local i32 @add_reduction_avx512(ptr nocapture noundef readonly %arr) #0 {
; CHECK-LABEL: add_reduction_avx512:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    vmovdqu64 (%rdi), %zmm0
; CHECK-NEXT:    vpaddd 64(%rdi), %zmm0, %zmm0
; CHECK-NEXT:    vextracti64x4 $1, %zmm0, %ymm1
; CHECK-NEXT:    vpaddd %zmm1, %zmm0, %zmm0
; CHECK-NEXT:    vextracti128 $1, %ymm0, %xmm1
; CHECK-NEXT:    vpaddd %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vpshufd {{.*#+}} xmm1 = xmm0[2,3,2,3]
; CHECK-NEXT:    vpaddd %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vpshufd {{.*#+}} xmm1 = xmm0[1,1,1,1]
; CHECK-NEXT:    vpaddd %xmm1, %xmm0, %xmm0
; CHECK-NEXT:    vmovd %xmm0, %eax
; CHECK-NEXT:    vzeroupper
; CHECK-NEXT:    retq
entry:
  %0 = load <32 x i32>, ptr %arr, align 4
  %1 = tail call i32 @llvm.vector.reduce.add.v32i32(<32 x i32> %0)
  ret i32 %1
}

declare i32 @llvm.vector.reduce.add.v32i32(<32 x i32>)

attributes #0 = { nounwind "target-features"="+avx512f,+evex512" }

; end INTEL_FEATURE_CPU_RYL
