; REQUIRES: asserts

; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-pre-vec-complete-unroll" -debug-only=hir-complete-unroll -disable-output 2>&1 < %s | FileCheck %s

; Verify that the cost of this loop is computed to be 16 which is (tripount * 2).
; Higher cost of 2 (vs regular cost of 1) is assigned to the call
; @llvm.maxnum.f32() as it is vectorizable and we are in prevec unroll. This is
; done to favor vectorization over unrolling. 

; CHECK: Cost: 16

; CHECK: + DO i1 = 0, 7, 1   <DO_LOOP>
; CHECK: |   %max = @llvm.maxnum.f32((%fptr)[i1],  0.000000e+00);
; CHECK: |   (%fptr)[i1] = %max;
; CHECK: + END LOOP


define void @foo(float* %fptr) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %gep = getelementptr float, ptr %fptr, i64 %iv
  %ld = load float, ptr %gep
  %max = tail call fast float @llvm.maxnum.f32(float %ld, float 0.000000e+00)
  store float %max, ptr %gep
  %iv.inc = add i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 8
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare float @llvm.maxnum.f32(float, float) #2

attributes #2 = { nofree nosync nounwind readnone speculatable willreturn }
