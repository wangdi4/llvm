target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

define <8 x float> @_Z4fabsDv8_f(<8 x float> %p) nounwind readnone {
  %1 = tail call <8 x float> @llvm.x86.avx.vbroadcastss.256(i8* bitcast ([4 x i32]* @mth_signMask to i8*)) nounwind
  %2 = bitcast <8 x float> %p to <8 x i32>
  %3 = bitcast <8 x float> %1 to <8 x i32>
  %4 = and <8 x i32> %2, %3
  %5 = bitcast <8 x i32> %4 to <8 x float>
  ret <8 x float> %5
}
