; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test memory allocations that are safe for DTrans

; Allocation of pointer-to-pointer
%struct.test00 = type { i32, i32, i32 }
@var_test00 = internal global %struct.test00** zeroinitializer, !dtrans_type !2
define internal void @test00() {
  ; Allocate with something other than a multiple of the structure size to
  ; verify that the structure is not marked with "Bad alloc size"
  %mem_i8 = call i8* @malloc(i64 64)
  %ptrtoptr = bitcast i8* %mem_i8 to %struct.test00**
  store %struct.test00** %ptrtoptr, %struct.test00*** @var_test00
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test00
; CHECK: Safety data: Global pointer{{ *$}}


; Allocation using malloc that is resolved based on location stored to.
%struct.test01member = type { i64, i64 }
%struct.test01 = type { %struct.test01member* }
@var_test01 = internal global %struct.test01 zeroinitializer
define internal void @test01() {
  %mem_i8 = call i8* @malloc(i64 16)
  %mystruct = bitcast i8* %mem_i8 to %struct.test01member*
  store %struct.test01member* %mystruct, %struct.test01member** getelementptr (%struct.test01, %struct.test01* @var_test01, i64 0, i32 0)
  ret void
}

define internal void @test01f() {
  %mem = load %struct.test01member*, %struct.test01member** getelementptr (%struct.test01, %struct.test01* @var_test01, i64 0, i32 0)
  %mem.p8 = bitcast %struct.test01member* %mem to i8*
  call void @free(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: Safety data: Global instance{{ *$}}

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01member
; CHECK: Safety data: No issues found


; Allocation using malloc that is resolved based on function return type metadata.
%struct.test02 = type { i32, i32 }
define %struct.test02* @test02() !dtrans_type !7 {
  %mem = call i8* @malloc(i64 8)
  %mem.as.struct = bitcast i8* %mem to %struct.test02*
  ret %struct.test02* %mem.as.struct
}

define void @test02f(%struct.test02* %pStruct) !dtrans_type !10 {
  %mem.p8 = bitcast %struct.test02* %pStruct to i8*
  call void @free(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: Safety data: No issues found


; Allocation using calloc
%struct.test03 = type { %struct.test03* }
define internal void @test03(%struct.test03* %in, i64 %index) !dtrans_type !14 {
  %mem_i8 = call i8* @calloc(i64 16, i64 8)
  %array = bitcast i8* %mem_i8 to %struct.test03**
  %array_elem = getelementptr %struct.test03*, %struct.test03** %array, i64 %index
  store %struct.test03* %in, %struct.test03** %array_elem

  call void @free(i8* %mem_i8)
  store %struct.test03* null, %struct.test03** %array_elem
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03
; CHECK: Safety data: No issues found


; Allocation using realloc
%struct.test04 = type { i64, i64 }
define internal %struct.test04* @test04(%struct.test04* %in) !dtrans_type !15 {
  %in_i8 = bitcast %struct.test04* %in to i8*
  %out_i8 = call i8* @realloc(i8* %in_i8, i64 256)
  %out = bitcast i8* %out_i8 to %struct.test04*
  ret %struct.test04* %out
}

define void @test04f(%struct.test04* %in) !dtrans_type !18 {
  %mem.p8 = bitcast %struct.test04* %in to i8*
  call void @free(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04
; CHECK: Safety data: No issues found


; Allocation using new
%struct.test05 = type { i64, i64 }
define %struct.test05* @test05() !dtrans_type !19 {
  %mem = call i8* @_Znwm(i64 16)
  %mem.as.struct = bitcast i8* %mem to %struct.test05*
  ret %struct.test05* %mem.as.struct
}

define void @test05f(%struct.test05* %pStruct) !dtrans_type !22 {
  %mem.p8 = bitcast %struct.test05* %pStruct to i8*
  call void @_ZdlPv(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test05
; CHECK: Safety data: Has C++ handling{{ *$}}


; Allocation using new[]
%struct.test06 = type { i64, i64 }
define %struct.test06* @test06(i64 %n) !dtrans_type !23 {
  %bytes = mul i64 %n, 16
  %mem = call i8* @_Znam(i64 %bytes)
  %mem.as.struct = bitcast i8* %mem to %struct.test06*
  ret %struct.test06* %mem.as.struct
}

define void @test06f(%struct.test06* %pStruct) !dtrans_type !26 {
  %mem.p8 = bitcast %struct.test06* %pStruct to i8*
  call void @_ZdaPv(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test06
; CHECK: Safety data: Has C++ handling{{ *$}}


; Test an allocation that is not used for the structure type, but instead
; for an i32* that is stored within the structure. This does not cause a
; safety flag to be set on the structure.
%struct.test07 = type { i32* }
@var07 = internal global %struct.test07 zeroinitializer
define void @test07() {
  ; Allocation that is not a multiple of the element size
  %mem = call i8* @malloc(i64 18)
  %mem.as.p32 = bitcast i8* %mem to i32*
  %field.addr = getelementptr %struct.test07, %struct.test07* @var07, i64 0, i32 0
  store i32* %mem.as.p32, i32** %field.addr
  ret void
}

define void @test07f() {
  %field.addr = getelementptr %struct.test07, %struct.test07* @var07, i64 0, i32 0
  %mem = load i32*, i32** %field.addr
  %mem.as.p8 = bitcast i32* %mem to i8*
  call void @free(i8* %mem.as.p8)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test07
; CHECK: Safety data: Global instance{{ *$}}


declare i8* @malloc(i64)
declare i8* @calloc(i64, i64)
declare i8* @realloc(i8*, i64)
declare void @free(i8*)

declare i8* @_Znwm(i64) ; new(unsigned long)
declare void @_ZdlPv(i8*) ; delete(void*)
declare i8* @_Znam(i64) ;  new[](unsigned long)
declare void @_ZdaPv(i8*) ; delete(void*)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{!3, i32 2}  ; %struct.test00**
!3 = !{!"R", %struct.test00 zeroinitializer, i32 0}  ; %struct.test00
!4 = !{i64 0, i32 0}  ; i64
!5 = !{!6, i32 1}  ; %struct.test01member*
!6 = !{!"R", %struct.test01member zeroinitializer, i32 0}  ; %struct.test01member
!7 = !{!"F", i1 false, i32 0, !8}  ; %struct.test02* ()
!8 = !{!9, i32 1}  ; %struct.test02*
!9 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!10 = !{!"F", i1 false, i32 1, !11, !8}  ; void (%struct.test02*)
!11 = !{!"void", i32 0}  ; void
!12 = !{!13, i32 1}  ; %struct.test03*
!13 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!14 = !{!"F", i1 false, i32 2, !11, !12, !4}  ; void (%struct.test03*, i64)
!15 = !{!"F", i1 false, i32 1, !16, !16}  ; %struct.test04* (%struct.test04*)
!16 = !{!17, i32 1}  ; %struct.test04*
!17 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!18 = !{!"F", i1 false, i32 1, !11, !16}  ; void (%struct.test04*)
!19 = !{!"F", i1 false, i32 0, !20}  ; %struct.test05* ()
!20 = !{!21, i32 1}  ; %struct.test05*
!21 = !{!"R", %struct.test05 zeroinitializer, i32 0}  ; %struct.test05
!22 = !{!"F", i1 false, i32 1, !11, !20}  ; void (%struct.test05*)
!23 = !{!"F", i1 false, i32 1, !24, !4}  ; %struct.test06* (i64)
!24 = !{!25, i32 1}  ; %struct.test06*
!25 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!26 = !{!"F", i1 false, i32 1, !11, !24}  ; void (%struct.test06*)
!27 = !{i32 0, i32 1}  ; i32*
!28 = !{!"S", %struct.test00 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!29 = !{!"S", %struct.test01member zeroinitializer, i32 2, !4, !4} ; { i64, i64 }
!30 = !{!"S", %struct.test01 zeroinitializer, i32 1, !5} ; { %struct.test01member* }
!31 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!32 = !{!"S", %struct.test03 zeroinitializer, i32 1, !12} ; { %struct.test03* }
!33 = !{!"S", %struct.test04 zeroinitializer, i32 2, !4, !4} ; { i64, i64 }
!34 = !{!"S", %struct.test05 zeroinitializer, i32 2, !4, !4} ; { i64, i64 }
!35 = !{!"S", %struct.test06 zeroinitializer, i32 2, !4, !4} ; { i64, i64 }
!36 = !{!"S", %struct.test07 zeroinitializer, i32 1, !27} ; { i32* }

!dtrans_types = !{!28, !29, !30, !31, !32, !33, !34, !35, !36}
