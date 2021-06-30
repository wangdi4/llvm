; RUN: opt -hir-ssa-deconstruction -hir-mv-variable-stride  -print-after=hir-mv-variable-stride -hir-details-dims -disable-output  <%s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-mv-variable-stride,print<hir>" -hir-details-dims <%s 2>&1 | FileCheck %s

; Test checks that HIR Multiversioning for Variable Stride process both fake and addressOf operands in function call.

; Original HIR
;<0>          BEGIN REGION { }
;<29>               + DO i1 = 0, %x + -1, 1   <DO_LOOP>
;<2>                |   if (%x >= 1)
;<2>                |   {
;<30>               |      + DO i2 = 0, %x + -1, 1   <DO_LOOP>
;<13>               |      |   @llvm.memcpy.p0i8.p0i8.i64(&((%B)[0:i1:%l(i8*:0)][0:i2:%n(i8*:0)]),  &((%A)[0:i1:%k(i8*:0)][0:i2:%m(i8*:0)]),  %y,  0);
;<30>               |      + END LOOP
;<2>                |   }
;<29>               + END LOOP
;<0>          END REGION


; *** IR Dump After HIR Multiversioning for variable stride (hir-mv-variable-stride) ***
; CHECK:      BEGIN REGION { }
; CHECK:            if (%n == 1 && %m == 1)
; CHECK:            {
; CHECK:               + DO i1 = 0, %x + -1, 1   <DO_LOOP>
;                      |   if (%x >= 1)
;                      |   {
;                      |      + DO i2 = 0, %x + -1, 1   <DO_LOOP>
; CHECK:               |      |   @llvm.memcpy.p0i8.p0i8.i64(&((%B)[0:i1:%l(i8*:0)][0:i2:1(i8*:0)]),  &((%A)[0:i1:%k(i8*:0)][0:i2:1(i8*:0)]),  %y,  0);
;                      |      + END LOOP
;                      |   }
; CHECK:               + END LOOP
; CHECK:            }
; CHECK:            else
; CHECK:            {
; CHECK:               + DO i1 = 0, %x + -1, 1   <DO_LOOP>
;                      |   if (%x >= 1)
;                      |   {
;                      |      + DO i2 = 0, %x + -1, 1   <DO_LOOP>
; CHECK:               |      |   @llvm.memcpy.p0i8.p0i8.i64(&((%B)[0:i1:%l(i8*:0)][0:i2:%n(i8*:0)]),  &((%A)[0:i1:%k(i8*:0)][0:i2:%m(i8*:0)]),  %y,  0);
;                      |      + END LOOP
;                      |   }
; CHECK:               + END LOOP
; CHECK:            }
; CHECK:      END REGION



; Function Attrs: norecurse nounwind uwtable
define dso_local void @foo(i8* nocapture %A, i8* nocapture readonly %B, i64 %k, i64 %l, i64 %m, i64 %n, i64 %x, i64 %y) local_unnamed_addr {
entry:
  %cmp3 = icmp slt i64 %x, 1
  %ub1 = add nsw i64 %x, 1
  %ub2 = add nuw nsw i64 %x, 1
  br label %outerloop

outerloop:                                              ; preds = %incbb, %entry
  %iv1 = phi i64 [ 1, %entry ], [ %iv1_inc, %incbb ]
  br i1 %cmp3, label %incbb, label %innerloop.pre

innerloop.pre:                                          ; preds = %outerloop
  %A_fetch = call i8* @llvm.intel.subscript.p0i8.i64.i64.p0i8.i64(i8 2, i64 1, i64 %k, i8* elementtype(i8) %A, i64 %iv1)
  %B_fetch = call i8* @llvm.intel.subscript.p0i8.i64.i64.p0i8.i64(i8 2, i64 1, i64 %l, i8* elementtype(i8) nonnull %B, i64 %iv1)
  br label %innerloop

innerloop:                                              ; preds = %innerloop.pre, %innerloop
  %iv2 = phi i64 [ 1, %innerloop.pre ], [ %iv2_inc, %innerloop ]
  %A_fetch_fetch = call i8* @llvm.intel.subscript.p0i8.i64.i64.p0i8.i64(i8 1, i64 1, i64 %m, i8* elementtype(i8) %A_fetch, i64 %iv2)
  %B_fetch_fetch = call i8* @llvm.intel.subscript.p0i8.i64.i64.p0i8.i64(i8 1, i64 1, i64 %n, i8* elementtype(i8) nonnull %B_fetch, i64 %iv2)
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 1 %B_fetch_fetch, i8* align 1 %A_fetch_fetch, i64 %y, i1 false)
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

declare i8* @llvm.intel.subscript.p0i8.i64.i64.p0i8.i64(i8, i64, i64, i8*, i64)

declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)
