; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test memory allocations that are safe for DTrans

; Allocation of pointer-to-pointer
%struct.test00 = type { i32, i32, i32 }
@var_test00 = internal global ptr zeroinitializer, !intel_dtrans_type !2
define internal void @test00() {
  ; Allocate with something other than a multiple of the structure size to
  ; verify that the structure is not marked with "Bad alloc size"
  %mem_i8 = call ptr @malloc(i64 64)
  store ptr %mem_i8, ptr @var_test00
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test00
; CHECK: Safety data: Global pointer{{ *$}}
; CHECK: End LLVMType: %struct.test00


; Allocation using malloc that is resolved based on location stored to.
%struct.test01member = type { i64, i64 }
%struct.test01 = type { ptr }
@var_test01 = internal global %struct.test01 zeroinitializer
define internal void @test01() {
  %mem_i8 = call ptr @malloc(i64 16)
  store ptr %mem_i8, ptr getelementptr (%struct.test01, ptr @var_test01, i64 0, i32 0)
  ret void
}

define internal void @test01f() {
  %mem = load ptr, ptr getelementptr (%struct.test01, ptr @var_test01, i64 0, i32 0)
  call void @free(ptr %mem)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test01

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01member
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01member


; Allocation using malloc that is resolved based on function return type metadata.
%struct.test02 = type { i32, i32 }
define "intel_dtrans_func_index"="1" ptr @test02() !intel.dtrans.func.type !6 {
  %mem = call ptr @malloc(i64 8)
  ret ptr %mem
}

define void @test02f(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  call void @free(ptr %pStruct)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


; Allocation using calloc
%struct.test03 = type { ptr }
define internal void @test03(ptr "intel_dtrans_func_index"="1" %in, i64 %index) !intel.dtrans.func.type !9 {
  %mem_i8 = call ptr @calloc(i64 16, i64 8)
  %array_elem = getelementptr ptr, ptr %mem_i8, i64 %index
  store ptr %in, ptr %array_elem

  call void @free(ptr %mem_i8)
  store ptr null, ptr %array_elem
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03


; Allocation using realloc
%struct.test04 = type { i64, i64 }
define internal "intel_dtrans_func_index"="1" ptr @test04(ptr "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !11 {
  %out_i8 = call ptr @realloc(ptr %in, i64 256)
  ret ptr %out_i8
}

define void @test04f(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !12 {
  call void @free(ptr %in)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test04


; Allocation using new
%struct.test05 = type { i64, i64 }
define "intel_dtrans_func_index"="1" ptr @test05() !intel.dtrans.func.type !14 {
  %mem = call ptr @_Znwm(i64 16)
  ret ptr %mem
}

define void @test05f(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !15 {
  call void @_ZdlPv(ptr %pStruct)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05
; CHECK: Safety data: Has C++ handling{{ *$}}
; CHECK: End LLVMType: %struct.test05


; Allocation using new[]
%struct.test06 = type { i64, i64 }
define "intel_dtrans_func_index"="1" ptr @test06(i64 %n) !intel.dtrans.func.type !17 {
  %bytes = mul i64 %n, 16
  %mem = call ptr @_Znam(i64 %bytes)
  ret ptr %mem
}

define void @test06f(ptr "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !18 {
  call void @_ZdaPv(ptr %pStruct)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06
; CHECK: Safety data: Has C++ handling{{ *$}}
; CHECK: End LLVMType: %struct.test06


; Test an allocation that is not used for the structure type, but instead
; for an i32* that is stored within the structure. This does not cause a
; safety flag to be set on the structure.
%struct.test07 = type { ptr }
@var07 = internal global %struct.test07 zeroinitializer
define void @test07() {
  ; Allocation that is not a multiple of the element size
  %mem = call ptr @malloc(i64 18)
  %field.addr = getelementptr %struct.test07, ptr @var07, i64 0, i32 0
  store ptr %mem, ptr %field.addr
  ret void
}

define void @test07f() {
  %field.addr = getelementptr %struct.test07, ptr @var07, i64 0, i32 0
  %mem = load ptr, ptr %field.addr
  call void @free(ptr %mem)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test07
; CHECK: Safety data: Global instance{{ *$}}
; CHECK: End LLVMType: %struct.test07


declare !intel.dtrans.func.type !21 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !22 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #1
declare !intel.dtrans.func.type !23 "intel_dtrans_func_index"="1" ptr @realloc(ptr "intel_dtrans_func_index"="2", i64) #2
declare !intel.dtrans.func.type !24 void @free(ptr "intel_dtrans_func_index"="1") #3

declare !intel.dtrans.func.type !25 "intel_dtrans_func_index"="1" ptr @_Znwm(i64) ; new(unsigned long)
declare !intel.dtrans.func.type !26 void @_ZdlPv(ptr "intel_dtrans_func_index"="1") ; delete(void*)
declare !intel.dtrans.func.type !27 "intel_dtrans_func_index"="1" ptr @_Znam(i64) ;  new[](unsigned long)
declare !intel.dtrans.func.type !28 void @_ZdaPv(ptr "intel_dtrans_func_index"="1") ; delete(void*)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
attributes #2 = { allockind("realloc") allocsize(1) "alloc-family"="malloc" }
attributes #3 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test00 zeroinitializer, i32 2}  ; %struct.test00**
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.test01member zeroinitializer, i32 1}  ; %struct.test01member*
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8}
!10 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!11 = distinct !{!10, !10}
!12 = distinct !{!10}
!13 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!14 = distinct !{!13}
!15 = distinct !{!13}
!16 = !{%struct.test06 zeroinitializer, i32 1}  ; %struct.test06*
!17 = distinct !{!16}
!18 = distinct !{!16}
!19 = !{i32 0, i32 1}  ; i32*
!20 = !{i8 0, i32 1}  ; i8*
!21 = distinct !{!20}
!22 = distinct !{!20}
!23 = distinct !{!20, !20}
!24 = distinct !{!20}
!25 = distinct !{!20}
!26 = distinct !{!20}
!27 = distinct !{!20}
!28 = distinct !{!20}
!29 = !{!"S", %struct.test00 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!30 = !{!"S", %struct.test01member zeroinitializer, i32 2, !3, !3} ; { i64, i64 }
!31 = !{!"S", %struct.test01 zeroinitializer, i32 1, !4} ; { %struct.test01member* }
!32 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!33 = !{!"S", %struct.test03 zeroinitializer, i32 1, !8} ; { %struct.test03* }
!34 = !{!"S", %struct.test04 zeroinitializer, i32 2, !3, !3} ; { i64, i64 }
!35 = !{!"S", %struct.test05 zeroinitializer, i32 2, !3, !3} ; { i64, i64 }
!36 = !{!"S", %struct.test06 zeroinitializer, i32 2, !3, !3} ; { i64, i64 }
!37 = !{!"S", %struct.test07 zeroinitializer, i32 1, !19} ; { i32* }

!intel.dtrans.types = !{!29, !30, !31, !32, !33, !34, !35, !36, !37}
