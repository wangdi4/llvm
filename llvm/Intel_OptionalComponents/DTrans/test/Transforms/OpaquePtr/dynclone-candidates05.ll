; REQUIRES: asserts
; This test verifies that no field is qualified for DynClone transformation
; due to memset.

;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s
;  RUN: opt < %s -dtransop-allow-typed-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume -intel-libirc-allowed -passes=dtrans-dyncloneop -debug-only=dtrans-dynclone -disable-output 2>&1 | FileCheck %s

; CHECK-LABEL:  Possible Candidate fields:
; CHECK:    struct: struct.test.01 Index: 1
; CHECK:    struct: struct.test.01 Index: 6
; CHECK:    struct: struct.test.01 Index: 7

; CHECK:  Invalid...written by memset
; CHECK:  No Candidate is qualified

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; 1, 6 and 7 fields are selected for possible candidates. After pruning,
; none of the fields is selected as final candidates due to memset.
; Fields are written using memset with non-zero value.
%struct.test.01 = type { i32, i64, i32, i32, i16, i64*, i64, i64 }

define i32 @main() {
entry:
  %call1 = tail call i8* @calloc(i64 10, i64 56)
  %j = bitcast i8* %call1 to %struct.test.01*
  %f01 = getelementptr %struct.test.01, %struct.test.01* %j, i64 0, i32 0
  call void @llvm.memset.p0i8.i64(i8* %call1, i8 1, i64 56, i1 false)
  ret i32 0
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0
declare !intel.dtrans.func.type !7 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{!"S", %struct.test.01 zeroinitializer, i32 8, !1, !2, !1, !1, !3, !4, !2, !2} ; { i32, i64, i32, i32, i16, i64*, i64, i64 }

!intel.dtrans.types = !{!8}
