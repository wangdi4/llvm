; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes=dtrans-deletefieldop -S 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test checks that the DTrans delete field pass handles updating
; global variable initializers when a field is being deleted from a type
; contained within another type when there is a non-zero initializer.

; The 'i16' field should be deleted from the nested structure.
%struct.test01t = type { i32, i16 }
%struct.test01 = type { i32, i32, %struct.test01t }

@glob = internal global %struct.test01 { i32 0, i32 1, %struct.test01t { i32 2, i16 3 } }
; CHECK: @glob = internal global %__DFDT_struct.test01 { i32 0, i32 1, %__DFT_struct.test01t { i32 2 } }

define i32 @test01(%struct.test01* "intel_dtrans_func_index"="1" %s1) !intel.dtrans.func.type !5 {
  %pA = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 0
  %pB = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 1
  %pC0 = getelementptr %struct.test01, %struct.test01* %s1, i64 0, i32 2, i32 0

  %vA = load i32, i32* %pA
  %vB = load i32, i32* %pB
  %vC0 = load i32, i32* %pC0

  %tmp = add i32 %vC0, %vB
  %res = add i32 %tmp, %vA
  ret i32 %res
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{%struct.test01t zeroinitializer, i32 0}  ; %struct.test01t
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01t zeroinitializer, i32 2, !1, !2} ; { i32, i16 }
!7 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !3} ; { i32, i32, %struct.test01t }

!intel.dtrans.types = !{!6, !7}
