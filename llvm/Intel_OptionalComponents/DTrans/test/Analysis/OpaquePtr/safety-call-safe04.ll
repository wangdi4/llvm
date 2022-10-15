; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to a structure when a bitcast function pointer is used
; for the function call. This is safe because we have the function's definition
; and the types match for the caller and callee.

%struct.test01 = type { i32, i32 }
define void @test01callee(%struct.test01* "intel_dtrans_func_index"="1" %pStruct)!intel.dtrans.func.type !3
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
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!6}
