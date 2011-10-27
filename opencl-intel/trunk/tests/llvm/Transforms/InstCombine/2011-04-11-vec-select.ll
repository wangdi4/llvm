target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64"
target triple = "x86_64-pc-win32"

; RUN: opt -S -instcombine < %s | FileCheck %s

; CHECK: test
; CHECK-NOT: select
; CHECK: ret
define <4 x i32> @test(<4 x i32> %A, <4 x i32> %B) nounwind {
entry:
  %F = select <4 x i1> <i1 1, i1 1, i1 1, i1 1>, <4 x i32> %A, <4 x i32> %B
  ret <4 x i32> %F
}

; CHECK: test1
; CHECK-NOT: select
; CHECK: ret
define <4 x i32> @test1(<4 x i32> %A, <4 x i32> %B) nounwind {
entry:
  %F = select <4 x i1> <i1 0, i1 0, i1 0, i1 0>, <4 x i32> %A, <4 x i32> %B
  ret <4 x i32> %F
}

; CHECK: test2
; CHECK: select
; CHECK: ret
define <4 x i32> @test2(<4 x i32> %A, <4 x i32> %B) nounwind {
entry:
  %F = select <4 x i1> <i1 1, i1 0, i1 1, i1 1>, <4 x i32> %A, <4 x i32> %B
  ret <4 x i32> %F
}