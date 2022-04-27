; RUN: opt -hir-ssa-deconstruction -analyze -hir-framework -enable-new-pm=0 -hir-details-dims < %s  2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,print<hir-framework>" -disable-output -hir-details-dims < %s 2>&1 | FileCheck %s

; Verify that the test passes and MAX_TC_EST of inner loop is 3.
; The test case was compfailing because the lower dimension stride of 16 for the
; loads did not evenly divide higher dimension stride of 40 which was being
; asserted by the code.

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, %n + -2, 1   <DO_LOOP>  <MAX_TC_EST = 3>
; CHECK: |   |   %t29 = (getelementptr inbounds ([5 x [5 x %simple_type]], [5 x [5 x %simple_type]]* @B, i64 0, i64 0, i64 0))[0:i1:40(%simple_type*:0)][0:i2:16(%simple_type*:3)].0;
; CHECK: |   |   (%ptr)[0:i1:24(%simple_type*:0)][0:i2:8(%simple_type*:3)].0 = %t29;
; CHECK: |   |   %t32 = (getelementptr inbounds ([5 x [5 x %simple_type]], [5 x [5 x %simple_type]]* @B, i64 0, i64 0, i64 0))[0:i1:40(%simple_type*:0)][0:i2:16(%simple_type*:3)].1;
; CHECK: |   |   (%ptr)[0:i1:24(%simple_type*:0)][0:i2:8(%simple_type*:3)].1 = %t32;
; CHECK: |   + END LOOP
; CHECK: + END LOOP


%"simple_type" = type <{ i32, float }>

@B = external hidden global [5 x [5 x %"simple_type"]]

define void @foo(%"simple_type"* %ptr, i64 %n) {
entry:
  br label %outer.loop

outer.loop:                           ; preds = %loop_exit139, %entry
  %iv.outer = phi i64 [ 1, %entry ], [ %add.45, %loop_exit139 ]
  %B.gep1 = call %"simple_type"* @"llvm.intel.subscript.p0s_simple_types.i64.i64.p0s_simple_types.i64"(i8 1, i64 1, i64 40, %"simple_type"* elementtype(%"simple_type") getelementptr inbounds ([5 x [5 x %"simple_type"]], [5 x [5 x %"simple_type"]]* @B, i64 0, i64 0, i64 0), i64 %iv.outer)
  %ptr.gep1 = call %"simple_type"* @"llvm.intel.subscript.p0s_simple_types.i64.i64.p0s_simple_types.i64"(i8 1, i64 1, i64 24, %"simple_type"* nonnull elementtype(%"simple_type") %ptr, i64 %iv.outer)
  br label %inner.loop

inner.loop:                                     ; preds = %outer.loop, %inner.loop
  %iv.inner = phi i64 [ 1, %outer.loop ], [ %add.43, %inner.loop ]
  %B.gep2 = call %"simple_type"* @"llvm.intel.subscript.p0s_simple_types.i64.i64.p0s_simple_types.i64"(i8 0, i64 1, i64 16, %"simple_type"* elementtype(%"simple_type") %B.gep1, i64 %iv.inner)
  %ptr.gep2 = call %"simple_type"* @"llvm.intel.subscript.p0s_simple_types.i64.i64.p0s_simple_types.i64"(i8 0, i64 1, i64 8, %"simple_type"* nonnull elementtype(%"simple_type") %ptr.gep1, i64 %iv.inner)
  %t27 = getelementptr inbounds %"simple_type", %"simple_type"* %B.gep2, i64 0, i32 0
  %t28 = getelementptr inbounds %"simple_type", %"simple_type"* %ptr.gep2, i64 0, i32 0
  %t29 = load i32, i32* %t27, align 1
  store i32 %t29, i32* %t28, align 1
  %t30 = getelementptr inbounds %"simple_type", %"simple_type"* %B.gep2, i64 0, i32 1
  %t31 = getelementptr inbounds %"simple_type", %"simple_type"* %ptr.gep2, i64 0, i32 1
  %t32 = load float, float* %t30, align 1
  store float %t32, float* %t31, align 1
  %add.43 = add nuw nsw i64 %iv.inner, 1
  %exitcond263 = icmp eq i64 %add.43, %n
  br i1 %exitcond263, label %loop_exit139, label %inner.loop

loop_exit139:                                     ; preds = %inner.loop
  %add.45 = add nuw nsw i64 %iv.outer, 1
  %exitcond264 = icmp eq i64 %add.45, 5
  br i1 %exitcond264, label %exit, label %outer.loop

exit:
  ret void
}

declare %"simple_type"* @"llvm.intel.subscript.p0s_simple_types.i64.i64.p0s_simple_types.i64"(i8, i64, i64, %"simple_type"*, i64) #3

attributes #3 = { nounwind readnone speculatable }
