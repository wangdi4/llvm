; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test the safety analysis for calls to memcpy that take the address of an
; element within a structure type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Copy the entire structure, starting from a GEP of field 0.
%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* %pStructA, %struct.test01* %pStructB) !dtrans_type !2 {
  %pFieldA = getelementptr %struct.test01, %struct.test01* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test01, %struct.test01* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: No issues found


; Copy a subset of structure fields, starting from a GEP of field 0.
%struct.test02 = type { i32, i32, i32 }
define void @test02(%struct.test02* %pStructA, %struct.test02* %pStructB) !dtrans_type !6 {
  %pFieldA = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test02, %struct.test02* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *}}
; CHECK: Safety data: Memfunc partial write{{ *$}}


; Copy beyond the end of the structure, starting from a GEP of field 0.
%struct.test03 = type { i32, i32, i32 }
define void @test03(%struct.test03* %pStructA, %struct.test03* %pStructB) !dtrans_type !9 {
  %pFieldA = getelementptr %struct.test03, %struct.test03* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test03, %struct.test03* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Bad memfunc size{{ *$}}


; Copy a subset of the structure, starting and ending on a field boundary.
%struct.test04 = type { i32, i32, i32, i32, i32 }
define void @test04(%struct.test04* %pStructA, %struct.test04* %pStructB) !dtrans_type !12 {
  %pFieldA = getelementptr %struct.test04, %struct.test04* %pStructA, i64 0, i32 2
  %pFieldB = getelementptr %struct.test04, %struct.test04* %pStructB, i64 0, i32 2
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}


; Copy a subset of the structure, with a size that extends beyond the structure
; end.
%struct.test05 = type { i32, i32, i32, i32, i32 }
define void @test05(%struct.test05* %pStructA, %struct.test05* %pStructB) !dtrans_type !15 {
  %pFieldA = getelementptr %struct.test05, %struct.test05* %pStructA, i64 0, i32 2
  %pFieldB = getelementptr %struct.test05, %struct.test05* %pStructB, i64 0, i32 2
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05
; CHECK: Safety data: Bad memfunc size{{ *$}}


; Copy a subset of the structure, with a size that does not end on a field
; boundary.
%struct.test06 = type { i32, i32, i32, i32, i32 }
define void @test06(%struct.test06* %pStructA, %struct.test06* %pStructB) !dtrans_type !18 {
  %pFieldA = getelementptr %struct.test06, %struct.test06* %pStructA, i64 0, i32 2
  %pFieldB = getelementptr %struct.test06, %struct.test06* %pStructB, i64 0, i32 2
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 6, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06
; CHECK: Safety data: Bad memfunc size{{ *$}}


; Copy one set of fields within a structure to a different set of fields. This
; is not supported by DTrans as a simplification for what the transformations
; need to handle.
%struct.test07 = type { i32, i32, i32 }
define void @test07(%struct.test07* %pStruct) !dtrans_type !21 {
  %pFieldA = getelementptr %struct.test07, %struct.test07* %pStruct, i64 0, i32 0
  %pFieldB = getelementptr %struct.test07, %struct.test07* %pStruct, i64 0, i32 1
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test07
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}

; Copy from one structure type to another. This is not supported by DTrans as a
; simplification for what the transformations need to handle.
%struct.test08a = type { i32, i32, i32 }
%struct.test08b = type { i32, i32, i32 }
define void @test08(%struct.test08a* %pStructA, %struct.test08b* %pStructB) !dtrans_type !24 {
  %pFieldA = getelementptr %struct.test08a, %struct.test08a* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test08b, %struct.test08b* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test08a
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test08b
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}


declare void @llvm.memcpy.p0i8.p0i8.i64(i8*, i8*, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 2, !3, !4, !4}  ; void (%struct.test01*, %struct.test01*)
!3 = !{!"void", i32 0}  ; void
!4 = !{!5, i32 1}  ; %struct.test01*
!5 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!6 = !{!"F", i1 false, i32 2, !3, !7, !7}  ; void (%struct.test02*, %struct.test02*)
!7 = !{!8, i32 1}  ; %struct.test02*
!8 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!9 = !{!"F", i1 false, i32 2, !3, !10, !10}  ; void (%struct.test03*, %struct.test03*)
!10 = !{!11, i32 1}  ; %struct.test03*
!11 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!12 = !{!"F", i1 false, i32 2, !3, !13, !13}  ; void (%struct.test04*, %struct.test04*)
!13 = !{!14, i32 1}  ; %struct.test04*
!14 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!15 = !{!"F", i1 false, i32 2, !3, !16, !16}  ; void (%struct.test05*, %struct.test05*)
!16 = !{!17, i32 1}  ; %struct.test05*
!17 = !{!"R", %struct.test05 zeroinitializer, i32 0}  ; %struct.test05
!18 = !{!"F", i1 false, i32 2, !3, !19, !19}  ; void (%struct.test06*, %struct.test06*)
!19 = !{!20, i32 1}  ; %struct.test06*
!20 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!21 = !{!"F", i1 false, i32 1, !3, !22}  ; void (%struct.test07*)
!22 = !{!23, i32 1}  ; %struct.test07*
!23 = !{!"R", %struct.test07 zeroinitializer, i32 0}  ; %struct.test07
!24 = !{!"F", i1 false, i32 2, !3, !25, !27}  ; void (%struct.test08a*, %struct.test08b*)
!25 = !{!26, i32 1}  ; %struct.test08a*
!26 = !{!"R", %struct.test08a zeroinitializer, i32 0}  ; %struct.test08a
!27 = !{!28, i32 1}  ; %struct.test08b*
!28 = !{!"R", %struct.test08b zeroinitializer, i32 0}  ; %struct.test08b
!29 = !{!"F", i1 false, i32 4, !3, !30, !31, !32, !33}  ; void (i8*, i8, i64, i1)
!30 = !{i8 0, i32 1}  ; i8*
!31 = !{i8 0, i32 0}  ; i8
!32 = !{i64 0, i32 0}  ; i64
!33 = !{i1 0, i32 0}  ; i1
!34 = !{!"F", i1 false, i32 4, !3, !30, !30, !32, !33}  ; void (i8*, i8*, i64, i1)
!35 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!36 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!37 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!38 = !{!"S", %struct.test04 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!39 = !{!"S", %struct.test05 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!40 = !{!"S", %struct.test06 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!41 = !{!"S", %struct.test07 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!42 = !{!"S", %struct.test08a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!43 = !{!"S", %struct.test08b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!44 = !{!"llvm.memcpy.p0i8.p0i8.i64", !34}

!dtrans_types = !{!35, !36, !37, !38, !39, !40, !41, !42, !43}
!dtrans_decl_types = !{!44}
