; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that safe GEP uses do not get marked with safety flags by the checks
; for 'Ambiguous GEP' or 'Bad pointer manipulation'

; Test accesses for simple GEPs.
%struct.test01 = type { i64, %struct.test01* }
define internal void @test01(%struct.test01* %in) !dtrans_type !4 {
  %f0 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %v0 = load i64, i64* %f0

  %f1 = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 1
  %v1 = load %struct.test01*, %struct.test01** %f1

  %f2 = getelementptr %struct.test01, %struct.test01* %v1, i64 0, i32 0
  %v2 = load i64, i64* %f2

  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


; Test a GEP that is used for a pointer-to-pointer, followed by a safe
; field access.
%struct.test02 = type { i64, i64 }
define internal void @test02(%struct.test02** %in) !dtrans_type !6 {
  %p2p = getelementptr %struct.test02*, %struct.test02** %in, i64 5
  %ptr = load %struct.test02*, %struct.test02** %p2p
  %field_addr = getelementptr %struct.test02, %struct.test02* %ptr, i64 0, i32 1
  store i64 0, i64* %field_addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: No issues found


; Test handling for a zero-sized array element at the end of the structure.
%struct.test03 = type { i64, [0 x i8] }
define void @test03(%struct.test03* %in) !dtrans_type !11 {
  %f1 = getelementptr %struct.test03, %struct.test03* %in, i64 0, i32 1, i32 2
  %v8 = load i8, i8* %f1
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Has zero-sized array{{ *$}}


; Test access with nested structure element using multiple GEPs to traverse structure
%struct.test04inner = type { i64, i32, i64* }
%struct.test04mid = type { i64, i64, %struct.test04inner }
%struct.test04outer = type { i64*, %struct.test04mid }
define internal i64 @test04(%struct.test04outer* %in) !dtrans_type !18 {
  %outer.1 = getelementptr %struct.test04outer, %struct.test04outer* %in, i64 0, i32 1
  %mid.2 = getelementptr %struct.test04mid, %struct.test04mid* %outer.1, i64 0, i32 2
  %inner.0 = getelementptr %struct.test04inner, %struct.test04inner* %mid.2, i64 0, i32 0
  %val0 = load i64, i64* %inner.0

  %inner.2 = getelementptr %struct.test04inner, %struct.test04inner* %mid.2, i64 0, i32 2
  %ptr = load i64*, i64** %inner.2
  %val2 = load i64, i64* %ptr

  ret i64 %val0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04inner
; CHECK: Safety data: Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04mid
; CHECK: Safety data: Nested structure | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04outer
; CHECK: Safety data: Contains nested structure{{ *$}}


; Test access with nested structure element using single GEP to traverse structure
%struct.test05inner = type { i64, i32, i64* }
%struct.test05mid = type { i64, i64, %struct.test05inner }
%struct.test05outer = type { i64*, %struct.test05mid }
define internal i64 @test05(%struct.test05outer* %in) !dtrans_type !23 {
  %addr0 = getelementptr %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 0
  %val0 = load i64, i64* %addr0
  %addr2 = getelementptr %struct.test05outer, %struct.test05outer* %in, i64 0, i32 1, i32 2, i32 2
  %ptr = load i64*, i64** %addr2
  %val2 = load i64, i64* %ptr
  ret i64 %val0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05inner
; CHECK: Safety data: Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05mid
; CHECK: Safety data: Nested structure | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05outer
; CHECK: Safety data: Contains nested structure{{ *$}}


; Test access to an element of an array member of a structure.
%struct.test06 = type { i32, [20 x i64] }
define internal i64 @test06(%struct.test06* %in) !dtrans_type !27 {
  %elem_addr = getelementptr inbounds %struct.test06, %struct.test06* %in, i64 0, i32 1, i64 4
  %val = load i64, i64* %elem_addr
  ret i64 %val
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06
; CHECK: Safety data: No issues found


; Test access to an element that is an array of pointers
%struct.test07 = type { i64, i64 }
@var07 = internal global [16 x %struct.test07*] zeroinitializer, !dtrans_type !30
define internal void @test07() {
  %addr = getelementptr [16 x %struct.test07*], [16 x %struct.test07*]* @var07, i64 0, i32 2
  %sptr = load %struct.test07*, %struct.test07** %addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test07
; CHECK: Safety data: Global pointer{{ *$}}


; Test access to a structure member from an array of structures
%struct.test08 = type { i64, i64 }
@var08 = internal global [16 x %struct.test08] zeroinitializer
define internal void @test08() {
  %addr = getelementptr [16 x %struct.test08], [16 x %struct.test08]* @var08, i64 0, i32 5, i32 1
  %sptr = load i64, i64* %addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test08
; CHECK: Safety data: Global instance | Global array{{ *$}}


; This is a special case of a GEP that uses a null value for the pointer
; operand.
%struct.test09 = type { void (i8*)*, i8* }
define internal void @test09(i64 %offset) {
  %null_offset = getelementptr %struct.test09, %struct.test09* null, i64 %offset
  %fptr_addr = getelementptr inbounds %struct.test09, %struct.test09* %null_offset, i64 0, i32 0
  %fptr = load void (i8*)*, void (i8*)** %fptr_addr

  %cptr_addr = getelementptr %struct.test09, %struct.test09* %null_offset, i64 0, i32 1
  %cptr = load i8*, i8** %cptr_addr
  call void %fptr(i8* %cptr), !dtrans_type !33

 ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test09
; CHECK: Safety data: Has function ptr{{ *$}}


!1 = !{i64 0, i32 0}  ; i64
!2 = !{!3, i32 1}  ; %struct.test01*
!3 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!4 = !{!"F", i1 false, i32 1, !5, !2}  ; void (%struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!"F", i1 false, i32 1, !5, !7}  ; void (%struct.test02**)
!7 = !{!8, i32 2}  ; %struct.test02**
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"A", i32 0, !10}  ; [0 x i8]
!10 = !{i8 0, i32 0}  ; i8
!11 = !{!"F", i1 false, i32 1, !5, !12}  ; void (%struct.test03*)
!12 = !{!13, i32 1}  ; %struct.test03*
!13 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!14 = !{i32 0, i32 0}  ; i32
!15 = !{i64 0, i32 1}  ; i64*
!16 = !{!"R", %struct.test04inner zeroinitializer, i32 0}  ; %struct.test04inner
!17 = !{!"R", %struct.test04mid zeroinitializer, i32 0}  ; %struct.test04mid
!18 = !{!"F", i1 false, i32 1, !1, !19}  ; i64 (%struct.test04outer*)
!19 = !{!20, i32 1}  ; %struct.test04outer*
!20 = !{!"R", %struct.test04outer zeroinitializer, i32 0}  ; %struct.test04outer
!21 = !{!"R", %struct.test05inner zeroinitializer, i32 0}  ; %struct.test05inner
!22 = !{!"R", %struct.test05mid zeroinitializer, i32 0}  ; %struct.test05mid
!23 = !{!"F", i1 false, i32 1, !1, !24}  ; i64 (%struct.test05outer*)
!24 = !{!25, i32 1}  ; %struct.test05outer*
!25 = !{!"R", %struct.test05outer zeroinitializer, i32 0}  ; %struct.test05outer
!26 = !{!"A", i32 20, !1}  ; [20 x i64]
!27 = !{!"F", i1 false, i32 1, !1, !28}  ; i64 (%struct.test06*)
!28 = !{!29, i32 1}  ; %struct.test06*
!29 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!30 = !{!"A", i32 16, !31}  ; [16 x %struct.test07*]
!31 = !{!32, i32 1}  ; %struct.test07*
!32 = !{!"R", %struct.test07 zeroinitializer, i32 0}  ; %struct.test07
!33 = !{!"F", i1 false, i32 1, !5, !34}  ; void (i8*)
!34 = !{i8 0, i32 1}  ; i8*
!35 = !{!33, i32 1}  ; void (i8*)*
!36 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01* }
!37 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!38 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !9} ; { i64, [0 x i8] }
!39 = !{!"S", %struct.test04inner zeroinitializer, i32 3, !1, !14, !15} ; { i64, i32, i64* }
!40 = !{!"S", %struct.test04mid zeroinitializer, i32 3, !1, !1, !16} ; { i64, i64, %struct.test04inner }
!41 = !{!"S", %struct.test04outer zeroinitializer, i32 2, !15, !17} ; { i64*, %struct.test04mid }
!42 = !{!"S", %struct.test05inner zeroinitializer, i32 3, !1, !14, !15} ; { i64, i32, i64* }
!43 = !{!"S", %struct.test05mid zeroinitializer, i32 3, !1, !1, !21} ; { i64, i64, %struct.test05inner }
!44 = !{!"S", %struct.test05outer zeroinitializer, i32 2, !15, !22} ; { i64*, %struct.test05mid }
!45 = !{!"S", %struct.test06 zeroinitializer, i32 2, !14, !26} ; { i32, [20 x i64] }
!46 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!47 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!48 = !{!"S", %struct.test09 zeroinitializer, i32 2, !35, !34} ; { void (i8*)*, i8* }

!dtrans_types = !{!36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48}
