; This testcase makes sure that size is taken to account when alias analysis 
; is performed.  It is not legal to delete the second load instruction because
; the value computed by the first load instruction is changed by the store.

<<<<<<< HEAD
; RUN: opt < %s -basicaa -gvn -instcombine -S | FileCheck %s
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basicaa -gvn -instcombine -S | FileCheck %s
=======
; RUN: opt < %s -basic-aa -gvn -instcombine -S | FileCheck %s
>>>>>>> feeed16a5f8127dde6ee01b023f1dbb20d203857

define i32 @test() {
; CHECK: %Y.DONOTREMOVE = load i32, i32* %A
; CHECK: %Z = sub i32 0, %Y.DONOTREMOVE
  %A = alloca i32
  store i32 0, i32* %A
  %X = load i32, i32* %A
  %B = bitcast i32* %A to i8*
  %C = getelementptr i8, i8* %B, i64 1
  store i8 1, i8* %C    ; Aliases %A
  %Y.DONOTREMOVE = load i32, i32* %A
  %Z = sub i32 %X, %Y.DONOTREMOVE
  ret i32 %Z
}

