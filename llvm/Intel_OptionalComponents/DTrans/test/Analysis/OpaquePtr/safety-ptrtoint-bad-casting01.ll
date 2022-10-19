; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test the safety checks for 'ptrtoint' instructions when there are unsafe
; uses.

; 'ptrtoint' of a pointer that may alias multiple types is not permitted.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i64 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct1) !intel.dtrans.func.type !4 {
  %pStruct1.as.pB = bitcast %struct.test01a* %pStruct1 to %struct.test01b*
  %use1 = getelementptr %struct.test01b, %struct.test01b* %pStruct1.as.pB, i64 0, i32 0
  %tmp1 = ptrtoint %struct.test01b* %pStruct1.as.pB to i64
  ret void
}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting | Ambiguous GEP{{ *$}}
; CHECK: End LLVMType: %struct.test01b


; 'ptrtoint' that does not go to the same size as a pointer is not permitted.
%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct1) !intel.dtrans.func.type !6 {
  %tmp1 = ptrtoint %struct.test02* %pStruct1 to i32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test02


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 1, !2} ; { i64 }
!9 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!7, !8, !9}
