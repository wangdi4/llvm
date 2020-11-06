; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test the special case where a structure begins with an array of i8 elements,
; whose address is passed to memset. This case is special because the address of
; the array is the same as the address of the entire structure and the type of a
; GEP to the start of the array element matches the type expected for parameter to
; memset. When opaque pointers are introduced this pattern may be seen for other
; types of array elements.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Test with memset that writes the entire structure, starting from the GEP of
; array element 0.
%struct.test01 = type { [200 x i8], i32, i32, i32, i32 }
@var01 = internal global %struct.test01 zeroinitializer
define void @test01(%struct.test01* %pStruct) !dtrans_type !4 {
  call void @llvm.memset.p0i8.i64(i8* getelementptr (%struct.test01, %struct.test01* @var01, i64 0, i32 0, i32 0),
                                  i8 1, i64 216, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: 0)Field LLVM Type: [200 x i8]
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: Global instance{{ *$}}


; Test with memset that just writes the array elements.
%struct.test02 = type { [200 x i8], i32, i32, i32, i32 }
@var02 = internal global %struct.test02 zeroinitializer
define void @test02(%struct.test02* %pStruct) !dtrans_type !8 {
  call void @llvm.memset.p0i8.i64(i8* getelementptr (%struct.test02, %struct.test02* @var02, i64 0, i32 0, i32 0),
                                  i8 1, i64 200, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: 0)Field LLVM Type: [200 x i8]
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Global instance | Memfunc partial write{{ *$}}


; Test with memset that only writes some of the array elements. This is not
; supported by DTrans because the analysis only supports complete field writes.
%struct.test03 = type { [200 x i8], i32, i32, i32, i32 }
@var03 = internal global %struct.test03 zeroinitializer
define void @test03(%struct.test03* %pStruct) !dtrans_type !11 {
  call void @llvm.memset.p0i8.i64(i8* getelementptr (%struct.test03, %struct.test03* @var03, i64 0, i32 0, i32 0),
                                  i8 1, i64 100, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: Global instance | Bad memfunc size{{ *$}}


declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

!1 = !{!"A", i32 200, !2}  ; [200 x i8]
!2 = !{i8 0, i32 0}  ; i8
!3 = !{i32 0, i32 0}  ; i32
!4 = !{!"F", i1 false, i32 1, !5, !6}  ; void (%struct.test01*)
!5 = !{!"void", i32 0}  ; void
!6 = !{!7, i32 1}  ; %struct.test01*
!7 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!8 = !{!"F", i1 false, i32 1, !5, !9}  ; void (%struct.test02*)
!9 = !{!10, i32 1}  ; %struct.test02*
!10 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!11 = !{!"F", i1 false, i32 1, !5, !12}  ; void (%struct.test03*)
!12 = !{!13, i32 1}  ; %struct.test03*
!13 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!14 = !{!"F", i1 false, i32 4, !5, !15, !2, !16, !17}  ; void (i8*, i8, i64, i1)
!15 = !{i8 0, i32 1}  ; i8*
!16 = !{i64 0, i32 0}  ; i64
!17 = !{i1 0, i32 0}  ; i1
!18 = !{!"S", %struct.test01 zeroinitializer, i32 5, !1, !3, !3, !3, !3} ; { [200 x i8], i32, i32, i32, i32 }
!19 = !{!"S", %struct.test02 zeroinitializer, i32 5, !1, !3, !3, !3, !3} ; { [200 x i8], i32, i32, i32, i32 }
!20 = !{!"S", %struct.test03 zeroinitializer, i32 5, !1, !3, !3, !3, !3} ; { [200 x i8], i32, i32, i32, i32 }
!21 = !{!"llvm.memset.p0i8.i64", !14}

!dtrans_types = !{!18, !19, !20}
!dtrans_decl_types = !{!21}
