; RUN: llc -mcpu=haswell < %s

define <4 x i64> @unpack_hi_32(<4 x i64> %m1, <4 x i64> %m2) nounwind readnone {
entry:
; CHECK: unpack_hi_32
; CHECK: vpunpckhdq
  %0 = bitcast <4 x i64> %m1 to <8 x i32>
  %1 = bitcast <4 x i64> %m2 to <8 x i32>
  %shuffle.i = shufflevector <8 x i32> %0, <8 x i32> %1, <8 x i32> <i32 4, i32 12, i32 5, i32 13, i32 6, i32 14, i32 7, i32 15>
  %2 = bitcast <8 x i32> %shuffle.i to <4 x i64>
  ret <4 x i64> %2
}

define <4 x i64> @unpack_lo_32(<4 x i64> %m1, <4 x i64> %m2) nounwind readnone {
entry:
; CHECK: unpack_lo_32
; CHECK: vpunpckldq
  %0 = bitcast <4 x i64> %m1 to <8 x i32>
  %1 = bitcast <4 x i64> %m2 to <8 x i32>
  %shuffle.i = shufflevector <8 x i32> %0, <8 x i32> %1, <8 x i32> <i32 0, i32 8, i32 1, i32 9, i32 2, i32 10, i32 3, i32 11>
  %2 = bitcast <8 x i32> %shuffle.i to <4 x i64>
  ret <4 x i64> %2
}

define <4 x i64> @unpack_hi_16(<4 x i64> %m1, <4 x i64> %m2) nounwind readnone {
entry:
; CHECK: unpack_hi_16
; CHECK: vpunpckhwd
  %0 = bitcast <4 x i64> %m1 to <16 x i16>
  %1 = bitcast <4 x i64> %m2 to <16 x i16>
  %shuffle.i = shufflevector <16 x i16> %0, <16 x i16> %1, <16 x i32> <i32 8, i32 24, i32 9, i32 25, i32 10, i32 26, i32 11, i32 27, i32 12, i32 28, i32 13, i32 29, i32 14, i32 30, i32 15, i32 31>
  %2 = bitcast <16 x i16> %shuffle.i to <4 x i64>
  ret <4 x i64> %2
}

define <4 x i64> @unpack_lo_16(<4 x i64> %m1, <4 x i64> %m2) nounwind readnone {
entry:
; CHECK: unpack_lo_16
; CHECK: vpunpcklwd
  %0 = bitcast <4 x i64> %m1 to <16 x i16>
  %1 = bitcast <4 x i64> %m2 to <16 x i16>
  %shuffle.i = shufflevector <16 x i16> %0, <16 x i16> %1, <16 x i32> <i32 0, i32 16, i32 1, i32 17, i32 2, i32 18, i32 3, i32 19, i32 4, i32 20, i32 5, i32 21, i32 6, i32 22, i32 7, i32 23>
  %2 = bitcast <16 x i16> %shuffle.i to <4 x i64>
  ret <4 x i64> %2
}

define <4 x i64> @unpack_hi_8(<4 x i64> %m1, <4 x i64> %m2) nounwind readnone {
entry:
; CHECK: unpack_hi_8
; CHECK: vpunpckhbw
  %0 = bitcast <4 x i64> %m1 to <32 x i8>
  %1 = bitcast <4 x i64> %m2 to <32 x i8>
  %shuffle.i = shufflevector <32 x i8> %0, <32 x i8> %1, <32 x i32> <i32 16, i32 48, i32 17, i32 49, i32 18, i32 50, i32 19, i32 51, i32 20, i32 52, i32 21, i32 53, i32 22, i32 54, i32 23, i32 55, i32 24, i32 56, i32 25, i32 57, i32 26, i32 58, i32 27, i32 59, i32 28, i32 60, i32 29, i32 61, i32 30, i32 62, i32 31, i32 63>
  %2 = bitcast <32 x i8> %shuffle.i to <4 x i64>
  ret <4 x i64> %2
}

define <4 x i64> @unpack_lo_8(<4 x i64> %m1, <4 x i64> %m2) nounwind readnone {
entry:
; CHECK: unpack_lo_8
; CHECK: vpunpcklbw
  %0 = bitcast <4 x i64> %m1 to <32 x i8>
  %1 = bitcast <4 x i64> %m2 to <32 x i8>
  %shuffle.i = shufflevector <32 x i8> %0, <32 x i8> %1, <32 x i32> <i32 0, i32 32, i32 1, i32 33, i32 2, i32 34, i32 3, i32 35, i32 4, i32 36, i32 5, i32 37, i32 6, i32 38, i32 7, i32 39, i32 8, i32 40, i32 9, i32 41, i32 10, i32 42, i32 11, i32 43, i32 12, i32 44, i32 13, i32 45, i32 14, i32 46, i32 15, i32 47>
  %2 = bitcast <32 x i8> %shuffle.i to <4 x i64>
  ret <4 x i64> %2
}

define <4 x i64> @unpack_hi_64(<4 x i64> %m1, <4 x i64> %m2) nounwind readnone {
entry:
; CHECK: unpack_hi_64
; CHECK: vpunpckhqdq
  %shuffle.i = shufflevector <4 x i64> %m1, <4 x i64> %m2, <4 x i32> <i32 2, i32 6, i32 3, i32 7>
  ret <4 x i64> %shuffle.i
}

define <4 x i64> @unpack_lo_64(<4 x i64> %m1, <4 x i64> %m2) nounwind readnone {
entry:
; CHECK: unpack_lo_64
; CHECK: vpunpcklqdq
  %shuffle.i = shufflevector <4 x i64> %m1, <4 x i64> %m2, <4 x i32> <i32 0, i32 4, i32 1, i32 5>
  ret <4 x i64> %shuffle.i
}
