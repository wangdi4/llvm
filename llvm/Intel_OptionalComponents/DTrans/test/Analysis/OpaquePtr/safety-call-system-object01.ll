; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that return values are marked as 'System object' for types returned by
; functions that may be external due to 'dllexport'

@var01 = internal global i8 zeroinitializer
%struct.test01 = type { i32, i32 }
define void @test01() {
  %p8 = call i8* @extern_func01()
  %p8.as.struct = bitcast i8* %p8 to %struct.test01*
  %field.addr = getelementptr %struct.test01, %struct.test01* %p8.as.struct, i64 0, i32 0
  store i32 0, i32* %field.addr
  ret void
}
; This function is defined with the 'dllexport' attribute to make it appear as
; an external function whose type cannot be changed.
define dllexport i8* @extern_func01() !dtrans_type !2 {
  ret i8* @var01
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: Bad casting | System object{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 0, !3}  ; i8* ()
!3 = !{i8 0, i32 1}  ; i8*
!4 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!4}
