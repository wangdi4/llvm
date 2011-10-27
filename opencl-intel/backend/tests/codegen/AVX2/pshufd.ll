; RUN: llc -mcpu=haswell < %s

define <8 x i32> @pshufd1(<8 x i32> %A) nounwind readnone {
entry:
; CHECK: pshufd1
; CHECK: vpshufd $108
; mask 01 10 11 00 = 108,d
  %B = shufflevector <8 x i32> %A, <8 x i32> undef, <8 x i32> <i32 0, i32 3, i32 2, i32 1, i32 4, i32 7, i32 6, i32 5>
  ret <8 x i32> %B
}


define <8 x i32> @pshufd2(<8 x i32> %A) nounwind readnone {
entry:
; CHECK: pshufd2
; CHECK: vpshufd $-84
; mask 10 10 11 00 = -84,d
  %B = shufflevector <8 x i32> %A, <8 x i32> undef, <8 x i32> <i32 undef, i32 3, i32 2, i32 2, i32 4, i32 undef, i32 6, i32 6>
  ret <8 x i32> %B
}

define <16 x i16> @pshuf_hw(<16 x i16> %A) nounwind readnone {
entry:
; CHECK: pshuf_hw
; CHECK: vpshufhw        $56
; mask 4, 6, 7, 4, = 00 11 10 00 = 0x38 = 56,d
  %B = shufflevector <16 x i16> %A, <16 x i16> undef, <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 6, i32 7, i32 4, i32 8, i32 9, i32 10, i32 undef, i32 12, i32 14, i32 undef, i32 12>
  ret <16 x i16> %B
}

define <16 x i16> @pshuf_lw(<16 x i16> %A) nounwind readnone {
entry:
; CHECK: pshuf_lw
; CHECK: vpshuflw        
  %B = shufflevector <16 x i16> %A, <16 x i16> undef, <16 x i32> <i32 1, i32 1, i32 3, i32 3, i32 4, i32 5, i32 6, i32 7, i32 9, i32 9, i32 11, i32 undef, i32 12, i32 13, i32 undef, i32 15>
  ret <16 x i16> %B
}