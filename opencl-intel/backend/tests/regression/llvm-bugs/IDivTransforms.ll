; RUN: llvm-as %s -o %t.bc
; RUN: opt -instcombine %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll
; CHECK: ret
; This is a test which reproduce the failure in commonIDivTransforms, when you have the second operand as constant expression
; ticket # : CSSD100005306

define <2 x i32> @test(<2 x i32> %a) nounwind {
  %b = sdiv <2 x i32> %a, <i32 bitcast (<1 x i32> <i32 0> to i32), i32 bitcast (<1 x i32> <i32 0> to i32)> ; <<2 x i32>> [#uses=1]
  ret <2 x i32> %b
}
