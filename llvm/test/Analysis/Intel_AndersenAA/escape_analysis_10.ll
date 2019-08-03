; This test verifies that Escape analysis (in Andersens) shouldn't
; consider memset intrinsic as escape point.

; RUN: opt < %s -anders-aa -aa-eval -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -print-all-alias-modref-info -disable-output 2>&1 | FileCheck %s

; CHECK: NoAlias:     double* %0, double* %Ptr

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@sumdeijda = internal unnamed_addr global double* null, align 8

define dso_local i32 @foo(double* %Ptr) {
L0:
  %0 = load double*, double** @sumdeijda
  %1 = tail call noalias i8* @malloc(i64 500)
  store i8* %1, i8** bitcast (double** @sumdeijda to i8**)
  %2 = bitcast i8* %1 to double*
  br label %L2
L1:
  %3 = load double*, double** @sumdeijda
  br label %L2
L2:
  %4 = phi double* [ %2, %L0 ], [ %3, %L1 ]
  %5 = bitcast double* %4 to i8*
  call void @llvm.memset.p0i8.i64(i8* %5, i8 1, i64 1024, i1 false)
  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg)
