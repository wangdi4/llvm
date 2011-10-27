; RUN: llc < %s -march=x86-64 -mcpu=corei7 | FileCheck %s

; Splat patterns below

define <4 x i32> @shl4(<4 x i32> %A) nounwind {
entry:
; CHECK:      pslld
; CHECK-NEXT: pslld
  %B = shl <4 x i32> %A,  < i32 2, i32 2, i32 2, i32 2>
  %C = shl <4 x i32> %A,  < i32 1, i32 1, i32 1, i32 1>
  %K = xor <4 x i32> %B, %C
  ret <4 x i32> %K
}

define <4 x i32> @shr4(<4 x i32> %A) nounwind {
entry:
; CHECK:      psrld
; CHECK-NEXT: psrld
  %B = lshr <4 x i32> %A,  < i32 2, i32 2, i32 2, i32 2>
  %C = lshr <4 x i32> %A,  < i32 1, i32 1, i32 1, i32 1>
  %K = xor <4 x i32> %B, %C
  ret <4 x i32> %K
}

define <4 x i32> @sra4(<4 x i32> %A) nounwind {
entry:
; CHECK:      psrad
; CHECK-NEXT: psrad
  %B = ashr <4 x i32> %A,  < i32 2, i32 2, i32 2, i32 2>
  %C = ashr <4 x i32> %A,  < i32 1, i32 1, i32 1, i32 1>
  %K = xor <4 x i32> %B, %C
  ret <4 x i32> %K
}

define <2 x i64> @shl2(<2 x i64> %A) nounwind {
entry:
; CHECK:      psllq
; CHECK-NEXT: psllq
  %B = shl <2 x i64> %A,  < i64 2, i64 2>
  %C = shl <2 x i64> %A,  < i64 9, i64 9>
  %K = xor <2 x i64> %B, %C
  ret <2 x i64> %K
}

define <2 x i64> @shr2(<2 x i64> %A) nounwind {
entry:
; CHECK:      psrlq
; CHECK-NEXT: psrlq
  %B = lshr <2 x i64> %A,  < i64 8, i64 8>
  %C = lshr <2 x i64> %A,  < i64 1, i64 1>
  %K = xor <2 x i64> %B, %C
  ret <2 x i64> %K
}


define <8 x i16> @shl8(<8 x i16> %A) nounwind {
entry:
; CHECK:      psllw
; CHECK-NEXT: psllw
  %B = shl <8 x i16> %A,  < i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2>
  %C = shl <8 x i16> %A,  < i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %K = xor <8 x i16> %B, %C
  ret <8 x i16> %K
}

define <8 x i16> @shr8(<8 x i16> %A) nounwind {
entry:
; CHECK:      psrlw
; CHECK-NEXT: psrlw
  %B = lshr <8 x i16> %A,  < i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2>
  %C = lshr <8 x i16> %A,  < i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %K = xor <8 x i16> %B, %C
  ret <8 x i16> %K
}

define <8 x i16> @sra8(<8 x i16> %A) nounwind {
entry:
; CHECK:      psraw
; CHECK-NEXT: psraw
  %B = ashr <8 x i16> %A,  < i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2, i16 2>
  %C = ashr <8 x i16> %A,  < i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %K = xor <8 x i16> %B, %C
  ret <8 x i16> %K
}

; non splat test


define <8 x i16> @sll8_nosplat(<8 x i16> %A) nounwind {
entry:
; CHECK: sll8_nosplat
; CHECK-NOT: psll
; CHECK-NOT: psll
; CHECK: ret
  %B = shl <8 x i16> %A,  < i16 1, i16 2, i16 3, i16 6, i16 2, i16 2, i16 2, i16 2>
  %C = shl <8 x i16> %A,  < i16 9, i16 7, i16 5, i16 1, i16 4, i16 1, i16 1, i16 1>
  %K = xor <8 x i16> %B, %C
  ret <8 x i16> %K
}


define <2 x i64> @shr2_nosplat(<2 x i64> %A) nounwind {
entry:
; CHECK: shr2_nosplat
; CHECK-NOT:  psrlq
; CHECK-NOT:  psrlq
; CHECK: ret
  %B = lshr <2 x i64> %A,  < i64 8, i64 1>
  %C = lshr <2 x i64> %A,  < i64 1, i64 0>
  %K = xor <2 x i64> %B, %C
  ret <2 x i64> %K
}

; Check for missing codegen patterns #0
define <4 x i8> @sll_missing_patterns0(<4 x i8> %A) nounwind {
entry: 
  %B = shl <4 x i8> %A,  < i8 0, i8 2, i8 3, i8 6>
  %C = shl <4 x i8> %B,  < i8 9, i8 7, i8 5, i8 1>
  %D = shl <4 x i8> %C,  < i8 1, i8 1, i8 1, i8 1>
  %E = shl <4 x i8> %D,  < i8 1, i8 1, i8 1, i8 1>
  %K = xor <4 x i8> %E, %C
  ret <4 x i8> %K
}

; Check for missing codegen patterns #1
define <8 x i16> @sll_missing_patterns1(<8 x i16>* %Ap) nounwind {
entry: 
  %A = load <8 x i16>* %Ap
  %B = shl <8 x i16> %A,  < i16 0, i16 2, i16 3, i16 6, i16 2, i16 2, i16 2, i16 2>
  %C = shl <8 x i16> %B,  < i16 9, i16 7, i16 5, i16 1, i16 4, i16 1, i16 1, i16 1>
  %D = shl <8 x i16> %C,  < i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %E = shl <8 x i16> %D,  < i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1, i16 1>
  %K = xor <8 x i16> %E, %C
  ret <8 x i16> %K
}

; Check for missing codegen patterns #3
define <4 x i16> @sll_missing_patterns3(<4 x i16>* %Ap) nounwind {
entry: 
  %A = load <4 x i16>* %Ap
  %B = shl <4 x i16> %A,  < i16 0, i16 2, i16 3, i16 6>
  %C = shl <4 x i16> %B,  < i16 9, i16 7, i16 5, i16 1>
  %D = shl <4 x i16> %C,  < i16 1, i16 1, i16 1, i16 1>
  %E = shl <4 x i16> %D,  < i16 1, i16 1, i16 1, i16 1>
  %K = xor <4 x i16> %E, %C
  ret <4 x i16> %K
}
