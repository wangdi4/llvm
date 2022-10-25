; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases for checking that storing 'null' is handled by safety analyzer.

; 'null' constants need to be treated as a special case because a Value object
; within the IR for 'null' can be any type when opaque pointers are in use
; because they are just 'p0 null'

; safe - using ptr-to-ptr
%struct.test01 = type { i32 }
define void @test01() {
  %local = alloca %struct.test01*, !intel_dtrans_type !2
  store %struct.test01* null, %struct.test01** %local
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; Safety data: Local pointer{{ *$}}
; CHECK: End LLVMType: %struct.test01


; safe - using element pointee is that target of the store
%struct.test02 = type { i32* }
define void @test02() {
  %local = alloca %struct.test02
  %pField = getelementptr %struct.test02, %struct.test02* %local, i64 0, i32 0
  store i32* null, i32** %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; Safety data: Local pointer{{ *$}}
; CHECK: End LLVMType: %struct.test02


; safe - because element zero is a pointer type
%struct.test03 = type { i32* }
define void @test03() {
  %local = alloca %struct.test03
  %local.as.p32 = bitcast %struct.test03* %local to i32**
  store i32* null, i32** %local.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; Safety data: Local pointer{{ *$}}
; CHECK: End LLVMType: %struct.test03


; unsafe - because element zero is not a pointer type
%struct.test04 = type { i32 }
define void @test04() {
  %local = alloca %struct.test04
  %local.as.p32 = bitcast %struct.test04* %local to i32**
  store i32* null, i32** %local.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; Safety data: Bad casting | Mismatched element access | Local pointer{{ *$}}
; CHECK: End LLVMType: %struct.test04


; Storing 'null' is a special case, because with opaque pointers, the Value
; object for 'null' will be the same object of type 'p0'. Verify that 'null' is
; handled without any safety violations when there are multiple uses of the
; Constant value within the function.
%struct.test05a = type { i32*, i64**, %struct.test05b* }
%struct.test05b = type { i32, i32, i32 }
define void @test05(%struct.test05a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %pField0 = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 0
  %pField1 = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 1
  %pField2 = getelementptr %struct.test05a, %struct.test05a* %pStruct, i64 0, i32 2
  store i32* null, i32** %pField0
  store i64** null, i64*** %pField1
  store %struct.test05b* null, %struct.test05b** %pField2
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05a
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test05a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05b
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test05b

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{i64 0, i32 2}  ; i64**
!5 = !{%struct.test05b zeroinitializer, i32 1}  ; %struct.test05b*
!6 = !{%struct.test05a zeroinitializer, i32 1}  ; %struct.test05a*
!7 = distinct !{!6}
!8 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 1, !3} ; { i32* }
!10 = !{!"S", %struct.test03 zeroinitializer, i32 1, !3} ; { i32* }
!11 = !{!"S", %struct.test04 zeroinitializer, i32 1, !1} ; { i32 }
!12 = !{!"S", %struct.test05a zeroinitializer, i32 3, !3, !4, !5} ; { i32*, i64**, %struct.test05b* }
!13 = !{!"S", %struct.test05b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!8, !9, !10, !11, !12, !13}
