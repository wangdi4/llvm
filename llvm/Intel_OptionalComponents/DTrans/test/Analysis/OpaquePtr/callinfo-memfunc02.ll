; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-callinfo -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-callinfo -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test the safety analysis collection of CallInfo objects for cases where
; one argument to memcpy is an element pointee, and the other is not an element
; pointee.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test with memcpy where the source and target types match, where the source
; pointer is a field within another structure, while the destination pointer is
; not.
%struct.test01a = type { i32, i32, i32, i32, i32 }
%struct.test01b = type { i32, %struct.test01a }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStructA, %struct.test01b* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !5 {
  %pDst = bitcast %struct.test01a* %pStructA to i8*
  %pField = getelementptr %struct.test01b, %struct.test01b* %pStructB, i64 0, i32 1
  %pSrc = bitcast %struct.test01a* %pField to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: Function: test01
; CHECK: MemfuncInfo:
; CHECK:     Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test01a = type { i32, i32, i32, i32, i32 }
; CHECK:   Region 1:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test01a = type { i32, i32, i32, i32, i32 }


; Test with memcpy where the source and target types match, where the
; destination pointer is a field within another structure, while the source
; pointer is not.
%struct.test02a = type { i32, i32, i32, i32, i32 }
%struct.test02b = type { i32, %struct.test02a }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStructA, %struct.test02b* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !9 {
  %pSrc = bitcast %struct.test02a* %pStructA to i8*
  %pField = getelementptr %struct.test02b, %struct.test02b* %pStructB, i64 0, i32 1
  %pDst = bitcast %struct.test02a* %pField to i8*
  tail call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 20, i1 false)
  ret void
}
; CHECK-LABEL: Function: test02
; CHECK: MemfuncInfo:
; CHECK:     Kind: memcpy
; CHECK:   Region 0:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test02a = type { i32, i32, i32, i32, i32 }
; CHECK:   Region 1:
; CHECK:     Complete: true
; CHECK:     Type: %struct.test02a = type { i32, i32, i32, i32, i32 }


declare !intel.dtrans.func.type !11 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!5 = distinct !{!3, !4}
!6 = !{%struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!7 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!8 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!9 = distinct !{!7, !8}
!10 = !{i8 0, i32 1}  ; i8*
!11 = distinct !{!10, !10}
!12 = !{!"S", %struct.test01a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!13 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !2} ; { i32, %struct.test01a }
!14 = !{!"S", %struct.test02a zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!15 = !{!"S", %struct.test02b zeroinitializer, i32 2, !1, !6} ; { i32, %struct.test02a }

!intel.dtrans.types = !{!12, !13, !14, !15}
