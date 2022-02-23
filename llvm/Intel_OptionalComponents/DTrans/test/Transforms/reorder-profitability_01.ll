; This test verifies that reordering transformation will NOT be
; enabled for struct.test based on profitability heuristic.

;  RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -dtrans-reorderfields -S 2>&1 | FileCheck %s
;  RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -whole-program-assume -passes=dtrans-reorderfields -S 2>&1 | FileCheck %s

; CHECK: %struct.test = type { i8, i8, i64 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


%struct.test = type { i8, i8, i64 }

define i32 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 16)
  %0 = bitcast i8* %call to %struct.test*
  ret i32 0
}

; Function Attrs: nounwind
declare dso_local noalias i8* @calloc(i64, i64)
