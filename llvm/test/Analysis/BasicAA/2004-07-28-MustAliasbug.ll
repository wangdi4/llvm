<<<<<<< HEAD
; RUN: opt < %s -basicaa -dse -S | FileCheck %s
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basicaa -dse -S | FileCheck %s
=======
; RUN: opt < %s -basic-aa -dse -S | FileCheck %s
>>>>>>> feeed16a5f8127dde6ee01b023f1dbb20d203857

define void @test({i32,i32 }* %P) {
; CHECK: store i32 0, i32* %X
  %Q = getelementptr {i32,i32}, {i32,i32}* %P, i32 1
  %X = getelementptr {i32,i32}, {i32,i32}* %Q, i32 0, i32 1
  %Y = getelementptr {i32,i32}, {i32,i32}* %Q, i32 1, i32 1
  store i32 0, i32* %X
  store i32 1, i32* %Y
  ret void
}
