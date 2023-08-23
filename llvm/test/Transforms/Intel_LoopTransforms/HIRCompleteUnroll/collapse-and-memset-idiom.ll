; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll,print<hir>" 2>&1 < %s | FileCheck %s

; Verify that the inner i3-i4 loopnest is not unrolled by pre-vec unroller
; because the entire perfect loopnest is identified as collapse + memset idiom.

; Unrolling perfect loopnests partially doesn't yield much profitability as no
; redundant loads/stores can be exposed between sibling loopnests.

; CHECK: + DO i1 = 0, 3, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 14, 1   <DO_LOOP>
; CHECK: |   |   + DO i3 = 0, 27, 1   <DO_LOOP>
; CHECK: |   |   |   + DO i4 = 0, 18, 1   <DO_LOOP>
; CHECK: |   |   |   |   (%i79)[0][i1][i2][i3][i4] = 0.000000e+00;
; CHECK: |   |   |   + END LOOP
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP

define void @foo() {
entry:
  %i79 = alloca [4 x [15 x [28 x [19 x double]]]], align 32
  br label %loop1

loop1:                                           ; preds = %latch1, %entry
  %i4186 = phi i64 [ 1, %entry ], [ %i4183, %latch1 ]
  %i4187 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 63840, ptr elementtype(double) nonnull %i79, i64 %i4186)
  br label %loop2

loop2:                                           ; preds = %loop1, %latch2
  %i4180 = phi i64 [ 1, %loop1 ], [ %i4177, %latch2 ]
  %i4181 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 4256, ptr elementtype(double) nonnull %i4187, i64 %i4180)
  br label %loop3

loop3:                                           ; preds = %loop2, %latch3
  %i4174 = phi i64 [ 0, %loop2 ], [ %i4171, %latch3 ]
  %i4175 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 0, i64 152, ptr elementtype(double) nonnull %i4181, i64 %i4174)
  br label %loop4

loop4:                                           ; preds = %loop3, %loop4
  %i4166 = phi i64 [ 1, %loop3 ], [ %i4168, %loop4 ]
  %i4167 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %i4175, i64 %i4166)
  store double 0.000000e+00, ptr %i4167, align 1
  %i4168 = add nuw nsw i64 %i4166, 1
  %i4169 = icmp eq i64 %i4168, 20
  br i1 %i4169, label %latch3, label %loop4

latch3:                                           ; preds = %loop4
  %i4171 = add nuw nsw i64 %i4174, 1
  %i4172 = icmp eq i64 %i4171, 28
  br i1 %i4172, label %latch2, label %loop3

latch2:                                           ; preds = %latch3
  %i4177 = add nuw nsw i64 %i4180, 1
  %i4178 = icmp eq i64 %i4177, 16
  br i1 %i4178, label %latch1, label %loop2

latch1:                                           ; preds = %latch2
  %i4183 = add nuw nsw i64 %i4186, 1
  %i4184 = icmp eq i64 %i4183, 5
  br i1 %i4184, label %exit, label %loop1

exit:
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #2 = { nounwind readnone speculatable }
