; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to a structure when a bitcast function pointer is used
; for the function call. This is safe because we have the function's definition
; and the types match for the caller and callee.

%struct.test01 = type { i32, i32 }
define void @test01callee(%struct.test01* %pStruct)!dtrans_type !2
{
  ; We need to use the argument for the analysis to check the parameter for
  ; safety in the caller.
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  store i32 1, i32* %pField
  ret void
}

define void @test01() {
  %vp = call i8* @malloc(i64 8)
  call void bitcast (void (%struct.test01*)* @test01callee
                       to void (i8*)*)(i8* %vp)

  ; Establish that %vp is used as %struct.test01* in the caller
  %pStruct = bitcast i8* %vp to %struct.test01*
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  store i32 0, i32* %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


declare i8* @malloc(i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (%struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!6}
