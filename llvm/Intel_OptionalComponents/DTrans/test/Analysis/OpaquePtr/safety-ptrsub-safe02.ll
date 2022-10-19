; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -debug-only=dtrans-safetyanalyzer-verbose -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -debug-only=dtrans-safetyanalyzer-verbose -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test the collection of type information for pointer subtraction cases that are
; safe for transformations which may require updating an operand that represents
; the structure size when modifying the structure.

; Subtracting two pointers of the same type yields a safe offset when the
; result is used for dividing by the structure's size.
%struct.test01 = type { i64, i64 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct1, %struct.test01* "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !3 {
  %t1 = ptrtoint %struct.test01* %pStruct1 to i64
  %t2 = ptrtoint %struct.test01* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  ; Division by structure size.
  %count = sdiv i64 %offset, 16
  ret void
}

; udiv is also safe.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct1, %struct.test02* "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !6 {
  %t1 = ptrtoint %struct.test02* %pStruct1 to i64
  %t2 = ptrtoint %struct.test02* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  ; Division by structure size.
  %count = udiv i64 %offset, 8
  ret void
}

; Pointer subtraction that gets used in a divide using a multiple of the
; structure size.
%struct.test03 = type { i64, i64, i64 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %ppStruct1, %struct.test03* "intel_dtrans_func_index"="2" %ppStruct2) !intel.dtrans.func.type !8 {
  %t1 = ptrtoint %struct.test03* %ppStruct1 to i64
  %t2 = ptrtoint %struct.test03* %ppStruct2 to i64
  %mult = mul i64 24, 4
  %offset = sub i64 %t2, %t1
  ; Division by a multiple of the structure size.
  %count = sdiv i64 %offset, %mult
  ret void
}

; CHECK: addPtrSubMapping: test01:   %offset = sub i64 %t2, %t1 -- %struct.test01 = type { i64, i64 }
; CHECK: addPtrSubMapping: test02:   %offset = sub i64 %t2, %t1 -- %struct.test02 = type { i32, i32 }
; CHECK: addPtrSubMapping: test03:   %offset = sub i64 %t2, %t1 -- %struct.test03 = type { i64, i64, i64 }

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{i32 0, i32 0}  ; i32
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5, !5}
!7 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!8 = distinct !{!7, !7}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!11 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i64, i64, i64 }

!intel.dtrans.types = !{!9, !10, !11}
