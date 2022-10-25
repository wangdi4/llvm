; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-outofboundsok=false -disable-output %s 2>&1 | FileCheck %s

; Test cases where an entire structure is written with a call to memset by
; passing a pointer to the structure to the memset call.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This test checks that all the structure fields are marked as "Written" by the
; memset call.
%struct.test01 = type { i32, i16, i8 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !5 {
  %p = bitcast %struct.test01* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 8, i1 false)
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


; This test checks when a multiple of the structure size is used, such as
; for an array of structures.
%struct.test02 = type { i32, i16, i8 }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %a) !intel.dtrans.func.type !7 {
  %p = bitcast %struct.test02* %a to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 32, i1 false)
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


; This test checks when a multiple of the structure size is used, such as for an
; array of structures, but the size is not a compile time constant.
%struct.test03 = type { i32, i16, i8 }
define void @test03(%struct.test03* "intel_dtrans_func_index"="1" %a, i32 %n) !intel.dtrans.func.type !9 {
  %p = bitcast %struct.test03* %a to i8*
  %conv = sext i32 %n to i64
  %mul = mul i64 %conv, 8
  tail call void @llvm.memset.p0i8.i64(i8* %p, i8 0, i64 %mul, i1 false)
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


; This test checks when a structure is composed of structures. Each of the
; the structure fields should be marked as written.
%struct.test04a = type { i16, i32, [2 x i32] }
%struct.test04b = type { i32, i16, i8 }
%struct.test04c = type { %struct.test04a, %struct.test04b }
define void @test04(%struct.test04c* "intel_dtrans_func_index"="1" %c) !intel.dtrans.func.type !14 {
  %c0 = bitcast %struct.test04c* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %c0, i8 0, i64 24, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: 0)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: [2 x i32]
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info: Written
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info: Written
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04c
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04c


; This test checks when a structure is composed of pointers to structures.
; This is a safe use, and only members of struct.test05c should be marked
; with 'Written'.
%struct.test05a = type { i16, i32, [2 x i32] }
%struct.test05b = type { i32, i16, i8 }
%struct.test05c = type { %struct.test05a*, %struct.test05b* }
define void @test05(%struct.test05c* "intel_dtrans_func_index"="1" %c) !intel.dtrans.func.type !18 {
  %c0 = bitcast %struct.test05c* %c to i8*
  tail call void @llvm.memset.p0i8.i64(i8* %c0, i8 0, i64 16, i1 false)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05a
; CHECK: 0)Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: [2 x i32]
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test05a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05b
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i16
; CHECK: Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i8
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test05b

; Not including the actual types of the pointer fields in check lines, because
; with opaque pointers they will just be 'p0'
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05c
; CHECK: 0)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: 1)Field LLVM Type:
; CHECK: Field info: Written
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test05c


declare !intel.dtrans.func.type !20 void @llvm.memset.p0i8.i64(i8* "intel_dtrans_func_index"="1", i8, i64, i1)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i16 0, i32 0}  ; i16
!3 = !{i8 0, i32 0}  ; i8
!4 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!5 = distinct !{!4}
!6 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!7 = distinct !{!6}
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8}
!10 = !{!"A", i32 2, !1}  ; [2 x i32]
!11 = !{%struct.test04a zeroinitializer, i32 0}  ; %struct.test04a
!12 = !{%struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!13 = !{%struct.test04c zeroinitializer, i32 1}  ; %struct.test04c*
!14 = distinct !{!13}
!15 = !{%struct.test05a zeroinitializer, i32 1}  ; %struct.test05a*
!16 = !{%struct.test05b zeroinitializer, i32 1}  ; %struct.test05b*
!17 = !{%struct.test05c zeroinitializer, i32 1}  ; %struct.test05c*
!18 = distinct !{!17}
!19 = !{i8 0, i32 1}  ; i8*
!20 = distinct !{!19}
!21 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!22 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!23 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!24 = !{!"S", %struct.test04a zeroinitializer, i32 3, !2, !1, !10} ; { i16, i32, [2 x i32] }
!25 = !{!"S", %struct.test04b zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!26 = !{!"S", %struct.test04c zeroinitializer, i32 2, !11, !12} ; { %struct.test04a, %struct.test04b }
!27 = !{!"S", %struct.test05a zeroinitializer, i32 3, !2, !1, !10} ; { i16, i32, [2 x i32] }
!28 = !{!"S", %struct.test05b zeroinitializer, i32 3, !1, !2, !3} ; { i32, i16, i8 }
!29 = !{!"S", %struct.test05c zeroinitializer, i32 2, !15, !16} ; { %struct.test05a*, %struct.test05b* }

!intel.dtrans.types = !{!21, !22, !23, !24, !25, !26, !27, !28, !29}
