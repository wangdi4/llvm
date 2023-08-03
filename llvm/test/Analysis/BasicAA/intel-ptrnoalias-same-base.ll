; RUN: opt -aa-pipeline="basic-aa" -passes="aa-eval" < %s -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; Check that %p and %q loaded from %a MayAlias even if loaded from "ptrnoalias" argument.

; CHECK-LABEL: foo
; CHECK-DAG: MustAlias: ptr* %a, ptr* %a1
; CHECK-DAG: MayAlias:	i32* %p, i32* %q

define void @foo(ptr "ptrnoalias" %a) {
  %p = load ptr, ptr %a
  %a1 = getelementptr ptr, ptr %a, i32 0
  %q = load ptr, ptr %a1
  %v1 = load i32, ptr %p
  %v2 = load i32, ptr %q
  ret void
}

