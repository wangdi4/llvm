; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,print<hir>,hir-dead-store-elimination,print<hir>" -hir-create-function-level-region 2>&1 < %s | FileCheck %s

; Check that the second store to (%i)[0] is eliminated but the first one is not
; due to aliasing with the load of (%c)[0]. The aliasing store of (%c)[0] is
; ignored.

;*** IR Dump Before HIR Dead Store Elimination ***
;Function: foo
;
; CHECK:      BEGIN REGION { }
; CHECK:            (%i)[0] = 1;
; CHECK:            %t = (%c)[0];
; CHECK:            (%i)[0] = 3;
; CHECK:            (%c)[0] = 1;
; CHECK:            (%i)[0] = 4;
; CHECK:            ret ;
; CHECK:      END REGION
;
;*** IR Dump After HIR Dead Store Elimination ***
;Function: foo
;
; CHECK-NOT:       (%i)[0] = 3;
;
define void @foo(ptr %c, ptr %i) {
entry:
  br label %bb

bb:
  store i32 1, ptr %i, align 4
  %t = load i8, ptr  %c, align 1
  store i32 3, ptr %i, align 4
  store i8 1, ptr %c, align 1
  store i32 4, ptr %i, align 4
  ret void
}


