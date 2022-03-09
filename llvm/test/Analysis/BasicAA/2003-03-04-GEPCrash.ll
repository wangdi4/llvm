<<<<<<< HEAD
; RUN: opt < %s -basic-aa -aa-eval -disable-output 2>/dev/null
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basic-aa -aa-eval -disable-output 2>/dev/null
=======
; RUN: opt < %s -aa-pipeline=basic-aa -passes=aa-eval -disable-output 2>/dev/null
>>>>>>> b81d5baa0fb06b17e646e703c9771478ca190249
; Test for a bug in BasicAA which caused a crash when querying equality of P1&P2
define void @test({[2 x i32],[2 x i32]}* %A, i64 %X, i64 %Y) {
	%P1 = getelementptr {[2 x i32],[2 x i32]}, {[2 x i32],[2 x i32]}* %A, i64 0, i32 0, i64 %X
	%P2 = getelementptr {[2 x i32],[2 x i32]}, {[2 x i32],[2 x i32]}* %A, i64 0, i32 1, i64 %Y
	ret void
}
