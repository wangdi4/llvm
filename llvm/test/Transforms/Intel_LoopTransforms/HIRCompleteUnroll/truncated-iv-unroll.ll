; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-pre-vec-complete-unroll -hir-post-vec-complete-unroll -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>,hir-post-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that complete unroll finishes successfully.
; CanonExpr::replaceIVByConstant() utility was throwing an assertion during unroll because truncated iv value of 4 became 0 in 'i2' type.

; CHECK: Function: func

; CHECK: BEGIN REGION { }
; CHECK: + DO i1 = 0, 5, 1   <DO_LOOP>
; CHECK: |   %bf.set13 = -2 * trunc.i8.i2(%t7) * i1 + -1 * trunc.i8.i2(%t7)  |  4 * (%bf.set13 /u 4);
; CHECK: + END LOOP
; CHECK: END REGION

; CHECK: Function: func

; CHECK: BEGIN REGION { modified }
; CHECK: %bf.set13 = -1 * trunc.i8.i2(%t7)  |  4 * (%bf.set13 /u 4);
; CHECK: %bf.set13 = -3 * trunc.i8.i2(%t7)  |  4 * (%bf.set13 /u 4);
; CHECK: %bf.set13 = -5 * trunc.i8.i2(%t7)  |  4 * (%bf.set13 /u 4);
; CHECK: %bf.set13 = -7 * trunc.i8.i2(%t7)  |  4 * (%bf.set13 /u 4);
; CHECK: %bf.set13 = -1 * trunc.i8.i2(%t7)  |  4 * (%bf.set13 /u 4);
; CHECK: %bf.set13 = -3 * trunc.i8.i2(%t7)  |  4 * (%bf.set13 /u 4);
; CHECK: END REGION

define void @func(i8 %bf.set.lcssa16, i8 %t7) {
for.cond3.preheader:
  br label %for.body5

for.body5:                                        ; preds = %for.body5, %for.cond3.preheader
  %bf.set13 = phi i8 [ %bf.set.lcssa16, %for.cond3.preheader ], [ %bf.set, %for.body5 ]
  %storemerge12 = phi i32 [ 2, %for.cond3.preheader ], [ %add, %for.body5 ]
  %t8 = trunc i32 %storemerge12 to i8
  %t9 = mul i8 %t7, %t8
  %t10 = sub i8 %t7, %t9
  %bf.value = and i8 %t10, 3
  %bf.clear = and i8 %bf.set13, -4
  %bf.set = or i8 %bf.value, %bf.clear
  %add = add nuw nsw i32 %storemerge12, 2
  %cmp4 = icmp ult i32 %add, 14
  br i1 %cmp4, label %for.body5, label %for.inc8

for.inc8:                                         ; preds = %for.body5
  %bf.set.lcssa = phi i8 [ %bf.set, %for.body5 ]
  ret void
}
