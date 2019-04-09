; RUN: opt -S -loop-rotate < %s | FileCheck %s
; RUN: opt -S -passes='require<targetir>,require<assumptions>,loop(rotate)' < %s | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @foo(i64* %p) {
entry:
  br label %bb145

bb145:
  %"var$19.0" = phi i64 [ 7, %entry ], [ %add32, %bb140 ]
  %"var$18.0" = phi i64 [ 1, %entry ], [ %add34, %bb140 ]
  %rel25 = icmp ult i64 %"var$18.0", 3
  br i1 %rel25, label %bb140, label %exit

bb140:                                            ; preds = %bb145
  %i = tail call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 2, i64 1, i64 324, i64* %p, i64 2)
  %j = tail call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 1, i64 1, i64 36, i64* %i, i64 1)
  %k = tail call i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8 0, i64 1, i64 4, i64* %j, i64 %"var$19.0")

  %v = load i64, i64* %k, align 4
  %v1 = add nsw i64 %v, 1
  store i64 %v1, i64* %p, align 4

  %add32 = add nuw nsw i64 %"var$19.0", 1
  %add34 = add nuw nsw i64 %"var$18.0", 1
; CHECK: icmp ult i64 %add34, 3
  br label %bb145

exit:
  ret void
}

declare i64* @llvm.intel.subscript.p0i64.i64.i64.p0i64.i64(i8, i64, i64, i64*, i64) #1

attributes #1 = { nounwind readnone speculatable }

