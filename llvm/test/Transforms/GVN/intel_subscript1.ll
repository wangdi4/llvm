; RUN: opt -basic-aa -gvn -S < %s 2>&1 | FileCheck %s
; AA and GVN of memrefs from constant-index subscripts should work.
; %add36 should be propagated to the mul, in place of the load.

; CHECK: %mul {{.*}} %add36

define void @func({ i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* noalias nocapture readonly %A) local_unnamed_addr  {
alloca:
  %"A_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %A, i64 0, i32 0
  %"A_$field0$1" = load i32*, i32** %"A_$field0$", align 8
    %0 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 4, i32* %"A_$field0$1", i64 2)
  %1= tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 5, i32* %0, i64 2)
  %2 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 6, i32* %1, i64 2)
  %3 = load i32, i32* %2, align 4
  %add36 = add nsw i32 %3, 11
  %4 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 4, i32* %"A_$field0$1", i64 1)
  %5= tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 5, i32* %4, i64 1)
  %6 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 6, i32* %5, i64 1)
  store i32 %add36, i32* %6, align 4
  %add43 = add nsw i32 %3, 233
  %7 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 4, i32* %"A_$field0$1", i64 3)
  %8 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 5, i32* %7, i64 3)
  %9 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 6, i32* %8, i64 3)
  store i32 %add43, i32* %9, align 4
  %10 = load i32, i32* %6, align 4
  %mul = shl i32 %10, 1
  %add53 = add nsw i32 %mul, %add43
  %11 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 4, i32* %"A_$field0$1", i64 5)
  %12 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 5, i32* %11, i64 5)
  %13 = tail call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 6, i32* %12, i64 5)
  store i32 %add53, i32* %13, align 4
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #2

declare i32 @for_write_seq_lis_xmit(i8*, i8*, i8*) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #2

attributes #2 = { nounwind readnone speculatable }
