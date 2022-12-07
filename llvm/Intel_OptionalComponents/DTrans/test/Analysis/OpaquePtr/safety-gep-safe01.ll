; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that safe GEP uses do not get marked with safety flags by the checks
; for 'Ambiguous GEP' or 'Bad pointer manipulation'

; Test accesses for simple GEPs.
%struct.test01 = type { i64, ptr }
define internal void @test01(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !3 {
  %f0 = getelementptr %struct.test01, ptr %in, i64 0, i32 0
  %v0 = load i64, ptr %f0

  %f1 = getelementptr %struct.test01, ptr %in, i64 0, i32 1
  %v1 = load ptr, ptr %f1

  %f2 = getelementptr %struct.test01, ptr %v1, i64 0, i32 0
  %v2 = load i64, ptr %f2

  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


; Test a GEP that is used for a pointer-to-pointer, followed by a safe
; field access.
%struct.test02 = type { i64, i64 }
define internal void @test02(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  %p2p = getelementptr ptr, ptr %in, i64 5
  %ptr = load ptr, ptr %p2p
  %field_addr = getelementptr %struct.test02, ptr %ptr, i64 0, i32 1
  store i64 0, ptr %field_addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


; Test handling for a zero-sized array element at the end of the structure.
%struct.test03 = type { i64, [0 x i8] }
define void @test03(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !9 {
  %f1 = getelementptr %struct.test03, ptr %in, i64 0, i32 1, i32 2
  %v8 = load i8, ptr %f1
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: Has zero-sized array{{ *$}}
; CHECK: End LLVMType: %struct.test03


; Test access with nested structure element using multiple GEPs to traverse structure
%struct.test04inner = type { i64, i32, ptr }
%struct.test04mid = type { i64, i64, %struct.test04inner }
%struct.test04outer = type { ptr, %struct.test04mid }
define internal i64 @test04(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !15 {
  %outer.1 = getelementptr %struct.test04outer, ptr %in, i64 0, i32 1
  %mid.2 = getelementptr %struct.test04mid, ptr %outer.1, i64 0, i32 2
  %inner.0 = getelementptr %struct.test04inner, ptr %mid.2, i64 0, i32 0
  %val0 = load i64, ptr %inner.0

  %inner.2 = getelementptr %struct.test04inner, ptr %mid.2, i64 0, i32 2
  %ptr = load ptr, ptr %inner.2
  %val2 = load i64, ptr %ptr

  ret i64 %val0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04inner
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04inner

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04mid
; CHECK: Safety data: Nested structure | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04mid

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04outer
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04outer


; Test access with nested structure element using single GEP to traverse structure
%struct.test05inner = type { i64, i32, ptr }
%struct.test05mid = type { i64, i64, %struct.test05inner }
%struct.test05outer = type { ptr, %struct.test05mid }
define internal i64 @test05(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !19 {
  %addr0 = getelementptr %struct.test05outer, ptr %in, i64 0, i32 1, i32 2, i32 0
  %val0 = load i64, ptr %addr0
  %addr2 = getelementptr %struct.test05outer, ptr %in, i64 0, i32 1, i32 2, i32 2
  %ptr = load ptr, ptr %addr2
  %val2 = load i64, ptr %ptr
  ret i64 %val0
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05inner
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05inner

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05mid
; CHECK: Safety data: Nested structure | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05mid

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05outer
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test05outer


; Test access to an element of an array member of a structure.
%struct.test06 = type { i32, [20 x i64] }
define internal i64 @test06(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !22 {
  %elem_addr = getelementptr inbounds %struct.test06, ptr %in, i64 0, i32 1, i64 4
  %val = load i64, ptr %elem_addr
  ret i64 %val
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test06


; Test access to an element that is an array of pointers
%struct.test07 = type { i64, i64 }
@var07 = internal global [16 x ptr] zeroinitializer, !intel_dtrans_type !23
define internal void @test07() {
  %addr = getelementptr [16 x ptr], ptr @var07, i64 0, i32 2
  %sptr = load ptr, ptr %addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test07
; CHECK: Safety data: Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test07


; Test access to a structure member from an array of structures
%struct.test08 = type { i64, i64 }
@var08 = internal global [16 x %struct.test08] zeroinitializer
define internal void @test08() {
  %addr = getelementptr [16 x %struct.test08], ptr @var08, i64 0, i32 5, i32 1
  %sptr = load i64, ptr %addr
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test08
; CHECK: Safety data: Global instance | Global array{{ *$}}
; CHECK: End LLVMType: %struct.test08


; This is a special case of a GEP that uses a null value for the pointer
; operand.
%struct.test09 = type { ptr, ptr }
define internal void @test09(i64 %offset) {
  %null_offset = getelementptr %struct.test09, ptr null, i64 %offset
  %fptr_addr = getelementptr inbounds %struct.test09, ptr %null_offset, i64 0, i32 0
  %fptr = load ptr, ptr %fptr_addr

  %cptr_addr = getelementptr %struct.test09, ptr %null_offset, i64 0, i32 1
  %cptr = load ptr, ptr %cptr_addr
  call void %fptr(ptr %cptr), !intel_dtrans_type !25

 ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test09
; CHECK: Safety data: Has function ptr{{ *$}}
; CHECK: End LLVMType: %struct.test09


!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{%struct.test02 zeroinitializer, i32 2}  ; %struct.test02**
!5 = distinct !{!4}
!6 = !{!"A", i32 0, !7}  ; [0 x i8]
!7 = !{i8 0, i32 0}  ; i8
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8}
!10 = !{i32 0, i32 0}  ; i32
!11 = !{i64 0, i32 1}  ; i64*
!12 = !{%struct.test04inner zeroinitializer, i32 0}  ; %struct.test04inner
!13 = !{%struct.test04mid zeroinitializer, i32 0}  ; %struct.test04mid
!14 = !{%struct.test04outer zeroinitializer, i32 1}  ; %struct.test04outer*
!15 = distinct !{!14}
!16 = !{%struct.test05inner zeroinitializer, i32 0}  ; %struct.test05inner
!17 = !{%struct.test05mid zeroinitializer, i32 0}  ; %struct.test05mid
!18 = !{%struct.test05outer zeroinitializer, i32 1}  ; %struct.test05outer*
!19 = distinct !{!18}
!20 = !{!"A", i32 20, !1}  ; [20 x i64]
!21 = !{%struct.test06 zeroinitializer, i32 1}  ; %struct.test06*
!22 = distinct !{!21}
!23 = !{!"A", i32 16, !24}  ; [16 x %struct.test07*]
!24 = !{%struct.test07 zeroinitializer, i32 1}  ; %struct.test07*
!25 = !{!"F", i1 false, i32 1, !26, !27}  ; void (i8*)
!26 = !{!"void", i32 0}  ; void
!27 = !{i8 0, i32 1}  ; i8*
!28 = !{!25, i32 1}  ; void (i8*)*
!29 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test01* }
!30 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!31 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !6} ; { i64, [0 x i8] }
!32 = !{!"S", %struct.test04inner zeroinitializer, i32 3, !1, !10, !11} ; { i64, i32, i64* }
!33 = !{!"S", %struct.test04mid zeroinitializer, i32 3, !1, !1, !12} ; { i64, i64, %struct.test04inner }
!34 = !{!"S", %struct.test04outer zeroinitializer, i32 2, !11, !13} ; { i64*, %struct.test04mid }
!35 = !{!"S", %struct.test05inner zeroinitializer, i32 3, !1, !10, !11} ; { i64, i32, i64* }
!36 = !{!"S", %struct.test05mid zeroinitializer, i32 3, !1, !1, !16} ; { i64, i64, %struct.test05inner }
!37 = !{!"S", %struct.test05outer zeroinitializer, i32 2, !11, !17} ; { i64*, %struct.test05mid }
!38 = !{!"S", %struct.test06 zeroinitializer, i32 2, !10, !20} ; { i32, [20 x i64] }
!39 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!40 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!41 = !{!"S", %struct.test09 zeroinitializer, i32 2, !28, !27} ; { void (i8*)*, i8* }

!intel.dtrans.types = !{!29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41}
