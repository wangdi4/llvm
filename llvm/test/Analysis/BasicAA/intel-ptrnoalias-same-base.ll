<<<<<<< HEAD
; RUN: opt < %s -aa-pipeline=basic-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
=======
; RUN: opt -aa-pipeline="basic-aa" -passes="aa-eval" < %s -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
>>>>>>> a42a60ac617026fc2f3b5f8dd0b5fb8cc4bda333

; Check that %p and %q loaded from %a MayAlias even if loaded from "ptrnoalias" argument.

; CHECK-LABEL: foo
; CHECK-DAG: MustAlias: i32** %a, i32** %a1
; CHECK-DAG: MayAlias:	i32* %p, i32* %q

define void @foo(i32** "ptrnoalias" %a) {
  %p = load i32*, i32** %a
  %a1 = getelementptr i32*, i32** %a, i32 0
  %q = load i32*, i32** %a1
  %v1 = load i32, i32* %p
  %v2 = load i32, i32* %q
  ret void
}

