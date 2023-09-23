target triple = "spir64-unknown-unknown"

declare <8 x float> @llvm.x86.avx.vbroadcastss.256(ptr) nounwind

define <8 x float> @_Z4fabsDv8_f(<8 x float> %p) nounwind readnone {
  %1 = tail call <8 x float> @llvm.x86.avx.vbroadcastss.256(ptr undef) nounwind
  %2 = bitcast <8 x float> %p to <8 x i32>
  %3 = bitcast <8 x float> %1 to <8 x i32>
  %4 = and <8 x i32> %2, %3
  %5 = bitcast <8 x i32> %4 to <8 x float>
  ret <8 x float> %5
}
