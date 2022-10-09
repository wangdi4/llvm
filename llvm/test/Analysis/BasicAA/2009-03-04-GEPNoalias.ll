<<<<<<< HEAD
; RUN: opt < %s -basic-aa -gvn -S | FileCheck %s
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basic-aa -gvn -S | FileCheck %s
=======
; RUN: opt < %s -aa-pipeline=basic-aa -gvn -S | FileCheck %s
>>>>>>> d3d84654467e03640a26fcae651a5d2d0e98541a

declare noalias i32* @noalias()

define i32 @test(i32 %x) {
; CHECK: load i32, i32* %a
  %a = call i32* @noalias()
  store i32 1, i32* %a
  %b = getelementptr i32, i32* %a, i32 %x
  store i32 2, i32* %b

  %c = load i32, i32* %a
  ret i32 %c
}
