<<<<<<< HEAD
; RUN: opt < %s -basicaa -aa-eval -disable-output 2>/dev/null
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basicaa -aa-eval -disable-output 2>/dev/null
=======
; RUN: opt < %s -basic-aa -aa-eval -disable-output 2>/dev/null
>>>>>>> feeed16a5f8127dde6ee01b023f1dbb20d203857
; Test for a bug in BasicAA which caused a crash when querying equality of P1&P2
define void @test({[2 x i32],[2 x i32]}* %A, i64 %X, i64 %Y) {
	%P1 = getelementptr {[2 x i32],[2 x i32]}, {[2 x i32],[2 x i32]}* %A, i64 0, i32 0, i64 %X
	%P2 = getelementptr {[2 x i32],[2 x i32]}, {[2 x i32],[2 x i32]}* %A, i64 0, i32 1, i64 %Y
	ret void
}
