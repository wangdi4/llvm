; RUN: llc < %s -march=x86-64 -mcpu=haswel | FileCheck %s

; Splat patterns below

define <8 x i32> @shl8(<8 x i32> %A) nounwind {
entry:
; CHECK: shl8
; CHECK: vpslld
; CHECK: vpslld

  %B = shl <8 x i32> %A,  < i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %C = shl <8 x i32> %A,  < i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %K = xor <8 x i32> %B, %C
  ret <8 x i32> %K
}

define <8 x i32> @shr8(<8 x i32> %A) nounwind {
entry:
; CHECK: shr8
; CHECK: vpsrld
; CHECK: vpsrld
; CHECK-NOT: vpsrld
  %B = lshr <8 x i32> %A,  < i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %C = lshr <8 x i32> %A,  < i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %K = xor <8 x i32> %B, %C
  ret <8 x i32> %K
}

define <8 x i32> @sra8xi32(<8 x i32> %A) nounwind {
entry:
; CHECK: sra8xi32
; CHECK: vpsrad
; CHECK: vpsrad
; CHECK-NOT: vpsrad
  %B = ashr <8 x i32> %A,  < i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2, i32 2>
  %C = ashr <8 x i32> %A,  < i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1, i32 1>
  %K = xor <8 x i32> %B, %C
  ret <8 x i32> %K
}

define <4 x i64> @shl4xi64(<4 x i64> %A) nounwind {
entry:
; CHECK: shl4xi64
; CHECK: vpsllq
; CHECK: vpsllq
; CHECK-NOT: vpsllq
  %B = shl <4 x i64> %A,  < i64 2, i64 2, i64 2, i64 2>
  %C = shl <4 x i64> %A,  < i64 9, i64 9, i64 9, i64 9>
  %K = xor <4 x i64> %B, %C
  ret <4 x i64> %K
}

define <4 x i64> @shr4xi64(<4 x i64> %A) nounwind {
entry:
; CHECK: shr4xi64
; CHECK: vpsrlq
; CHECK: vpsrlq
; CHECK-NOT: vpsrlq
  %B = lshr <4 x i64> %A,  < i64 8, i64 8, i64 8, i64 8>
  %C = lshr <4 x i64> %A,  < i64 1, i64 1, i64 1, i64 1>
  %K = xor <4 x i64> %B, %C
  ret <4 x i64> %K
}


define <16 x i16> @shl16xi16(<16 x i16> %A) nounwind {
entry:
; CHECK: shl16xi16
; CHECK: vpsllw
; CHECK: vpsllw
  %B = shl <16 x i16> %A,  < i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2>
  %C = shl <16 x i16> %A,  < i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %K = xor <16 x i16> %B, %C
  ret <16 x i16> %K
}

define <16 x i16> @shr8xi16(<16 x i16> %A) nounwind {
entry:
; CHECK: shr8xi16
; CHECK: vpsrlw
; CHECK: vpsrlw
  %B = lshr <16 x i16> %A,  < i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2 >
  %C = lshr <16 x i16> %A,  < i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1 >
  %K = xor <16 x i16> %B, %C
  ret <16 x i16> %K
}

define <16 x i16> @sra16xi16(<16 x i16> %A) nounwind {
entry:
; CHECK: sra16xi16
; CHECK: vpsraw
; CHECK: vpsraw
  %B = ashr <16 x i16> %A,  < i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2>
  %C = ashr <16 x i16> %A,  < i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %K = xor <16 x i16> %B, %C
  ret <16 x i16> %K
}

; non splat test


define <16 x i16> @sll_16xi16_nosplat(<16 x i16> %A) nounwind {
entry:
; CHECK: sll8_nosplat
; CHECK: vpsllvd
; CHECK: vpsllvd
; CHECK: vpsllvd
; CHECK: vpsllvd
; CHECK: ret
  %B = shl <16 x i16> %A,  < i16 1, i16 2, i16 3, i16 6, i16 2, i16 2, i16 2, i16 2, i16 1, i16 2, i16 3, i16 6, i16 2, i16 2, i16 2, i16 2>
  %C = shl <16 x i16> %A,  < i16 9, i16 7, i16 5, i16 1, i16 4, i16 1, i16 1, i16 1, i16 9, i16 7, i16 5, i16 1, i16 4, i16 1, i16 1, i16 1>
  %K = xor <16 x i16> %B, %C
  ret <16 x i16> %K
}


define <4 x i64> @shr2_nosplat(<4 x i64> %A) nounwind {
entry:
; CHECK: shr2_nosplat
; CHECK:  vpsrlvq
; CHECK:  vpsrlvq
; CHECK: vpxor
; CHECK: ret
  %B = lshr <4 x i64> %A,  < i64 8, i64 1, i64 3, i64 2>
  %C = lshr <4 x i64> %A,  < i64 1, i64 0, i64 2, i64 3>
  %K = xor <4 x i64> %B, %C
  ret <4 x i64> %K
}

; Check for missing codegen patterns #0
define <8 x i8> @sll_8xi8_nosplat(<8 x i8> %A) nounwind {
entry: 
  %B = shl <8 x i8> %A,  < i8 0, i8 2, i8 3, i8 6, i8 0, i8 2, i8 3, i8 6>
  ret <8 x i8> %B
}

; Check for missing codegen patterns #1
define <8 x i16> @sll_8xi16_nosplat(<8 x i16>* %Ap) nounwind {
entry: 
; CHECK:  vpsllvd
  %A = load <8 x i16>* %Ap
  %B = shl <8 x i16> %A,  < i16 0, i16 2, i16 3, i16 6, i16 2, i16 2, i16 2, i16 2>
  %C = xor <8 x i16> %A, %B
  ret <8 x i16> %C
}

