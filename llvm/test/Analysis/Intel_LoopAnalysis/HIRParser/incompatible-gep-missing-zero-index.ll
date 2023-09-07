; RUN: opt -passes="hir-ssa-deconstruction,print<hir>" -hir-framework-debug=parser -hir-create-function-level-region -disable-output < %s 2>&1 | FileCheck %s

; Verify that we can recover a missing zero index between %gep1 and %gep2 which
; refers to the array field [2 x i32] access of the structure %struct and are
; able to combine the two GEPs into one.

; CHECK: BEGIN REGION { }
; CHECK: %ld = (%S)[3].0[1];
; CHECK: ret %ld;
; CHECK: END REGION

%struct = type { [2 x i32], i64 }

define i32 @foo(ptr %S) {
 entry:
  br label %bb

bb:
  %gep1 = getelementptr inbounds %struct, ptr %S, i64 3
  %gep2 = getelementptr inbounds i32, ptr %gep1, i64 1
  %ld = load i32, ptr %gep2
  ret i32 %ld
}
