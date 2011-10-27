; RUN: llc -mcpu=haswell < %s

define <4 x i64> @add_4_i64(<4 x i64> %A) nounwind readnone {
; CHECK: add_4_i64
; CHECK-NOT: extract
; CHECK: vpaddq
; CHECK-NOT: insert
  %1 = add <4 x i64> %A, <i64 0, i64 0, i64 0, i64 0>
  ret <4 x i64> %1
  }
define <8 x i32> @add_8_i32(<8 x i32> %A) nounwind readnone {
; CHECK: add_8_i32
; CHECK: vpaddd
  %1 = add <8 x i32> %A, <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %1
  }

define <32 x i8> @add_32_i8(<32 x i8> %A) nounwind readnone {
; CHECK: add_32_i8
; CHECK: vpaddb
  %1 = add <32 x i8> %A, <i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3 >
  ret <32 x i8> %1
  }

define <16 x i16> @add_16_i16(<16 x i16> %A) nounwind readnone {
; CHECK: add_16_i16
; CHECK-NOT: insert
; CHECK: vpaddw
  %1 = add <16 x i16> %A, <i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 8, i16 9, i16 2, i16 3>
  ret <16 x i16> %1
  }

define <4 x i64> @sub_4_i64(<4 x i64> %A) nounwind readnone {
; CHECK: sub_4_i64
; CHECK-NOT: extract
; CHECK: vpsubq
; CHECK-NOT: insert
  %1 = sub <4 x i64> %A, <i64 0, i64 0, i64 0, i64 0>
  ret <4 x i64> %1
  }
define <8 x i32> @sub_8_i32(<8 x i32> %A) nounwind readnone {
; CHECK: sub_8_i32
; CHECK: vpsubd
  %1 = sub <8 x i32> %A, <i32 0, i32 1, i32 2, i32 3, i32 0, i32 1, i32 2, i32 3>
  ret <8 x i32> %1
  }

define <32 x i8> @sub_32_i8(<32 x i8> %A) nounwind readnone {
; CHECK: sub_32_i8
; CHECK: vpsubb
  %1 = sub <32 x i8> %A, <i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 0, i8 1, i8 2, i8 3, i8 8, i8 9, i8 2, i8 3 >
  ret <32 x i8> %1
  }

define <16 x i16> @sub_16_i16(<16 x i16> %A) nounwind readnone {
; CHECK: sub_16_i16
; CHECK-NOT: insert
; CHECK: vpsubw
  %1 = sub <16 x i16> %A, <i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 0, i16 1, i16 2, i16 3, i16 8, i16 9, i16 2, i16 3>
  ret <16 x i16> %1
  }

