; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test pointer type recovery for calloc library calls based on
; inference from uses of the value created by the call instruction.

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.test01 = type { %struct.test01* }
define internal void @test01(%struct.test01* %in, i64 %index) !dtrans_type !1 {
  %mem_i8 = call i8* @calloc(i64 16, i64 8)
  %array = bitcast i8* %mem_i8 to %struct.test01**
  %array_elem = getelementptr %struct.test01*, %struct.test01** %array, i64 %index

  ; This is the instruction that provides the information need to determine
  ; the allocated type because it relies on the metadata information from
  ; the function signature.
  ; This infers the allocation type based on the type of the value being
  ; stored.
  store %struct.test01* %in, %struct.test01** %array_elem
  ret void
}
; CHECK-LABEL: void @test01
; CHECK-CUR:  %mem_i8 = call i8* @calloc(i64 16, i64 8)
; CHECK-FUT:  %mem_i8 = call p0 @calloc(i64 16, i64 8)
; CHECK-NEXT: LocalPointerInfo:
; CHECK-NEXT: Aliased types:
; CHECK-NEXT:   %struct.test01**{{ *$}}
; CHECK-NEXT:   i8*{{ *$}}
; CHECK-NEXT: No element pointees.


declare i8* @calloc(i64, i64)

!1 = !{!"F", i1 false, i32 2, !2, !3, !5}  ; void (%struct.test01*, i64)
!2 = !{!"void", i32 0}  ; void
!3 = !{!4, i32 1}  ; %struct.test01*
!4 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!5 = !{i64 0, i32 0}  ; i64
!6 = !{!"S", %struct.test01 zeroinitializer, i32 1, !3} ; { %struct.test01* }

!dtrans_types = !{!6}
