; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a store uses a pointer to the start of a structure, but
; stores a type that does not match the type that the structure begins with.
; These cases are storing a scalar type when the structure starts with a
; pointer to a structure type.

%struct.test01a = type { %struct.test01b*, %struct.test01b*, %struct.test01b* }
%struct.test01b = type { i32, i32, i32 }
define void @test01(%struct.test01a* %pStruct) !dtrans_type !4 {
  %pStruct.as.p8 = bitcast %struct.test01a* %pStruct to i8*
  store i8 0, i8* %pStruct.as.p8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}

; By default, "Unsafe pointer store" is pointer carried.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


%struct.test02a = type { %struct.test02b*, %struct.test02b*, %struct.test02b* }
%struct.test02b = type { i32, i32, i32 }
define void @test02(%struct.test02a* %pStruct) !dtrans_type !10 {
  %pStruct.as.p16 = bitcast %struct.test02a* %pStruct to i16*
  store i16 0, i16* %pStruct.as.p16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


; This case could be considered safe, since it is equivalent to storing a null
; pointer to the element zero pointer, but for now we will treat it as unsafe to
; avoid special case code that looks at the value being stored when analyzing
; the store instructions.
%struct.test03a = type { %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* %pStruct) !dtrans_type !15 {
  %pStruct.as.p64 = bitcast %struct.test03a* %pStruct to i64*
  store i64 0, i64* %pStruct.as.p64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Bad casting | Unsafe pointer store{{ *$}}


!1 = !{!2, i32 1}  ; %struct.test01b*
!2 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01a*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01a*
!7 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!8 = !{!9, i32 1}  ; %struct.test02b*
!9 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!10 = !{!"F", i1 false, i32 1, !5, !11}  ; void (%struct.test02a*)
!11 = !{!12, i32 1}  ; %struct.test02a*
!12 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!13 = !{!14, i32 1}  ; %struct.test03b*
!14 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!15 = !{!"F", i1 false, i32 1, !5, !16}  ; void (%struct.test03a*)
!16 = !{!17, i32 1}  ; %struct.test03a*
!17 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!18 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b*, %struct.test01b*, %struct.test01b* }
!19 = !{!"S", %struct.test01b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!20 = !{!"S", %struct.test02a zeroinitializer, i32 3, !8, !8, !8} ; { %struct.test02b*, %struct.test02b*, %struct.test02b* }
!21 = !{!"S", %struct.test02b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }
!22 = !{!"S", %struct.test03a zeroinitializer, i32 3, !13, !13, !13} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!23 = !{!"S", %struct.test03b zeroinitializer, i32 3, !3, !3, !3} ; { i32, i32, i32 }

!dtrans_types = !{!18, !19, !20, !21, !22, !23}
