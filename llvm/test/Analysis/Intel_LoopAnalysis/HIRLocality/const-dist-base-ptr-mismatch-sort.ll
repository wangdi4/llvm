; REQUIRES: asserts

; RUN: opt < %s -passes='hir-ssa-deconstruction,print<hir-locality-analysis>' -hir-spatial-locality -debug-only=hir-locality-analysis -disable-output 2>&1 | FileCheck %s

; Verify that (i32*)((i8) %base)[4 * i1] and ((i32) %base)[i1 + 1] with
; different BasePtrElementType are put in the same locality group and
; (i32*)((i8) %base)[4 * i1] is inserted before ((i32) %base)[i1 + 1] even
; though it has a higher IV coefficient. This is because ((i32) %base)[i1 + 1]
; is converted into ((i8) %base)[4 * i1 + 4] for sorting and distance
; computation.


; HIR-
; + DO i1 = 0, 10, 1   <DO_LOOP>
; |   %ld1 = (i32*)(%base)[4 * i1];
; |   %ld2 = (%base)[i1 + 1];
; |   (i32*)(%a)[i1] = %ld1 + %ld2;
; |   %base = &((%c)[0]);
; + END LOOP

; CHECK: Group 0 contains:
; CHECK:         (i32*)(%base)[4 * i1]
; CHECK:         (%base)[i1 + 1]
; CHECK: Group 1 contains:
; CHECK:         (i32*)(%a)[i1]


define void @foo(ptr noalias %a, ptr noalias %b, ptr noalias %c) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop]
  %base = phi ptr [ %b, %entry], [ %c, %loop]
  %iv.inc = add i64 %iv, 1
  %mul = mul i64 %iv, 4
  %gep1 = getelementptr i8, ptr %base, i64 %mul
  %ld1 = load i32, ptr %gep1
  %gep2 = getelementptr i32, ptr %base, i64 %iv.inc
  %ld2 = load i32, ptr %gep2
  %gep3 = getelementptr i8, ptr %a, i64 %iv
  %add = add i32 %ld1, %ld2
  store i32 %add, ptr %gep3 
  %cmp = icmp eq i64 %iv, 10
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}
  
  
