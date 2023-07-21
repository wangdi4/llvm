; RUN: opt -passes='argpromotion,cgscc(inline)' -inline-report=0xf859 < %s -S 2>&1 | FileCheck %s 
; Inline report via metadata
; RUN: opt -passes='inlinereportsetup,argpromotion,cgscc(inline),inlinereportemitter' -inline-report=0xf8d8 -S < %s 2>&1 | FileCheck %s

; CMPLRLLVM-32526: Check that the inlining report specified with -qopt-report=3
; does not seg fault when argument promotion is performed before inlining.

; CHECK: Begin Inlining Report
; CHECK: Option Values:
; CHECK: inline-threshold: 225
; CHECK: inlinehint-threshold: 325
; CHECK: inlinecold-threshold: 45
; CHECK: inlineoptsize-threshold: 15
; CHECK: DEAD STATIC FUNC: test
; CHECK: COMPILE FUNC: caller
; CHECK: INLINE: test <stdin>
; CHECK: End Inlining Report

%T = type { i32, i32, i32, i32 }
@G = constant %T { i32 0, i32 0, i32 17, i32 25 }

define internal i32 @test(ptr %p) {
entry:
  %a.gep = getelementptr %T, ptr %p, i64 0, i32 3
  %b.gep = getelementptr %T, ptr %p, i64 0, i32 2
  %a = load i32, ptr %a.gep
  %b = load i32, ptr %b.gep
  %v = add i32 %a, %b
  ret i32 %v
}

define i32 @caller() {
entry:
  %v = call i32 @test(ptr @G)
  ret i32 %v
}
