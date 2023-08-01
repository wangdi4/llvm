; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-output 2>&1 | FileCheck %s

; Check %p should not be merged into subscript chain - subscript chain has incompatible outer dimension.

; CHECK:     BEGIN REGION { }
; CHECK:           + DO i1 = 0, 0, 1   <DO_LOOP>
; CHECK:           |   %p = &((%A)[0][0]);
; CHECK:           |   (%p)[10][11][12] = 1.000000e+00;
; CHECK:           + END LOOP
; CHECK:     END REGION

source_filename = "ld-temp.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

define hidden fastcc void @foo(ptr %A) unnamed_addr #1 {
entry:
  br label %bb83

bb83:
  %p = getelementptr inbounds [4 x double], ptr %A, i64 0, i64 0
  %p0 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 32, ptr elementtype(double) nonnull %p, i64 11) #2
  %p1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 32, ptr elementtype(double) nonnull %p0, i64 12) #2
  %p2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) nonnull %p1, i64 13) #2
  store double 1.0, ptr %p2, align 1
  %tmp85 = icmp eq i64 undef, undef
  br i1 %tmp85, label %bb87, label %bb83

bb87:
  ret void
}

attributes #0 = { nounwind readnone speculatable }
attributes #1 = { "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

