; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Load a pointer to a structure with a different type than expected
; for the structure. This should result in the 'Bad casting' safety
; condition. When pointers are opaque, the bitcast will not exist,
; but the types are mismatched so it will still be treated as if it
; were cast.

%struct.test01a = type { i32 }
%struct.test01b = type { i16, i16 }
define void @test01(%struct.test01a** "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !4 {
  %bc = bitcast %struct.test01a** %pStruct to %struct.test01b**
  %pB = load %struct.test01b*, %struct.test01b** %bc
  %pField = getelementptr %struct.test01b, %struct.test01b* %pB, i64 0, i32 1
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


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{%struct.test01a zeroinitializer, i32 2}  ; %struct.test01a**
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { i32 }
!6 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !2} ; { i16, i16 }

!intel.dtrans.types = !{!5, !6}
