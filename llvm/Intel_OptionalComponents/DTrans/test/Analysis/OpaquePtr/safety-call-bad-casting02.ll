; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to an aggregate type to a function that expects a
; pointer to a different aggregate type. This should trigger the "Bad casting"
; safety flag.

%struct.test01a = type { i64 }
%struct.test01b = type { i32, i32 }
define void @test01() {
  %pStruct = alloca %struct.test01a
  %pStruct.as.pb = bitcast %struct.test01a* %pStruct to %struct.test01b*
  call void @test01callee(%struct.test01b* %pStruct.as.pb)
  ret void
}

define void @test01callee(%struct.test01b* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !4 {
  %fieldAddr = getelementptr %struct.test01b, %struct.test01b* %pStruct, i64 0, i32 1
  store i32 0, i32* %fieldAddr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test01b


!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i64 }
!6 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!intel.dtrans.types = !{!5, !6}
