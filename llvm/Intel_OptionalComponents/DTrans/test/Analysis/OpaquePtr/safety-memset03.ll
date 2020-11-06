; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test cases where memset is called on an element of an array that starts the
; structure, but the array element address is not known to be the same as the
; structure address.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Pass a pointer to an array element that is not the same as the structure
; address to memset. In this case, the element pointee information is collected
; about the GEP being an element of the array.
;
; NOTE: The safety results on this case differ from the behavior of the
; LocalPointerAnalyzer (LPA) implementation. The LPA did not recognize that the
; array is nested within the structure when collecting the element pointee info,
; and therefore treated it as safe regardless of whether the size argument would
; stay within the bounds of the array or not.
%struct.test01 = type { [200 x i8], i32, i32, i32, i32 }
@var01 = internal global %struct.test01 zeroinitializer
define void @test01(%struct.test01* %pStruct) !dtrans_type !4 {
  %elem = getelementptr %struct.test01, %struct.test01* @var01, i64 0, i32 0, i32 32
  call void @llvm.memset.p0i8.i64(i8* %elem, i8 1, i64 40, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Global instance | Bad memfunc size{{ *$}}


; Pass a runtime dependent array element address to memset. In this case, the
; runtime dependent index may be 0, so DTrans information will recognize that
; the array field starts the structure, and therefore could have the same
; address as the structure as a whole, and mark the type as not being able to be
; handled by DTrans.
%struct.test02 = type { [200 x i8], i32, i32, i32, i32 }
@var02 = internal global %struct.test02 zeroinitializer
define void @test02(%struct.test02* %pStruct, i32 %idx) !dtrans_type !8 {
  %elem = getelementptr %struct.test02, %struct.test02* @var02, i64 0, i32 0, i32 %idx
  call void @llvm.memset.p0i8.i64(i8* %elem, i8 1, i64 1, i1 false)
  ret void
}
; NOTE: This test is run with -dtrans-outofboundsok=false, because otherwise the
; GEP sets an additional safety flag on the structure.

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: Global instance | Bad memfunc size{{ *$}}


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{!"A", i32 200, !2}  ; [200 x i8]
!2 = !{i8 0, i32 0}  ; i8
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01*
!7 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!8 = !{!"F", i1 false, i32 2, !5, !9, !3}  ; void (%struct.test02*, i32)
!9 = !{!10, i32 1}  ; %struct.test02*
!10 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!11 = !{!"F", i1 false, i32 4, !5, !12, !2, !13, !14}  ; void (i8*, i8, i64, i1)
!12 = !{i8 0, i32 1}  ; i8*
!13 = !{i64 0, i32 0}  ; i64
!14 = !{i1 0, i32 0}  ; i1
!15 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !3, !3, !3, !3} ; { [200 x i8], i32, i32, i32, i32 }
!16 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !3, !3, !3, !3} ; { [200 x i8], i32, i32, i32, i32 }
!17 = !{!"llvm.memset.p0i8.i64", !11}

!dtrans_types = !{!15, !16}
!dtrans_decl_types = !{!17}
