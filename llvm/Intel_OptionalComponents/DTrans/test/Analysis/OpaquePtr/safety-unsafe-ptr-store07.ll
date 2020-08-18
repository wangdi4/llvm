; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where a store uses a pointer to the start of a structure, but
; stores a type that does not match the type that the structure begins with.
; These cases are storing a pointer type that does not match the element zero
; type of a nested structure

%struct.test01a = type { %struct.test01b, %struct.test01b, %struct.test01b }
%struct.test01b = type { i32*, i32*, i32* }
define void @test01(%struct.test01a* %pStruct) !dtrans_type !3 {
  %mem = call i8* @malloc(i64 128)
  %pStruct.as.pp8 = bitcast %struct.test01a* %pStruct to i8**
  store i8* %mem, i8** %pStruct.as.pp8
  ret void
}
; This case gets treated as safe by DTrans because the i8* will be compatible
; with the element zero type of the structure.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01a
; CHECK: Safety data: Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01b
; CHECK: Safety data: Nested structure{{ *$}}


%struct.test02a = type{ %struct.test02b, %struct.test02b, %struct.test02b }
%struct.test02b = type { i32*, i32*, i32* }
define void @test02(%struct.test02a* %pStruct) !dtrans_type !8 {
  %local = alloca i16
  %pStruct.as.pp16 = bitcast %struct.test02a* %pStruct to i16**
  store i16* %local, i16** %pStruct.as.pp16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Nested structure{{ *$}}


%struct.test03a = type{ %struct.test03b, %struct.test03b, %struct.test03b }
%struct.test03b = type { i32*, i32*, i32* }
define void @test03(%struct.test03a* %pStruct) !dtrans_type !12 {
  %local = alloca i64
  %pStruct.as.pp64 = bitcast %struct.test03a* %pStruct to i64**
  store i64* %local, i64** %pStruct.as.pp64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Nested structure{{ *$}}


%struct.test04a = type { %struct.test04b, %struct.test04b, %struct.test04b }
%struct.test04b = type { %struct.test04b*, i32*, i32* }
%struct.test04c = type { %struct.test04c*, i16* }
define void @test04(%struct.test04a* %pStruct) !dtrans_type !20 {
  %local = alloca %struct.test04c
  %pStruct.as.ppS4c = bitcast %struct.test04a* %pStruct to %struct.test04c**
  store %struct.test04c* %local, %struct.test04c** %pStruct.as.ppS4c
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Contains nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Nested structure{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04c
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}

declare i8* @malloc(i64)


!1 = !{!"R", %struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{!"F", i1 false, i32 1, !4, !5}  ; void (%struct.test01a*)
!4 = !{!"void", i32 0}  ; void
!5 = !{!6, i32 1}  ; %struct.test01a*
!6 = !{!"R", %struct.test01a zeroinitializer, i32 0}  ; %struct.test01a
!7 = !{!"R", %struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!8 = !{!"F", i1 false, i32 1, !4, !9}  ; void (%struct.test02a*)
!9 = !{!10, i32 1}  ; %struct.test02a*
!10 = !{!"R", %struct.test02a zeroinitializer, i32 0}  ; %struct.test02a
!11 = !{!"R", %struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!12 = !{!"F", i1 false, i32 1, !4, !13}  ; void (%struct.test03a*)
!13 = !{!14, i32 1}  ; %struct.test03a*
!14 = !{!"R", %struct.test03a zeroinitializer, i32 0}  ; %struct.test03a
!15 = !{!"R", %struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!16 = !{!15, i32 1}  ; %struct.test04b*
!17 = !{!18, i32 1}  ; %struct.test04c*
!18 = !{!"R", %struct.test04c zeroinitializer, i32 0}  ; %struct.test04c
!19 = !{i16 0, i32 1}  ; i16*
!20 = !{!"F", i1 false, i32 1, !4, !21}  ; void (%struct.test04a*)
!21 = !{!22, i32 1}  ; %struct.test04a*
!22 = !{!"R", %struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!23 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b, %struct.test01b, %struct.test01b }
!24 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32*, i32*, i32* }
!25 = !{!"S", %struct.test02a zeroinitializer, i32 3, !7, !7, !7} ; { %struct.test02b, %struct.test02b, %struct.test02b }
!26 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32*, i32*, i32* }
!27 = !{!"S", %struct.test03a zeroinitializer, i32 3, !11, !11, !11} ; { %struct.test03b, %struct.test03b, %struct.test03b }
!28 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32*, i32*, i32* }
!29 = !{!"S", %struct.test04a zeroinitializer, i32 3, !15, !15, !15} ; { %struct.test04b, %struct.test04b, %struct.test04b }
!30 = !{!"S", %struct.test04b zeroinitializer, i32 3, !16, !2, !2} ; { %struct.test04b*, i32*, i32* }
!31 = !{!"S", %struct.test04c zeroinitializer, i32 2, !17, !19} ; { %struct.test04c*, i16* }

!dtrans_types = !{!23, !24, !25, !26, !27, !28, !29, !30, !31}
