; RUN: opt < %s -hir-details-dims -hir-ssa-deconstruction -hir-framework -analyze | FileCheck %s
; RUN: opt < %s -hir-details-dims -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that the following subscripts will create two dimensions even if they have same stride.

; integer A(1,3)
; A = 5
; end

; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 2, 1   <DO_LOOP>
; CHECK:       |   (@"_unnamed_main$$_$A")[0:0:12([3 x [1 x i32]]*:0)][0:i1:4([3 x [1 x i32]]:3)][0:0:4([1 x i32]:1)] = 5;
; CHECK:       + END LOOP
; CHECK: END REGION

@"_unnamed_main$$_$A" = internal unnamed_addr global [3 x [1 x i32]] zeroinitializer, align 16
@0 = internal unnamed_addr constant i32 2

define void @MAIN__() {
alloca:
  %0 = tail call i32 @for_set_reentrancy(i32* nonnull @0)
  br label %bb17

bb17:                                             ; preds = %alloca, %bb17
  %"var$3.016" = phi i64 [ 1, %alloca ], [ %add11, %bb17 ]
  %1 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 4, i32* getelementptr inbounds ([3 x [1 x i32]], [3 x [1 x i32]]* @"_unnamed_main$$_$A", i64 0, i64 0, i64 0), i64 %"var$3.016")
  %2 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* %1, i64 1)
  store i32 5, i32* %2, align 4
  %add11 = add nuw nsw i64 %"var$3.016", 1
  %exitcond = icmp eq i64 %add11, 4
  br i1 %exitcond, label %bb1, label %bb17

bb1:                                              ; preds = %bb17
  ret void
}

declare i32 @for_set_reentrancy(i32*) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1

attributes #1 = { nounwind readnone speculatable }

