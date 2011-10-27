; RUN: llc -mcpu=haswell < %s

define <4 x i32> @shuf_0123(<8 x i32> %A) nounwind readnone {
  %1 = shufflevector <8 x i32> %A, <8 x i32> undef, <4 x i32> <i32 0, i32 2, i32 undef, i32 undef>
  ret <4 x i32> %1
  }

