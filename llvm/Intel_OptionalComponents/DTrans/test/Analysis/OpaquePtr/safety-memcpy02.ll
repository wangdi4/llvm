; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where an entire structure is written with a call to memcpy by
; passing a pointer to the structure to the memcpy call.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; Copy the entire structure.
%struct.test01 = type { i32, i16, i8 }
define void @test01(ptr "intel_dtrans_func_index"="1" %pStruct1, ptr "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !5 {
  call void @llvm.memcpy.p0.p0.i64(ptr %pStruct1, ptr %pStruct2, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; Copy that uses a multiple of the structure size, such as for an array of structures.
%struct.test02 = type { i32, i16, i8 }
define void @test02(ptr "intel_dtrans_func_index"="1" %pStruct1, ptr "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !7 {
  call void @llvm.memcpy.p0.p0.i64(ptr %pStruct1, ptr %pStruct2, i64 80, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


; Copy that a multiple of the structure size, such as for an array of
; structures, where the size is not a compile time constant, but the multiplier
; is a multiple of the structure size.
%struct.test03 = type { i32, i16, i8 }
define void @test03(ptr "intel_dtrans_func_index"="1" %pStruct1, ptr "intel_dtrans_func_index"="2" %pStruct2, i32 %n) !intel.dtrans.func.type !9 {
  %conv = sext i32 %n to i64
  %mul = mul i64 %conv, 8
  call void @llvm.memcpy.p0.p0.i64(ptr %pStruct1, ptr %pStruct2, i64 %mul, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03


; Copy a structure, where the size of the copy cannot be resolved to a multiple
; of the structure size.
%struct.test04 = type { i32, i16, i8 }
define void @test04(ptr "intel_dtrans_func_index"="1" %pStruct1, ptr "intel_dtrans_func_index"="2" %pStruct2, i32 %n, i64 %sz) !intel.dtrans.func.type !11 {
  %conv = sext i32 %n to i64
  %mul = mul i64 %conv, %sz
  call void @llvm.memcpy.p0.p0.i64(ptr %pStruct1, ptr %pStruct2, i64 %mul, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: Bad memfunc size{{ *$}}
; CHECK: End LLVMType: %struct.test04


; This test checks when a structure is composed of structures. Each of the
; the structure fields should be marked as written.
%struct.test05a = type { i16, i32, [2 x i32] }
%struct.test05b = type { i32, i16, i8 }
%struct.test05c = type { %struct.test05a, %struct.test05b }
define void @test05(ptr "intel_dtrans_func_index"="1" %pStruct1, ptr "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !16 {
  call void @llvm.memcpy.p0.p0.i64(ptr %pStruct1, ptr %pStruct2, i64 24, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05a
; CHECK: 0)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: [2 x i32]
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05c
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05c


; This test checks when a structure is composed of pointers to structures.
; This is a safe use, and only members of struct.test06c should be marked
; with 'Written'.
%struct.test06a = type { i16, i32, [2 x i32] }
%struct.test06b = type { i32, i16, i8 }
%struct.test06c = type { ptr, ptr }
define void @test06(ptr "intel_dtrans_func_index"="1" %pStruct1, ptr "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !20 {
  call void @llvm.memcpy.p0.p0.i64(ptr %pStruct1, ptr %pStruct2, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06a
; CHECK: 0)Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: [2 x i32]
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test06a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test06b

; Not including the actual types of the pointer fields in check lines, because
; with opaque pointers they will just be 'ptr'
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06c
; CHECK: 0)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test06c


; Copy a subset of the structure using a pointer to the start of the structure.
%struct.test07 = type { i32, i32, i32, i32 }
define void @test07(ptr "intel_dtrans_func_index"="1" %pStruct1, ptr "intel_dtrans_func_index"="2" %pStruct2) !intel.dtrans.func.type !22 {
  call void @llvm.memcpy.p0.p0.i64(ptr %pStruct1, ptr %pStruct2, i64 8, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test07
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written ComplexUse
; CHECK: 2)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 3)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Memfunc partial write{{ *$}}
; CHECK: End LLVMType: %struct.test07

declare !intel.dtrans.func.type !24 void @llvm.memcpy.p0.p0.i64(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", i64, i1)


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{i8 0, i32 0}  ; i8
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4, !4}
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!6, !6}
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8, !8}
!10 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!11 = distinct !{!10, !10}
!12 = !{!"A", i32 2, !1}  ; [2 x i32]
!13 = !{%struct.test05a zeroinitializer, i32 0}  ; %struct.test05a
!14 = !{%struct.test05b zeroinitializer, i32 0}  ; %struct.test05b
!15 = !{%struct.test05c zeroinitializer, i32 1}  ; %struct.test05c*
!16 = distinct !{!15, !15}
!17 = !{%struct.test06a zeroinitializer, i32 1}  ; %struct.test06a*
!18 = !{%struct.test06b zeroinitializer, i32 1}  ; %struct.test06b*
!19 = !{%struct.test06c zeroinitializer, i32 1}  ; %struct.test06c*
!20 = distinct !{!19, !19}
!21 = !{%struct.test07 zeroinitializer, i32 1}  ; %struct.test07*
!22 = distinct !{!21, !21}
!23 = !{i8 0, i32 1}  ; i8*
!24 = distinct !{!23, !23}
!25 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!26 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!27 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!28 = !{!"S", %struct.test04 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!29 = !{!"S", %struct.test05a zeroinitializer, i32 3, !2, !1, !12} ; { i16, i32, [2 x i32] }
!30 = !{!"S", %struct.test05b zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!31 = !{!"S", %struct.test05c zeroinitializer, i32 2, !13, !14} ; { %struct.test05a, %struct.test05b }
!32 = !{!"S", %struct.test06a zeroinitializer, i32 3, !2, !1, !12} ; { i16, i32, [2 x i32] }
!33 = !{!"S", %struct.test06b zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!34 = !{!"S", %struct.test06c zeroinitializer, i32 2, !17, !18} ; { %struct.test06a*, %struct.test06b* }
!35 = !{!"S", %struct.test07 zeroinitializer, i32 4, !1, !1, !1, !1} ; { i32, i32, i32, i32 }

!intel.dtrans.types = !{!25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35}
