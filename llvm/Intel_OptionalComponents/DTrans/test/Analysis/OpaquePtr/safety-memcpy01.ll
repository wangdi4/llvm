; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test the safety analysis for calls to memcpy that take the address of an
; element within a structure type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Copy the entire structure, starting from a GEP of field 0.
%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStructA, %struct.test01* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !3 {
  %pFieldA = getelementptr %struct.test01, %struct.test01* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test01, %struct.test01* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; Copy a subset of structure fields, starting from a GEP of field 0.
%struct.test02 = type { i32, i32, i32 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStructA, %struct.test02* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !5 {
  %pFieldA = getelementptr %struct.test02, %struct.test02* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test02, %struct.test02* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *}}
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test02


; Copy beyond the end of the structure, starting from a GEP of field 0.
%struct.test03 = type { i32, i32, i32 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %pStructA, %struct.test03* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !7 {
  %pFieldA = getelementptr %struct.test03, %struct.test03* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test03, %struct.test03* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test03


; Copy a subset of the structure, starting and ending on a field boundary.
%struct.test04 = type { i32, i32, i32, i32, i32 }
define void @test04(%struct.test04* "intel_dtrans_func_index"="1" %pStructA, %struct.test04* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !9 {
  %pFieldA = getelementptr %struct.test04, %struct.test04* %pStructA, i64 0, i32 2
  %pFieldB = getelementptr %struct.test04, %struct.test04* %pStructB, i64 0, i32 2
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 4)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test04


; Copy a subset of the structure, with a size that extends beyond the structure
; end.
%struct.test05 = type { i32, i32, i32, i32, i32 }
define void @test05(%struct.test05* "intel_dtrans_func_index"="1" %pStructA, %struct.test05* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !11 {
  %pFieldA = getelementptr %struct.test05, %struct.test05* %pStructA, i64 0, i32 2
  %pFieldB = getelementptr %struct.test05, %struct.test05* %pStructB, i64 0, i32 2
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test05


; Copy a subset of the structure, with a size that does not end on a field
; boundary.
%struct.test06 = type { i32, i32, i32, i32, i32 }
define void @test06(%struct.test06* "intel_dtrans_func_index"="1" %pStructA, %struct.test06* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !13 {
  %pFieldA = getelementptr %struct.test06, %struct.test06* %pStructA, i64 0, i32 2
  %pFieldB = getelementptr %struct.test06, %struct.test06* %pStructB, i64 0, i32 2
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 6, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test06


; Copy one set of fields within a structure to a different set of fields. This
; is not supported by DTrans as a simplification for what the transformations
; need to handle.
%struct.test07 = type { i32, i32, i32 }
define void @test07(%struct.test07* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !15 {
  %pFieldA = getelementptr %struct.test07, %struct.test07* %pStruct, i64 0, i32 0
  %pFieldB = getelementptr %struct.test07, %struct.test07* %pStruct, i64 0, i32 1
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test07
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test07

; Copy from one structure type to another. This is not supported by DTrans as a
; simplification for what the transformations need to handle.
%struct.test08a = type { i32, i32, i32 }
%struct.test08b = type { i32, i32, i32 }
define void @test08(%struct.test08a* "intel_dtrans_func_index"="1" %pStructA, %struct.test08b* "intel_dtrans_func_index"="2" %pStructB) !intel.dtrans.func.type !18 {
  %pFieldA = getelementptr %struct.test08a, %struct.test08a* %pStructA, i64 0, i32 0
  %pFieldB = getelementptr %struct.test08b, %struct.test08b* %pStructB, i64 0, i32 0
  %pDst = bitcast i32* %pFieldA to i8*
  %pSrc = bitcast i32* %pFieldB to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %pDst, i8* %pSrc, i64 12, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test08a
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test08a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test08b
; CHECK: Safety data: Bad memfunc manipulation{{ *$}}
; CHECK: End LLVMType: %struct.test08b


declare !intel.dtrans.func.type !20 void @llvm.memcpy.p0i8.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8* "intel_dtrans_func_index"="2", i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2, !2}
!4 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!5 = distinct !{!4, !4}
!6 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!7 = distinct !{!6, !6}
!8 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!9 = distinct !{!8, !8}
!10 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!11 = distinct !{!10, !10}
!12 = !{%struct.test06 zeroinitializer, i32 1}  ; %struct.test06*
!13 = distinct !{!12, !12}
!14 = !{%struct.test07 zeroinitializer, i32 1}  ; %struct.test07*
!15 = distinct !{!14}
!16 = !{%struct.test08a zeroinitializer, i32 1}  ; %struct.test08a*
!17 = !{%struct.test08b zeroinitializer, i32 1}  ; %struct.test08b*
!18 = distinct !{!16, !17}
!19 = !{i8 0, i32 1}  ; i8*
!20 = distinct !{!19, !19}
!21 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!22 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!23 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!24 = !{!"S", %struct.test04 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!25 = !{!"S", %struct.test05 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!26 = !{!"S", %struct.test06 zeroinitializer, i32 5, !1, !1, !1, !1, !1} ; { i32, i32, i32, i32, i32 }
!27 = !{!"S", %struct.test07 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!28 = !{!"S", %struct.test08a zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!29 = !{!"S", %struct.test08b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!21, !22, !23, !24, !25, !26, !27, !28, !29}
