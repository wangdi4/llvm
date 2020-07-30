; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases of converting a pointer to an integer for use in a subtract
; instruction that are NOT safe.

; Subtracting two pointers that came from field addresses is computing the
; layout of an aggregate, and is not safe. This will cause %struct.test01a to be
; marked with "Bad pointer manipulation". %struct.test01b is not 'bad pointer
; manipulation' even though those are the types involved in the subtraction
; because they are ptr-to-ptr types. However, %struct.test01c is "Bad pointer
; manipulation" because it is nested with the structure that the address offsets
; were computed, and therefore this type of pointer subtraction could result in
; its size being deduced by pointer arithmetic of the field addresses.
%struct.test01a = type { %struct.test01b*, %struct.test01c, %struct.test01b*, %struct.test01b* }
%struct.test01b = type { i32, i32 }
%struct.test01c = type { i64 }
define void @test01(%struct.test01a* %pStruct) !dtrans_type !6 {
  %pField0 = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 0
  %pField2 = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 2
  %tmp1 = ptrtoint %struct.test01b** %pField0 to i64
  %tmp2 = ptrtoint %struct.test01b** %pField2 to i64
  %offset = sub i64 %tmp2, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Bad pointer manipulation | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: No issues found

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01c
; CHECK: Safety data: Bad pointer manipulation | Nested structure{{ *$}}


; Subtraction with one value being an field element and the other not is not
; permitted even if both pointers are the same type.
%struct.test02a = type { i32, %struct.test02b }
%struct.test02b = type { i32, i32 }
define void @test02(%struct.test02a* %pStructA, %struct.test02b* %pStructB) !dtrans_type !11 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStructA, i64 0, i32 1
  %tmp1 = ptrtoint %struct.test02b* %pField to i64
  %tmp2 = ptrtoint %struct.test02b* %pStructB to i64
  %offset = sub i64 %tmp2, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Bad pointer manipulation | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Bad pointer manipulation | Nested structure{{ *$}}


; Subtracting two pointers of different types is not permitted
%struct.test03a = type { i32, i32 }
%struct.test03b = type { i64 }
define void @test03(%struct.test03a* %pStructA, %struct.test03b* %pStructB) !dtrans_type !15 {
  %tmp1 = ptrtoint %struct.test03a* %pStructA to i64
  %tmp2 = ptrtoint %struct.test03b* %pStructB to i64
  %offset = sub i64 %tmp2, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Bad pointer manipulation{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Bad pointer manipulation{{ *$}}


; Subtracting a scalar from a pointer is not permitted
%struct.test04 = type { i32, i32 }
define void @test04(%struct.test04* %pStruct, i64 %other) !dtrans_type !20 {
  %tmp1 = ptrtoint %struct.test04* %pStruct to i64
  %offset = sub i64 %tmp1, %other
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: Safety data: Bad pointer manipulation{{ *$}}


; Subtracting a scalar from a pointer is not permitted
%struct.test05 = type { i32, i32 }
define void @test05(%struct.test05* %pStruct, i64 %other) !dtrans_type !23 {
  %tmp1 = ptrtoint %struct.test05* %pStruct to i64
  %offset = sub i64 %other, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05
; CHECK: Safety data: Bad pointer manipulation{{ *$}}


; Ambiguous types are not permitted
%struct.test06a = type { i32, i32 }
%struct.test06b = type { i64 }
define void @test06(%struct.test06a* %pStruct1, %struct.test06a* %pStruct2) !dtrans_type !26 {
  %pStruct1.as.pB = bitcast %struct.test06a* %pStruct1 to %struct.test06b*
  %pStruct2.as.pB = bitcast %struct.test06a* %pStruct1 to %struct.test06b*
  %use1 = getelementptr %struct.test06b, %struct.test06b* %pStruct1.as.pB, i64 0, i32 0
  %use2 = getelementptr %struct.test06b, %struct.test06b* %pStruct2.as.pB, i64 0, i32 0
  %tmp1 = ptrtoint %struct.test06b* %pStruct1.as.pB to i64
  %tmp2 = ptrtoint %struct.test06b* %pStruct2.as.pB to i64
  %offset = sub i64 %tmp2, %tmp1
  %div = sdiv i64 %offset, 8
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06a
; CHECK: Safety data: Bad casting | Bad pointer manipulation | Ambiguous GEP{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06b
; CHECK: Safety data: Bad casting | Bad pointer manipulation | Ambiguous GEP{{ *$}}


; Subtracting two pointers of the same type, but not using it for a divide.
; This prevents DTrans from handling it
%struct.test07 = type { i64, i64 }
define void @test07(%struct.test07* %pStruct1, %struct.test07* %pStruct2, i64* %distance) !dtrans_type !29 {
  %t1 = ptrtoint %struct.test07* %pStruct1 to i64
  %t2 = ptrtoint %struct.test07* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  store i64 %offset, i64* %distance
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test07
; CHECK: Safety data: Bad pointer manipulation{{ *$}}


; Subtracting two pointers but dividing by a value that is not the recognized
; size of the aggregate is not permitted
%struct.test08 = type { i64, i64 }
define void @test08(%struct.test08* %pStruct1, %struct.test08* %pStruct2, i64 %n) !dtrans_type !33 {
  %t1 = ptrtoint %struct.test08* %pStruct1 to i64
  %t2 = ptrtoint %struct.test08* %pStruct2 to i64
  %offset = sub i64 %t2, %t1
  %div = sdiv i64 %offset, %n
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test08
; CHECK: Safety data: Bad pointer manipulation{{ *$}}


!1 = !{!2, i32 1}  ; %struct.test01b*
!2 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!3 = !{!"R", %struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!4 = !{i32 0, i32 0}  ; i32
!5 = !{i64 0, i32 0}  ; i64
!6 = !{!"F", i1 false, i32 1, !7, !8}  ; void (%struct.test01a*)
!7 = !{!"void", i32 0}  ; void
!8 = !{!9, i32 1}  ; %struct.test01a*
!9 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!10 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!11 = !{!"F", i1 false, i32 2, !7, !12, !14}  ; void (%struct.test02a*, %struct.test02b*)
!12 = !{!13, i32 1}  ; %struct.test02a*
!13 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!14 = !{!10, i32 1}  ; %struct.test02b*
!15 = !{!"F", i1 false, i32 2, !7, !16, !18}  ; void (%struct.test03a*, %struct.test03b*)
!16 = !{!17, i32 1}  ; %struct.test03a*
!17 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!18 = !{!19, i32 1}  ; %struct.test03b*
!19 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!20 = !{!"F", i1 false, i32 2, !7, !21, !5}  ; void (%struct.test04*, i64)
!21 = !{!22, i32 1}  ; %struct.test04*
!22 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!23 = !{!"F", i1 false, i32 2, !7, !24, !5}  ; void (%struct.test05*, i64)
!24 = !{!25, i32 1}  ; %struct.test05*
!25 = !{!"R", %struct.test05 zeroinitializer, i32 0}  ; %struct.test05
!26 = !{!"F", i1 false, i32 2, !7, !27, !27}  ; void (%struct.test06a*, %struct.test06a*)
!27 = !{!28, i32 1}  ; %struct.test06a*
!28 = !{!"R", %struct.test06a zeroinitializer, i32 0}  ; %struct.test06a
!29 = !{!"F", i1 false, i32 3, !7, !30, !30, !32}  ; void (%struct.test07*, %struct.test07*, i64*)
!30 = !{!31, i32 1}  ; %struct.test07*
!31 = !{!"R", %struct.test07 zeroinitializer, i32 0}  ; %struct.test07
!32 = !{i64 0, i32 1}  ; i64*
!33 = !{!"F", i1 false, i32 3, !7, !34, !34, !5}  ; void (%struct.test08*, %struct.test08*, i64)
!34 = !{!35, i32 1}  ; %struct.test08*
!35 = !{!"R", %struct.test08 zeroinitializer, i32 0}  ; %struct.test08
!36 = !{!"S", %struct.test01a zeroinitializer, i32 4, !1, !3, !1, !1} ; { %struct.test01b*, %struct.test01c, %struct.test01b*, %struct.test01b* }
!37 = !{!"S", %struct.test01b zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!38 = !{!"S", %struct.test01c zeroinitializer, i32 1, !5} ; { i64 }
!39 = !{!"S", %struct.test02a zeroinitializer, i32 2, !4, !10} ; { i32, %struct.test02b }
!40 = !{!"S", %struct.test02b zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!41 = !{!"S", %struct.test03a zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!42 = !{!"S", %struct.test03b zeroinitializer, i32 1, !5} ; { i64 }
!43 = !{!"S", %struct.test04 zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!44 = !{!"S", %struct.test05 zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!45 = !{!"S", %struct.test06a zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!46 = !{!"S", %struct.test06b zeroinitializer, i32 1, !5} ; { i64 }
!47 = !{!"S", %struct.test07 zeroinitializer, i32 2, !5, !5} ; { i64, i64 }
!48 = !{!"S", %struct.test08 zeroinitializer, i32 2, !5, !5} ; { i64, i64 }

!dtrans_types = !{!36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48}
