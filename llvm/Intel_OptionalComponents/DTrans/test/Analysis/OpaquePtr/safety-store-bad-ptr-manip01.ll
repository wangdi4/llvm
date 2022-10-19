; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_T
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s --check-prefix=CHECK_ALWAYS --check-prefix=CHECK_OOB_F

target triple = "x86_64-unknown-linux-gnu"

; Test storing a runtime dependent array member that is contained within a
; structure.
;
; When -dtrans-outofboundsok=true, storing the address of an unknown array
; element member within a structure type is treated as "Bad pointer
; manipulation".
;
; When -dtrans-outofboundsok=false, it is assumed to access a location within
; the array and will trigger the "Field address taken memory" flag instead because
; the address could be for the zeroth element.

%struct.test01a = type { i64, float, %struct.test01b }
%struct.test01b = type { i64, [10 x i8] }
@var01a = internal global %struct.test01a zeroinitializer
@var01charptr = internal global i8* zeroinitializer, !intel_dtrans_type !6
define void @test01(i64 %arg)  {
  %array_elem_addr = getelementptr %struct.test01a, %struct.test01a* @var01a, i64 0, i32 2, i32 1, i64 %arg
  store i8* %array_elem_addr, i8** @var01charptr
  ret void
}
; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: LLVMType: %struct.test01a
; CHECK_OOB_T: Safety data: Bad pointer manipulation | Global instance | Contains nested structure{{ *$}}
; CHECK_OOB_F: Safety data: Global instance | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK_ALWAYS-LABEL: DTRANS_StructInfo:
; CHECK_ALWAYS: LLVMType: %struct.test01b
; CHECK_OOB_T: Safety data: Bad pointer manipulation | Global instance | Nested structure{{ *$}}
; CHECK_OOB_F: Safety data: Field address taken memory | Global instance | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01


!1 = !{i64 0, i32 0}  ; i64
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!4 = !{!"A", i32 10, !5}  ; [10 x i8]
!5 = !{i8 0, i32 0}  ; i8
!6 = !{i8 0, i32 1}  ; i8*
!7 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !2, !3} ; { i64, float, %struct.test01b }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !4} ; { i64, [10 x i8] }

!intel.dtrans.types = !{!7, !8}
