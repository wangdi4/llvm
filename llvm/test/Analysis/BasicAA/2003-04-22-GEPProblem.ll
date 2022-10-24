<<<<<<< HEAD
; RUN: opt < %s -aa-pipeline=basic-aa -gvn -instcombine -S | FileCheck %s
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basic-aa -gvn -instcombine -S | FileCheck %s
=======
; RUN: opt < %s -aa-pipeline=basic-aa -passes=gvn,instcombine -S | FileCheck %s
>>>>>>> ec9ccb1668f60ae29e2f6c9627142f5ebfe15080

; BasicAA was incorrectly concluding that P1 and P2 didn't conflict!

define i32 @test(i32 *%Ptr, i64 %V) {
; CHECK: sub i32 %X, %Y
  %P2 = getelementptr i32, i32* %Ptr, i64 1
  %P1 = getelementptr i32, i32* %Ptr, i64 %V
  %X = load i32, i32* %P1
  store i32 5, i32* %P2
  %Y = load i32, i32* %P1
  %Z = sub i32 %X, %Y
  ret i32 %Z
}
