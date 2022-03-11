; RUN: opt < %s -hir-ssa-deconstruction -analyze -enable-new-pm=0 -hir-framework -hir-create-function-level-region | FileCheck %s
; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-create-function-level-region 2>&1 | FileCheck %s

; Verify that we successfully parse the load which is a bitcast of a function
; pointer.
; The test was compfailing because we were trying to compute the size of
; function type which is not a sized type.

; CHECK: %ld = (i32*)(@bar)[0];

define i32 @foo() {
entry:
  br label %bb

bb:
  %ld = load i32, i32* bitcast (void ()* @bar to i32*), align 1
  ret i32 %ld
}

declare void @bar()
