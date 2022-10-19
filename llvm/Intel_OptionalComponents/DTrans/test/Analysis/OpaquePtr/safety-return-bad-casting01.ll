; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test return of pointer which does not match the aggregate type pointer.

; Return a pointer to a structure as a pointer to an integer
%struct.test01 = type { i64, i64 }
define "intel_dtrans_func_index"="1" i32* @test01(%struct.test01* "intel_dtrans_func_index"="2" %pStruct) !intel.dtrans.func.type !4 {
  %pStruct.as.p32 = bitcast %struct.test01* %pStruct to i32*
  ret i32* %pStruct.as.p32
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Return a pointer to a structure as a pointer to a different type of structure
%struct.test02a = type { i32, i32 }
%struct.test02b = type { i64 }
define "intel_dtrans_func_index"="1" %struct.test02b* @test02(%struct.test02a* "intel_dtrans_func_index"="2" %pStruct) !intel.dtrans.func.type !8 {
  %pStruct.as.2b = bitcast %struct.test02a* %pStruct to %struct.test02b*
  ret %struct.test02b* %pStruct.as.2b
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test02b


!1 = !{i64 0, i32 0}  ; i64
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!2, !3}
!5 = !{i32 0, i32 0}  ; i32
!6 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!7 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!8 = distinct !{!6, !7}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!10 = !{!"S", %struct.test02a zeroinitializer, i32 2, !5, !5} ; { i32, i32 }
!11 = !{!"S", %struct.test02b zeroinitializer, i32 1, !1} ; { i64 }

!intel.dtrans.types = !{!9, !10, !11}
