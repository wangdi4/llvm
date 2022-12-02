<<<<<<< HEAD
; RUN: opt -basic-aa -gvn -S < %s | FileCheck %s
=======
; RUN: opt -passes=gvn -S < %s | FileCheck %s
>>>>>>> 881c6c0d46ae1b72fb60bbb6a547577f79a5d14f
; ModuleID = 'test3.ll'
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

define i32 @main(i32 *%foo)  {
entry:
; CHECK: load i32, i32* %foo, align 4
  %0 = load i32, i32* %foo, align 4
  store i32 5, i32* undef, align 4
; CHECK-NOT: load i32, i32* %foo, align 4
  %1 = load i32, i32* %foo, align 4
; CHECK: add i32 %0, %0
  %2 = add i32 %0, %1
  ret i32 %2
}
