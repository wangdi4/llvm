<<<<<<< HEAD
; RUN: opt < %s -basicaa -aa-eval -disable-output 2>/dev/null
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basicaa -aa-eval -disable-output 2>/dev/null
=======
; RUN: opt < %s -basic-aa -aa-eval -disable-output 2>/dev/null
>>>>>>> feeed16a5f8127dde6ee01b023f1dbb20d203857

define i32 @MTConcat([3 x i32]* %a.1) {
	%tmp.961 = getelementptr [3 x i32], [3 x i32]* %a.1, i64 0, i64 4
	%tmp.97 = load i32, i32* %tmp.961
	%tmp.119 = getelementptr [3 x i32], [3 x i32]* %a.1, i64 1, i64 0
	%tmp.120 = load i32, i32* %tmp.119
	%tmp.1541 = getelementptr [3 x i32], [3 x i32]* %a.1, i64 0, i64 4
	%tmp.155 = load i32, i32* %tmp.1541
	ret i32 0
}
