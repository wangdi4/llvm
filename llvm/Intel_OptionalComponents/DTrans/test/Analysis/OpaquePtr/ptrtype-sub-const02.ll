; REQUIRES: asserts

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test pointer type recovery on subtract instructions

; Compute: const - const
;
; This case is to verify that a compiler assertion is not triggered when
; there is no pointer type information that needs to be associated with
; either operand of a subtract instruction, or the result produced by the
; subtract. (CMPLRLLVM-39915)
%struct.test01 = type { i32, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %p1) !intel.dtrans.func.type !3 {  
  %t1 = ptrtoint ptr %p1 to i64
  %tmp = sub i64 8, 4
  %div = sdiv i64 %tmp, 4
  ret void
}
; CHECK-LABEL: define void @test01
; CHECK: %tmp = sub i64 8, 4
; CHECK-NEXT: %div = sdiv i64 %tmp, 4


; Compute: pointer - pointer
;
; This case produces an integer value, so there is no pointer info tracked
; for the result of the subtract instruction.
%struct.test02 = type { i32, i32 }
define void @test02(ptr "intel_dtrans_func_index"="1" %p1, ptr "intel_dtrans_func_index"="2" %p2) !intel.dtrans.func.type !5 {
  ; %p1 - %p2
  %t1 = ptrtoint ptr %p1 to i64
  %t2 = ptrtoint ptr %p2 to i64
  %offset = sub i64 %t1, %t2
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: define void @test02
; CHECK: %offset = sub i64 %t1, %t2
; CHECK-NEXT: %div = sdiv i64 %offset, 8

; Compute: const - pointer
;
; For compatibility with the legacy DTrans analysis, subtracting a
; pointer address from a constant is not tracked as a pointer type.
%struct.test03 = type { i32, i32 }
define void @test03(ptr "intel_dtrans_func_index"="1" %p1) !intel.dtrans.func.type !7 {  
  %t1 = ptrtoint ptr %p1 to i64
  %tmp = sub i64 8, %t1
  %div = sdiv i64 %tmp, 4
  ret void
}
; CHECK-LABEL: define void @test03
; CHECK:  %tmp = sub i64 8, %t1
; CHECK-NEXT: %div = sdiv i64 %tmp, 4

; Compute: pointer - const
;
; This case treats the result of the subtract as being a pointer
; of the same type as the source zero operand. The safety analyzer
; will verify that the use is just for a division by the size
; of the structure to ensure that the operation is computing the
; distance between two elements of an array of the type.
%struct.test04 = type { i32, i32 }
define void @test04(ptr "intel_dtrans_func_index"="1" %p1) !intel.dtrans.func.type !9 {  
  %t1 = ptrtoint ptr %p1 to i64
  %tmp = sub i64 %t1, 8
  %div = sdiv i64 %tmp, 4
  ret void
}
; CHECK-LABEL: define void @test04
; CHECK: %tmp = sub i64 %t1, 8
; CHECK-NEXT:  LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        %struct.test04*
; CHECK-NEXT:      No element pointees.


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4, !4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6}
!8 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!9 = distinct !{!8}
!10 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!12 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!13 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!10, !11, !12, !13}

