; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; This test verifies that structs with safety violations are rejected for
; DynClone transformation.

;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-dynclone -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Both %struct.test.01 and %struct.test.02 are not considered as
; candidates due to safety checks.
%struct.test.01 = type { i32, i64, i32, i32, i16, %struct.test.02*, i64 }
%struct.test.02 = type { i32, i64, i32, i32, i16, i64, i64 }

; CHECK: DynCloning Transformation
; CHECK:   Looking for candidate structures.
; CHECK-DAG:     Rejecting struct.test.01 based on safety data.
; CHECK-DAG:     Rejecting struct.test.02 based on safety data.
; CHECK:     No possible candidates found.

define i32 @main() {
entry:
  %call1 = tail call noalias i8* @calloc(i64 10, i64 48)
  %j = bitcast i8* %call1 to %struct.test.01*
  %i = bitcast i8* %call1 to i32*
  store i32 10, i32* %i, align 8
  %k = bitcast %struct.test.01* %j to %struct.test.02*
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
