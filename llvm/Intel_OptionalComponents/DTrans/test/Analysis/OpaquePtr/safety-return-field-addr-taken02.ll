; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test returning a field address with a type that differs than the type
; defined for the field. This should result in "Bad casting" in addition to
; "Field address taken return".

; Cast an arbitrary i8* to a structure pointer type.
%struct.test01a = type { i32, i8 }
%struct.test01b = type { i32, i32 }
@p.test2 = internal unnamed_addr global %struct.test01a zeroinitializer
define %struct.test01b* @test2() !dtrans_type !3 {
  %s = bitcast i8* getelementptr( %struct.test01a, %struct.test01a* @p.test2,
                                  i64 0, i32 1) to %struct.test01b*
  ret %struct.test01b* %s
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: 0)Field LLVM Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i8
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Bad casting | Global instance | Field address taken return{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting{{ *$}}


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 0}  ; i8
!3 = !{!"F", i1 false, i32 0, !4}  ; %struct.test01b* ()
!4 = !{!5, i32 1}  ; %struct.test01b*
!5 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!6 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !2} ; { i32, i8 }
!7 = !{!"S", %struct.test01b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!6, !7}
