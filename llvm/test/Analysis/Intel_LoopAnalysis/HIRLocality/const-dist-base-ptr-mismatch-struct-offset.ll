; REQUIRES: asserts

; RUN: opt < %s -passes='hir-ssa-deconstruction,print<hir-locality-analysis>' -hir-spatial-locality -debug-only=hir-locality-analysis -disable-output 2>&1 | FileCheck %s

; Verify that all 3 refs based on %b are in the same group and we are
; successfully able to compute distance between them. One of the refs
; has BasePtrElementType of i32 and the other two have a struct type.
; One of the struct type ref also has a trailing offset.

; + DO i1 = 0, 10, 1   <DO_LOOP>
; |   %ld1 = (i8*)(%b)[1];
; |   %ld2 = (i8*)(%b)[2].1;
; |   %ld3 = (i8*)(%b)[3];
; |   (%a)[i1] = %ld1 + %ld2 + %ld3;
; + END LOOP

; CHECK: Group 0 contains:
; CHECK:       (i8*)(%b)[1] {sb:17}
; CHECK:       (i8*)(%b)[3] {sb:19}
; CHECK:       (i8*)(%b)[2].1 {sb:18}

; CHECK: Group 1 contains:
; CHECK:       (%a)[i1]

; CHECK: Locality Info for Loop level:

%struct = type { i32, i32 }

define void @foo(ptr noalias %a, ptr noalias %b) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop]
  %iv.inc = add i64 %iv, 1
  %gep1 = getelementptr %struct, ptr %b, i64 1
  %ld1 = load i8, ptr %gep1
  %gep2 = getelementptr %struct, ptr %b, i64 2, i32 1
  %ld2 = load i8, ptr %gep2
  %gep3 = getelementptr i32, ptr %b, i64 3
  %ld3 = load i8, ptr %gep3
  %add1 = add i8 %ld1 , %ld2
  %add2 = add i8 %add1 , %ld3
  %gep4 = getelementptr i8, ptr %a, i64 %iv
  store i8 %add2, ptr %gep4
  %cmp = icmp eq i64 %iv, 10
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}
  
  
