<<<<<<< HEAD
; RUN: opt < %s -basic-aa -dse -S | FileCheck %s
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basic-aa -dse -S | FileCheck %s
=======
; RUN: opt < %s -aa-pipeline=basic-aa -dse -S | FileCheck %s
>>>>>>> d3d84654467e03640a26fcae651a5d2d0e98541a

define void @test({i32,i32 }* %P) {
; CHECK: store i32 0, i32* %X
  %Q = getelementptr {i32,i32}, {i32,i32}* %P, i32 1
  %X = getelementptr {i32,i32}, {i32,i32}* %Q, i32 0, i32 1
  %Y = getelementptr {i32,i32}, {i32,i32}* %Q, i32 1, i32 1
  store i32 0, i32* %X
  store i32 1, i32* %Y
  ret void
}
