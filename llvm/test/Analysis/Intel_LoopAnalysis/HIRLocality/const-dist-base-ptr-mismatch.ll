; REQUIRES: asserts

; RUN: opt < %s -passes='hir-ssa-deconstruction,print<hir-locality-analysis>' -hir-spatial-locality -debug-only=hir-locality-analysis -disable-output 2>&1 | FileCheck %s

; Verify that all 3 refs based on %base are in the same group and we are
; successfully able to compute distance between them. One of the refs
; has BasePtrElementType of i32 and the other two have i8.

; + DO i1 = 0, 10, 1   <DO_LOOP>
; |   (%base)[0] = 5;
; |   (%base)[0] = 2;
; |   %ld1 = (%base)[2];
; |   (%a)[i1] = %ld1;
; |   %base = &((%c)[0]);
; + END LOOP

; CHECK: Group 0 contains:
; CHECK:       (%base)[0]
; CHECK:       (%base)[0]
; CHECK:       (%base)[2]
; CHECK: Group 1 contains:
; CHECK:       (%a)[i1]

; CHECK: Locality Info for Loop level:

define void @foo(ptr noalias %a, ptr noalias %b, ptr noalias %c) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop]
  %base = phi ptr [ %b, %entry], [ %c, %loop]
  %iv.inc = add i64 %iv, 1
  store i32 5, ptr %base
  store i8 2, ptr %base
  %gep1 = getelementptr i8, ptr %base, i64 2
  %ld1 = load i8, ptr %gep1
  %gep3 = getelementptr i8, ptr %a, i64 %iv
  store i8 %ld1, ptr %gep3 
  %cmp = icmp eq i64 %iv, 10
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}
  
  
