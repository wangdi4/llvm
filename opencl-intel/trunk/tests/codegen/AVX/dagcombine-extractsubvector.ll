; RUN: llc -mcpu=sandybridge < %s | FileCheck %s

define <4 x i32> @test1(<4 x i32> %v1) nounwind readonly {
; CHECK: test1:
; CHECK-NOT: vinsert
; CHECK: ret
  %1 = shufflevector <4 x i32> %v1, <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %2 = shufflevector <8 x i32> %1, <8 x i32> undef, <4 x i32> <i32 0, i32 1, i32 2, i32 3>
  ret <4 x i32> %2
}

define <4 x i32> @test2(<4 x i32> %v1) nounwind readonly {
; CHECK: test2:
; CHECK-NOT: vinsert
; CHECK: ret
  %1 = shufflevector <4 x i32> %v1, <4 x i32> undef, <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
  %2 = shufflevector <8 x i32> undef, <8 x i32> %1, <4 x i32> <i32 4, i32 5, i32 6, i32 7>
  ret <4 x i32> %2
}
