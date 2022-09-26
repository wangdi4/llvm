; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir>" -hir-create-function-level-region -hir-details 2>&1 -disable-output | FileCheck %s

; Verify that parser does not crete fake refs for calls with 
; 'inaccessiblememonly' attribute as the call does not 
; access memory through its operands.

; CHECK: @llvm.assume(-1); [ align(&((%ptr)[0])64) ]
; CHECK: <RVAL-REG> &((LINEAR i32* %ptr)[i64 0])
; CHECK-NOT: FAKE-LVAL-REG

define void @foo(i32* %ptr) {
entry:
  br label %bb

bb:
  call void @llvm.assume(i1 true) [ "align"(i32* %ptr, i64 64) ]
  ret void
}

declare void @llvm.assume(i1 noundef) #1

attributes #1 = { inaccessiblememonly }
