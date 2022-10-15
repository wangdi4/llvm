; REQUIRES: asserts
; This test verifies only i64 type fields are selected as candidates for
; DynClone transformation.

;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK: DynCloning Transformation
; CHECK:    Possible Candidate fields:
; CHECK:    struct: struct.test.01    Index: 1
; CHECK-NOT:    struct: struct.test.01    Index: 5
; CHECK:    struct: struct.test.01    Index: 6

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Only 1 and 6 fields are selected for candidates.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64 }

define i32 @main() {
entry:
  %call1 = tail call i8* @calloc(i64 10, i64 48)
  %j = bitcast i8* %call1 to %struct.test.01*
  %f01 = getelementptr %struct.test.01, %struct.test.01* %j, i64 0, i32 0
  store i32 10, i32* %f01
  ret i32 0
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test.01 zeroinitializer, i32 7, !1, !2, !1, !1, !3, !4, !2} ; { i32, i64, i32, i32, i16, i64*, i64 }

!intel.dtrans.types = !{!7}
