; RUN: opt -passes="hir-ssa-deconstruction,print<hir>,hir-loop-reversal,print<hir>" -hir-loop-reversal-assume-profitability -aa-pipeline="basic-aa" -S 2>&1 < %s  | FileCheck %s

; Verify that we give up on reversing the loop by checking for dd edges between call instruction's fake ddrefs.

; CHECK: Function: foo

; CHECK: + DO i1 = 0, 15, 1   <DO_LOOP>
; CHECK: |   @llvm.memcpy.p0.p0.i64(&((%dst)[16 * i1]),  &((%src)[16 * i1]),  16,  0);
; CHECK: + END LOOP

; CHECK: Function: foo


; CHECK-NOT: modified


define void @foo(ptr %src, ptr %dst) {
entry:
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.inc, %loop ]
  %srcphi = phi ptr [ %src, %entry ], [ %srcinc, %loop ]
  %dstphi = phi ptr [ %dst, %entry ], [ %dstinc, %loop ]
  tail call void @llvm.memcpy.p0.p0.i64(ptr nonnull %dstphi, ptr nonnull %srcphi, i64 16, i1 false)
  %srcinc = getelementptr inbounds i8, ptr %srcphi, i64 16
  %dstinc = getelementptr inbounds i8, ptr %dstphi, i64 16
  %iv.inc = add nsw i64 %iv, 1
  %cmp = icmp eq i64 %iv.inc, 16
  br i1 %cmp, label %exit, label %loop

exit:
  ret void
}

declare void @llvm.memcpy.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #1
attributes #1 = { argmemonly nounwind }
