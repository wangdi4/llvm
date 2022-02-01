; REQUIRES: asserts
; (CMPLRLLVM-34048) This test verifies that Andersens Analysis is disabled
; when size of any function exceeds max limit.
; anders-function-size-max-limit option is used to set max size to 2.

; RUN: opt < %s -anders-aa -anders-function-size-max-limit=2 -print-anders-constraints -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -passes='require<anders-aa>' -anders-function-size-max-limit=2 -print-anders-constraints -disable-output 2>&1 | FileCheck %s

; CHECK: Anders disabled...exceeded FunctionSizeMaxLimit

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@reff = internal unnamed_addr global double* null, align 8

define dso_local i32 @main() {
entry:
  %0 = tail call noalias i8* @malloc(i64 500)
  store i8* %0, i8** bitcast (double** @reff to i8**)
  %1 = bitcast i8* %0 to double*
  br label %L2
L1:
  %2 = load double*, double** @reff
  br label %L2
L2:
  %3 = phi double* [ %1, %entry ], [ %2, %L1 ]
  %4 = tail call noalias i8* @malloc(i64 500)
  store double* null, double** @reff
  %5 = bitcast double* %1 to i8*
  tail call void @free(i8* %5)
  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)
declare dso_local void @free(i8* nocapture)
