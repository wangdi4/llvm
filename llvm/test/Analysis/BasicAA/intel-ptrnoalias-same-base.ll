; RUN: opt < %s -basicaa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; Check that %p and %q loaded from %a MayAlias even if loaded from "ptrnoalias" argument.

; CHECK-LABEL: foo
; CHECK-DAG: MayAlias:     i32* %p, i32* %q

define void @foo(i32** "ptrnoalias" %a) {
  %p = load i32*, i32** %a
  %q = load i32*, i32** %a
  ret void
}

