; REQUIRES: asserts
; This test verifies that structs with safety violations are rejected for
; DynClone transformation.

;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

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
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %j = bitcast i8* %call1 to %struct.test.01*
  %k = bitcast %struct.test.01* %j to %struct.test.02*
  %f01 = getelementptr %struct.test.01, %struct.test.01* %j, i64 0, i32 0
  %g01 = getelementptr %struct.test.02, %struct.test.02* %k, i64 0, i32 0
  ret i32 0
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{%struct.test.02 zeroinitializer, i32 1}  ; %struct.test.02*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, %struct.test.02*, i64 }
!8 = !{!"S", %struct.test.02 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !2, !2} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!7, !8}
