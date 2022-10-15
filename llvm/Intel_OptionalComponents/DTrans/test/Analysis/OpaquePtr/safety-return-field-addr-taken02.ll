; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test returning a field address with a type that differs than the type
; defined for the field. This should result in "Bad casting" in addition to
; "Field address taken return".

; Cast an arbitrary i8* to a structure pointer type.
%struct.test01a = type { i32, i8 }
%struct.test01b = type { i32, i32 }
@p.test2 = internal unnamed_addr global %struct.test01a zeroinitializer
define "intel_dtrans_func_index"="1" %struct.test01b* @test2() !intel.dtrans.func.type !4 {
  %s = bitcast i8* getelementptr( %struct.test01a, %struct.test01a* @p.test2,
                                  i64 0, i32 1) to %struct.test01b*
  ret %struct.test01b* %s
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: 0)Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i8
; CHECK: DTrans Type: i8
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Bad casting | Global instance | Field address taken return{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Bad casting{{ *$}}
; CHECK: End LLVMType: %struct.test01b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 0}  ; i8
!3 = !{%struct.test01b zeroinitializer, i32 1}  ; %struct.test01b*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !2} ; { i32, i8 }
!6 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!5, !6}
