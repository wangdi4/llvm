target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; RUN: opt -S -instcombine < %s | FileCheck %s

; CHECK: func
; CHECK: extractelement
; CHECK: zext i16
; CHECK: ret

define float* @func(float* %p, <4 x i16>* %data) {
  %F = load <4 x i16>* %data
  %Y = zext <4 x i16> %F to <4 x i32>
  %T0 = extractelement <4 x i32> %Y, i32 0
  %J0 = getelementptr float* %p, i32 %T0
  %T1 = extractelement <4 x i32> %Y, i32 0
  %J1 = getelementptr float* %p, i32 %T1
  ret float* %J0
}


; CHECK: func2
; CHECK: extractelement
; CHECK: sext
; CHECK-NOT: sext <
; CHECK: ret
define float* @func2(float* %p, <4 x i32>* %data) {
  %F = load <4 x i32>* %data
  %Y = sext <4 x i32> %F to <4 x i64>
  %T0 = extractelement <4 x i64> %Y, i32 0
  %J0 = getelementptr float* %p, i64 %T0
  %T1 = extractelement <4 x i64> %Y, i32 0
  %J1 = getelementptr float* %p, i64 %T1
  ret float* %J0
}

; CHECK: func3
; CHECK-NOT: = trunc
; CHECK: ret
define i16 @func3(i1 %s, i32 %r) {
  %T = extractelement <4 x i32><i32 8, i32 2, i32 0, i32 4>, i32 %r
  %F = select i1 %s, i32 %T, i32 1
  %K = trunc i32 %F to i16
  ret i16 %K
}

; CHECK: func4
; CHECK-NOT: trunc
; CHECK: ret
define <4 x i16> @func4(i1 %s, i32 %r) {
  %V = select i1 %s, i32 1, i32 2
  %T = insertelement <4 x i32><i32 1, i32 2, i32 3, i32 4>, i32 %V, i32 0
  %F = select i1 %s, <4 x i32> %T, <4 x i32><i32 8, i32 2, i32 0, i32 5>
  %K = trunc <4 x i32> %F to <4 x i16>
  ret <4 x i16> %K
}

; CHECK: func5
; CHECK-NOT: trunc
; CHECK: ret
define <4 x i16> @func5(i1 %s, i32 %r, <4 x i16> %t, <4 x i32>* %TT) {
  %V = select i1 %s, i32 1, i32 2
  %F = select i1 %s, <4 x i16><i16 1, i16 2, i16 3, i16 0> , <4 x i16><i16 5, i16 2, i16 1, i16 0>
  %Y = zext <4 x i16> %F to <4 x i32>  
  %T = insertelement <4 x i32> %Y, i32 %V, i32 0
  %S = select i1 %s, <4 x i32> %T, <4 x i32><i32 8, i32 2, i32 0, i32 0>
  %K = trunc <4 x i32> %S to <4 x i16>
  ret <4 x i16> %K
}

; CHECK: func6
; CHECK-NOT: = zext
; CHECK: ret
define i64 @func6(i1 %s, i32 %r, <4 x i16> %t, <4 x i32>* %TT) {
  %F = select i1 %s, <4 x i16><i16 1, i16 2, i16 3, i16 0> , <4 x i16><i16 5, i16 2, i16 1, i16 0>
  %Y = zext <4 x i16> %F to <4 x i32>  
  %T = extractelement <4 x i32> %Y, i32 0
  %S = select i1 %s, i32 %T, i32 0
  %K = zext i32 %S to i64
  ret i64 %K
}


