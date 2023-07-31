; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-variable-stride,print<hir>" -hir-details-dims <%s 2>&1 | FileCheck %s -check-prefix=ORIG
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-mv-variable-stride" -print-changed -disable-output <%s 2>&1 | FileCheck %s -check-prefix=CHECK-CHANGED

; Test checks that HIR Multiversioning for Variable Stride doesn't happen when there is a function call in the loop.

; Original HIR
;<0>          BEGIN REGION { }
;<29>               + DO i1 = 0, %x + -1, 1   <DO_LOOP>
;<30>               |   + DO i2 = 0, %x + -1, 1   <DO_LOOP>
;<13>               |   |   @llvm.memcpy.p0.p0.i64(&((%B)[0:i1:%l(i8:0)][0:i2:%n(i8:0)]),  &((%A)[0:i1:%k(i8:0)][0:i2:%m(i8:0)]),  %y,  0);
;<30>               |   + END LOOP
;<29>               + END LOOP
;<0>          END REGION

; ORIG: Function: foo
; ORIG:     BEGIN REGION { }
; ORIG:           + DO i1 = 0, %x + -1, 1   <DO_LOOP>
; ORIG:           |   + DO i2 = 0, %x + -1, 1   <DO_LOOP>
; ORIG:           |   |   @llvm.memcpy.p0.p0.i64(&((%B)[0:i1:%l(i8:0)][0:i2:%n(i8:0)]),  &((%A)[0:i1:%k(i8:0)][0:i2:%m(i8:0)]),  %y,  0);
; ORIG:           |   + END LOOP
; ORIG:           + END LOOP
; ORIG:     END REGION

; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-variable-stride,print<hir>" -hir-mv-allow-fake-refs -hir-details-dims <%s 2>&1 | FileCheck %s -check-prefix=OPT

; Test checks that HIR Multiversioning for Variable Stride happens if we explicitly allowed a function call in the loop.

; OPT: Function: foo
; OPT:     BEGIN REGION { }
; OPT:             if (%n == 1 & %m == 1)
; OPT:             {
; OPT:                + DO i1 = 0, %x + -1, 1   <DO_LOOP>
; OPT:                |   + DO i2 = 0, %x + -1, 1   <DO_LOOP>
; OPT:                |   |   @llvm.memcpy.p0.p0.i64(&((%B)[0:i1:%l(i8:0)][0:i2:1(i8:0)]),  &((%A)[0:i1:%k(i8:0)][0:i2:1(i8:0)]),  %y,  0);
; OPT:                |   + END LOOP
; OPT:                + END LOOP
; OPT:             }
; OPT:             else
; OPT:             {
; OPT:                + DO i1 = 0, %x + -1, 1   <DO_LOOP>
; OPT:                |   + DO i2 = 0, %x + -1, 1   <DO_LOOP>
; OPT:                |   |   @llvm.memcpy.p0.p0.i64(&((%B)[0:i1:%l(i8:0)][0:i2:%n(i8:0)]),  &((%A)[0:i1:%k(i8:0)][0:i2:%m(i8:0)]),  %y,  0);
; OPT:                |   + END LOOP
; OPT:                + END LOOP
; OPT:             }

; Verify that pass is not dumped with print-changed if it bails out.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED-NOT: Dump After HIRMVForVariableStride

; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(ptr nocapture %A, ptr nocapture readonly %B, i64 %k, i64 %l, i64 %m, i64 %n, i64 %x, i64 %y) local_unnamed_addr {
entry:
  %cmp3 = icmp slt i64 %x, 1
  %ub1 = add nsw i64 %x, 1
  %ub2 = add nuw nsw i64 %x, 1
  br label %outerloop

outerloop:                                              ; preds = %incbb, %entry
  %iv1 = phi i64 [ 1, %entry ], [ %iv1_inc, %incbb ]
  br i1 %cmp3, label %incbb, label %innerloop.pre

innerloop.pre:                                          ; preds = %outerloop
  %A_fetch = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %k, ptr elementtype(i8) %A, i64 %iv1)
  %B_fetch = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %l, ptr elementtype(i8) nonnull %B, i64 %iv1)
  br label %innerloop

innerloop:                                              ; preds = %innerloop.pre, %innerloop
  %iv2 = phi i64 [ 1, %innerloop.pre ], [ %iv2_inc, %innerloop ]
  %A_fetch_fetch = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %m, ptr elementtype(i8) %A_fetch, i64 %iv2)
  %B_fetch_fetch = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %n, ptr elementtype(i8) nonnull %B_fetch, i64 %iv2)
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 1 %B_fetch_fetch, ptr align 1 %A_fetch_fetch, i64 %y, i1 false)
  %iv2_inc = add nuw nsw i64 %iv2, 1
  %cmp2 = icmp eq i64 %iv2_inc, %ub1
  br i1 %cmp2, label %bb, label %innerloop

bb:
  br label %incbb

incbb:                                              ; preds = %bb, %outerloop
  %iv1_inc = add nuw nsw i64 %iv1, 1
  %cmp1 = icmp eq i64 %iv1_inc, %ub2
  br i1 %cmp1, label %exit, label %outerloop

exit:                                              ; preds = %incbb
  ret void
}

declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64)

declare void @llvm.memcpy.p0.p0.i64(ptr, ptr, i64, i1)
