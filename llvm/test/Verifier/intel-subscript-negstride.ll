; RUN: opt -passes="require<verify>" %s | FileCheck %s
; CHECK-NOT: error

; CMPLRLLVM-31836: Verifier was not accepting llvm.intel.subscript calls with
; negative stride.

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%complex_256bit = type { fp128, fp128 }
%complex_64bit = type { float, float }
%"TYPE_DEFS$.btSEQ" = type <{ float, [10 x [9 x float]], double, [10 x double], [12 x i8], fp128, [10 x fp128], %complex_64bit, [10 x [9 x %complex_64bit]], [10 x %complex_128bit], %complex_128bit, [8 x i8], %complex_256bit, [10 x %complex_256bit], [9 x i8], [10 x [9 x i8]], i32, [10 x [9 x i32]], i64, [1 x i8], [10 x [9 x [1 x i8]]], [10 x i64], [9 x i8], [10 x [9 x [9 x i8]]], i32, [10 x [9 x i32]], [1 x i8], [10 x [1 x i8]], i32, [10 x i32], [8 x i8] }>
%complex_128bit = type { double, double }
; Function Attrs: nounwind readnone speculatable
declare %"TYPE_DEFS$.btSEQ"* @"llvm.intel.subscript.p0s_TYPE_DEFS$.btSEQs.i64.i64.p0s_TYPE_DEFS$.btSEQs.i64"(i8, i64, i64, %"TYPE_DEFS$.btSEQ"*, i64) 

; Function Attrs: nounwind uwtable
define dso_local void @MAIN__() local_unnamed_addr  {
  %1 = inttoptr i64 undef to %"TYPE_DEFS$.btSEQ"*
  %2 = call %"TYPE_DEFS$.btSEQ"* @"llvm.intel.subscript.p0s_TYPE_DEFS$.btSEQs.i64.i64.p0s_TYPE_DEFS$.btSEQs.i64"(i8 0, i64 1, i64 -7584, %"TYPE_DEFS$.btSEQ"* elementtype(%"TYPE_DEFS$.btSEQ") %1, i64 0)
  ret void
}
