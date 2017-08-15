; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-complete-unroll-loopnest-trip-threshold=40 -print-before=hir-post-vec-complete-unroll -print-after=hir-post-vec-complete-unroll 2>&1 < %s | FileCheck %s

; Verify that we unroll both the inner loops in [8 x 8] matrix multiplication case.

; CHECK: Before HIR PostVec Complete Unroll

; CHECK: + DO i1 = 0, 7, 1   <DO_LOOP>
; CHECK: |   + DO i2 = 0, 7, 1   <DO_LOOP>
; CHECK: |   |   %.pre1593 = (@t_run_test.f_1)[0][i2][i1];
; CHECK: |   |
; CHECK: |   |   + DO i3 = 0, 7, 1   <DO_LOOP>
; CHECK: |   |   |   %1 = (%cosMatrixA)[0][i1][i3];
; CHECK: |   |   |   %2 = (@t_run_test.F_1)[0][i2][i3];
; CHECK: |   |   |   (@t_run_test.F_1)[0][i2][i3] = %2 + (sext.i8.i64(%.pre1593) * %1);
; CHECK: |   |   + END LOOP
; CHECK: |   + END LOOP
; CHECK: + END LOOP


; CHECK: After HIR PostVec Complete Unroll

; CHECK: BEGIN REGION { modified }
; CHECK: DO i1
; CHECK-NOT: DO i2
; CHECK-NOT: DO i3


@t_run_test.F_1 = internal unnamed_addr global [8 x [8 x i64]] zeroinitializer, align 16
@t_run_test.f_1 = internal unnamed_addr global [8 x [8 x i8]] zeroinitializer, align 1

define void @foo() {
entry:
  %cosMatrixA = alloca [8 x [8 x i64]], align 16
  br label %for.cond191.preheader

for.cond191.preheader:                            ; preds = %for.inc214, %entry
  %inc2151471 = phi i64 [ 0, %entry ], [ %inc215, %for.inc214 ]
  br label %for.cond195.preheader

for.cond195.preheader:                            ; preds = %for.inc211, %for.cond191.preheader
  %0 = phi i64 [ 0, %for.cond191.preheader ], [ %inc212, %for.inc211 ]
  %arrayidx200.phi.trans.insert = getelementptr inbounds [8 x [8 x i8]], [8 x [8 x i8]]* @t_run_test.f_1, i64 0, i64 %0, i64 %inc2151471
  %.pre1593 = load i8, i8* %arrayidx200.phi.trans.insert, align 1
  %conv201 = sext i8 %.pre1593 to i64
  br label %for.body198

for.body198:                                      ; preds = %for.body198, %for.cond195.preheader
  %inc2091468 = phi i64 [ 0, %for.cond195.preheader ], [ %inc209, %for.body198 ]
  %arrayidx203 = getelementptr inbounds [8 x [8 x i64]], [8 x [8 x i64]]* %cosMatrixA, i64 0, i64 %inc2151471, i64 %inc2091468
  %1 = load i64, i64* %arrayidx203, align 8
  %mul204 = mul nsw i64 %conv201, %1
  %arrayidx206 = getelementptr inbounds [8 x [8 x i64]], [8 x [8 x i64]]* @t_run_test.F_1, i64 0, i64 %0, i64 %inc2091468
  %2 = load i64, i64* %arrayidx206, align 8
  %add207 = add nsw i64 %2, %mul204
  store i64 %add207, i64* %arrayidx206, align 8
  %inc209 = add nuw nsw i64 %inc2091468, 1
  %exitcond = icmp eq i64 %inc209, 8
  br i1 %exitcond, label %for.inc211, label %for.body198

for.inc211:                                       ; preds = %for.body198
  %inc212 = add nuw nsw i64 %0, 1
  %exitcond1549 = icmp eq i64 %inc212, 8
  br i1 %exitcond1549, label %for.inc214, label %for.cond195.preheader

for.inc214:                                       ; preds = %for.inc211
  %inc215 = add nuw nsw i64 %inc2151471, 1
  %exitcond1550 = icmp eq i64 %inc215, 8
  br i1 %exitcond1550, label %exit, label %for.cond191.preheader

exit:
  ret void
}
