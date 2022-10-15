; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases that are safe uses of the return instruction.

; Test with return of pointer of the expected type
; Return a properly typed pointer to a structure.
%struct.test01 = type { i32, i32 }
define "intel_dtrans_func_index"="1" %struct.test01* @test1(%struct.test01* "intel_dtrans_func_index"="2" %pStruct) !intel.dtrans.func.type !3 {
  ret %struct.test01* %pStruct
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; Test with return of null pointer.
%struct.test02 = type { i32, i32 }
define "intel_dtrans_func_index"="1" %struct.test02* @test2() !intel.dtrans.func.type !5 {
  ret %struct.test02* null
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


; Return an i64 type that does not represent a pointer to a
; structure.
%struct.test03 = type { i64 }
define i64 @test3(%struct.test03* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !8 {
  %addr = getelementptr %struct.test03, %struct.test03* %pStruct, i64 0, i32 0
  %val = load i64, i64* %addr
  ret i64 %val
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4}
!6 = !{i64 0, i32 0}  ; i64
!7 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!11 = !{!"S", %struct.test03 zeroinitializer, i32 1, !6} ; { i64 }

!intel.dtrans.types = !{!9, !10, !11}
