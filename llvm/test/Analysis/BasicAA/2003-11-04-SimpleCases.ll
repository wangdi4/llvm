; This testcase consists of alias relations which should be completely
; resolvable by basicaa.

<<<<<<< HEAD
; RUN: opt < %s -basic-aa -aa-eval -print-may-aliases -disable-output 2>&1 | FileCheck %s
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basic-aa -aa-eval -print-may-aliases -disable-output 2>&1 | FileCheck %s
=======
; RUN: opt < %s -aa-pipeline=basic-aa -passes=aa-eval -print-may-aliases -disable-output 2>&1 | FileCheck %s
>>>>>>> b81d5baa0fb06b17e646e703c9771478ca190249

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%T = type { i32, [10 x i8] }

; CHECK:     Function: test
; CHECK-NOT:   MayAlias:

define void @test(%T* %P) {
  %A = getelementptr %T, %T* %P, i64 0
  %B = getelementptr %T, %T* %P, i64 0, i32 0
  %C = getelementptr %T, %T* %P, i64 0, i32 1
  %D = getelementptr %T, %T* %P, i64 0, i32 1, i64 0
  %E = getelementptr %T, %T* %P, i64 0, i32 1, i64 5
  ret void
}
