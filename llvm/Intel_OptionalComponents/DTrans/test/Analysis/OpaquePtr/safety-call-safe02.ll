; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test passing a pointer to a structure where the callee does not use the
; argument. This does not require marking it as "Address taken"

; Test as a pointer-sized int type
%struct.test01a = type { i32, i32 }
define void @test01() {
  %pStruct = alloca %struct.test01a
  %pStruct.as.i64 = ptrtoint %struct.test01a* %pStruct to i64
  call void @test01callee(i64 %pStruct.as.i64)
  ret void
}
define void @test01callee(i64 %in) {
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01a


; Test with bitcast function call
%struct.test02a = type { i32, i32 }
%struct.test02b = type { i64 }
define void @test02(%struct.test02a** "intel_dtrans_func_index"="1" %pp) !intel.dtrans.func.type !4 {
  %pStruct = load %struct.test02a*, %struct.test02a** %pp
  call void bitcast (void (%struct.test02b*)* @test02callee
                       to void (%struct.test02a*)*)(%struct.test02a* %pStruct)
  ret void
}
define void @test02callee(%struct.test02b* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !6 {
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test02a zeroinitializer, i32 2}  ; %struct.test02a**
!4 = distinct !{!3}
!5 = !{%struct.test02b zeroinitializer, i32 1}  ; %struct.test02b*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!8 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!9 = !{!"S", %struct.test02b zeroinitializer, i32 1, !2} ; { i64 }

!intel.dtrans.types = !{!7, !8, !9}
