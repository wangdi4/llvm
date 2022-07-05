; This test verifies that "store ptr %call2" is NOT removed by
; instcombine using Andersens's points-to analysis.
; AndersensAA should handle direct calls when types of call and
; callee don't match.

; RUN: opt < %s -passes='require<anders-aa>,instcombine' -S 2>&1 | FileCheck %s

; CHECK: call{{.*}}Z_myalloc{{.*}}16
; CHECK: call{{.*}}Z_myalloc{{.*}}200
; CHECK: store{{.*}}ptr %mantissa

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.R_flstr = type { i32, i32, ptr }

; Function Attrs: nounwind uwtable
define dso_local ptr @R_makefloat() local_unnamed_addr {
entry:
  %call = tail call ptr (i64, ...) @_Z_myalloc(i64 noundef 16)
  %call2 = tail call ptr (i64, ...) @_Z_myalloc(i64 noundef 200)
  %mantissa = getelementptr inbounds %struct.R_flstr, ptr %call, i64 0, i32 2
  store ptr %call2, ptr %mantissa, align 8
  ret ptr %call
}

declare dso_local ptr @_Z_myalloc(...) local_unnamed_addr
