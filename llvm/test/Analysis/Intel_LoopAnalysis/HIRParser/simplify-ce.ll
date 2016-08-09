; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for the loop verifying that the subscript gets simplified by cancelling common factor of 4 from numerator and denominator.

; Original HIR-
; + DO i1 = 0, smax(3, %n) + -3, 1   <DO_LOOP>
; |   %0 = {al:4}(%p3)[i1 + 1];
; |   {al:4}(%p3)[i1] = %0;
; |   %1 = {al:4}(%p1)[(-4 * i1 + 4 * sext.i32.i64(%n) + -8)/u4];
; |   %2 = {al:4}(%p3)[i1 + 1];
; + END LOOP

; CHECK: + DO i1 = 0, smax(3, %n) + -3, 1   <DO_LOOP>
; CHECK: |   %0 = {al:4}(%p3)[i1 + 1];
; CHECK: |   {al:4}(%p3)[i1] = %0;
; CHECK: |   %1 = {al:4}(%p1)[-1 * i1 + sext.i32.i64(%n) + -2];
; CHECK: |   %2 = {al:4}(%p3)[i1 + 1];
; CHECK: + END LOOP

define i32 @t_run_test(i32* %p1, i32 %n, i32* %p3) {
entry:                             ; preds = %for.body70
  %add.ptr72 = getelementptr inbounds i32, i32* %p1, i32 %n
  %add.ptr73 = getelementptr inbounds i32, i32* %add.ptr72, i32 -1
  %incdec.ptr74 = getelementptr inbounds i32, i32* %add.ptr73, i32 -1
  %incdec.ptr75 = getelementptr inbounds i32, i32* %p3, i32 1
  br label %for.body80

for.body80:                                       ; preds = %entry, %for.body80
  %shr87321 = phi i32 [ %shr87, %for.body80 ], [ 0, %entry ]
  %incdec.ptr82320 = phi i32* [ %incdec.ptr82, %for.body80 ], [ %incdec.ptr74, %entry ]
  %incdec.ptr81319 = phi i32* [ %incdec.ptr81, %for.body80 ], [ %p3, %entry ]
  %incdec.ptr83318 = phi i32* [ %incdec.ptr83, %for.body80 ], [ %incdec.ptr75, %entry ]
  %i1.2317 = phi i32 [ %inc89, %for.body80 ], [ 2, %entry ]
  %0 = load i32, i32* %incdec.ptr83318, align 4
  %incdec.ptr81 = getelementptr inbounds i32, i32* %incdec.ptr81319, i32 1
  store i32 %0, i32* %incdec.ptr81319, align 4
  %incdec.ptr82 = getelementptr inbounds i32, i32* %incdec.ptr82320, i32 -1
  %1 = load i32, i32* %incdec.ptr82320, align 4
  %incdec.ptr83 = getelementptr inbounds i32, i32* %incdec.ptr83318, i32 1
  %2 = load i32, i32* %incdec.ptr83318, align 4
  %mul84 = mul nsw i32 %2, %1
  %add85 = add i32 %mul84, 4096
  %add86 = add i32 %add85, %shr87321
  %shr87 = ashr i32 %add86, 13
  %inc89 = add nuw nsw i32 %i1.2317, 1
  %cmp79 = icmp slt i32 %inc89, %n
  br i1 %cmp79, label %for.body80, label %for.cond78.for.end90_crit_edge

for.cond78.for.end90_crit_edge:
  ret i32 0
}
