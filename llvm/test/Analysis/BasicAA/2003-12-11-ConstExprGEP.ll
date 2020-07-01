; This testcase consists of alias relations which should be completely
; resolvable by basicaa, but require analysis of getelementptr constant exprs.

<<<<<<< HEAD
; RUN: opt < %s -basicaa -aa-eval -print-may-aliases -disable-output 2>&1 | FileCheck %s
; INTEL
; RUN: opt -convert-to-subscript -S < %s | opt -basicaa -aa-eval -print-may-aliases -disable-output 2>&1 | FileCheck %s
=======
; RUN: opt < %s -basic-aa -aa-eval -print-may-aliases -disable-output 2>&1 | FileCheck %s
>>>>>>> feeed16a5f8127dde6ee01b023f1dbb20d203857

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%T = type { i32, [10 x i8] }

@G = external global %T

; CHECK:     Function: test
; CHECK-NOT:   MayAlias:

define void @test() {
  %D = getelementptr %T, %T* @G, i64 0, i32 0
  %E = getelementptr %T, %T* @G, i64 0, i32 1, i64 5
  %F = getelementptr i32, i32* getelementptr (%T, %T* @G, i64 0, i32 0), i64 0
  %X = getelementptr [10 x i8], [10 x i8]* getelementptr (%T, %T* @G, i64 0, i32 1), i64 0, i64 5

  ret void
}
