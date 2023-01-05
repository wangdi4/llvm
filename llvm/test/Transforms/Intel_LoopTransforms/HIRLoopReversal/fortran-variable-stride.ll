; RUN: opt -passes="hir-ssa-deconstruction,hir-loop-reversal,print<hir>" -aa-pipeline="basic-aa" -S 2>&1 < %s  | FileCheck %s
;
; This test case is original from fortran code. It encountered arithmetic exception before the change because the ref does not have a constant stride for the dimension and get 0 for the stride leading to arithmetic exception in AccumuWeightPosIVs computation.
;
;*** IR Dump Before HIR Loop Reversal ***
;Function: foo
;
;<0>          BEGIN REGION { }
;<16>               + DO i1 = 0, 4, 1   <DO_LOOP>
;<6>                |   %t20 = (%"foo_$A_$field0$1")[-1 * i1 + 9][20];
;<7>                |   (%"foo_$A_$field0$1")[i1 + 5][17] = %t20;
;<16>               + END LOOP
;<0>          END REGION
;
;*** IR Dump After HIR Loop Reversal ***
;CHECK: DO i1
;
define void @foo (i32* nonnull %"foo_$A_$field0$1", { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias nocapture readonly %"foo_$A" ) {

bb119:
  %"foo_$A_$field6$_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"foo_$A", i64 0, i32 6, i64 0, i32 1
  %t0 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"foo_$A_$field6$_$field1$", i32 0)
  %t1 = load i64, i64* %t0, align 8
  %t2 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) nonnull %"foo_$A_$field6$_$field1$", i32 1)
  %t3 = load i64, i64* %t2, align 8

br label %bb172

bb172:                                            ; preds = %bb172, %bb119
  %"var$23.0305" = phi i64 [ 5, %bb119 ], [ %add77, %bb172 ]
  %"var$24.0304" = phi i64 [ 9, %bb119 ], [ %add75, %bb172 ]
  %t16 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 5, i64 %t3, i32* elementtype(i32) nonnull %"foo_$A_$field0$1", i64 %"var$24.0304")
  %t17 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 17, i64 %t1, i32* elementtype(i32) %t16, i64 20)
  %t18 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 5, i64 %t3, i32* elementtype(i32) nonnull %"foo_$A_$field0$1", i64 %"var$23.0305")
  %t19 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 17, i64 %t1, i32* elementtype(i32) %t18, i64 17)
  %t20 = load i32, i32* %t17, align 4
  store i32 %t20, i32* %t19, align 4
  %add75 = add nsw i64 %"var$24.0304", -1
  %add77 = add nuw nsw i64 %"var$23.0305", 1
  %exitcond319 = icmp eq i64 %add77, 10
  br i1 %exitcond319, label %bb182, label %bb172

bb182:
  ret void
}

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1
