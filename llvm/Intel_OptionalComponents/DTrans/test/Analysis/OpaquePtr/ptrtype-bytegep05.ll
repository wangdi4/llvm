; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test of byte flattened GEP that feeds a memset call.

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Byte-flattened GEP that does not align to the start of a field, but is an
; inter-field padding byte. When the result is passed to 'memset', this needs
; to be tracked as a valid non-field access to support the special case seen
; where a memset starts on the inter-field padding byte.
%struct.test01 = type { i32, i16, i32 }
define internal void @test01(i32 %x) {
  %local = alloca %struct.test01
  %flat = bitcast %struct.test01* %local to i8*
  %pad_addr = getelementptr i8, i8* %flat, i32 6
  call void @llvm.memset.p0i8.i64(i8* %pad_addr, i8 0, i64 6, i1 false)
  ret void
}
; CHECK-LABEL: void @test01
; CHECK-NONOPAQUE: %pad_addr = getelementptr i8, i8* %flat, i32 6
; CHECK-OPAQUE: %pad_addr = getelementptr i8, ptr %flat, i32 6
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i8*
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.test01 @ not-field ByteOffset: 6

declare !intel.dtrans.func.type !4 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i16, i32 }

!intel.dtrans.types = !{!5}
