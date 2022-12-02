<<<<<<< HEAD
; RUN: opt -basic-aa -instcombine -S < %s | FileCheck %s
=======
; RUN: opt -passes=instcombine -S < %s | FileCheck %s
>>>>>>> 881c6c0d46ae1b72fb60bbb6a547577f79a5d14f

; CHECK-LABEL: @test_load_combine_aa(
; CHECK: %[[V:.*]] = load i32, ptr %0
; CHECK: store i32 0, ptr %3
; CHECK: store i32 %[[V]], ptr %1
; CHECK: store i32 %[[V]], ptr %2
define void @test_load_combine_aa(ptr, ptr, ptr, ptr noalias) {
  %a = load i32, ptr %0
  store i32 0, ptr %3
  %b = load i32, ptr %0
  store i32 %a, ptr %1
  store i32 %b, ptr %2
  ret void
}
