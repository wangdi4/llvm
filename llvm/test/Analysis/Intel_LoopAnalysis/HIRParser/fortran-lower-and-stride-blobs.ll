; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-framework -hir-framework-debug=parser -hir-details -hir-details-dims | FileCheck %s

; Verify that the load below contains blob ddrefs for the blobs %t17 and %t18 which are the lower and stride for the 2nd dimension.
; CHECK: %t21 = (%"ul_arg_0.0_GEPs_$field0$33")[0:i1:1(i32*:0)][%t17:%t:%t18(i32*:0)];
; CHECK: <RVAL-REG>
; CHECK: <BLOB> NON-LINEAR i64 %t18
; CHECK: <BLOB> NON-LINEAR i64 %t17

define void @func({ i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias %PART, i64 %t) {
bb60:
  %"ul_arg_0.0_GEPs_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %PART, i64 0, i32 0
  %"ul_arg_0.0_GEPs_$field6$_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %PART, i64 0, i32 6, i64 0, i32 1
  %t13 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"ul_arg_0.0_GEPs_$field6$_$field1$", i32 0)
  %t14 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"ul_arg_0.0_GEPs_$field6$_$field1$", i32 1)
  br label %bb64

bb64:                                             ; preds = %bb64, %bb60
  %indvars.iv2486 = phi i64 [ %indvars.iv.next2487, %bb64 ], [ 1, %bb60 ]
  %"ul_arg_0.0_GEPs_$field0$33" = load i32*, i32** %"ul_arg_0.0_GEPs_$field0$", align 8
  %t17 = load i64, i64* %t13, align 8
  %t18 = load i64, i64* %t14, align 8
  %t19 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 1, i32* %"ul_arg_0.0_GEPs_$field0$33", i64 %indvars.iv2486)
  %t20 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 %t17, i64 %t18, i32* %t19, i64 %t)
  %t21 = load i32, i32* %t20, align 4
  %indvars.iv.next2487 = add nuw nsw i64 %indvars.iv2486, 1
  %exitcond2488 = icmp eq i64 %indvars.iv.next2487, 10
  br i1 %exitcond2488, label %bb65, label %bb64

bb65:
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #1


