; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test cases where a store uses a pointer to the start of a structure, but
; stores a type that does not match the type that the structure begins with.
; These cases are storing a pointer type that does not match the element zero
; type of a nested structure

%struct.test01a = type { %struct.test01b, %struct.test01b, %struct.test01b }
%struct.test01b = type { i32*, i32*, i32* }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !4 {
  %mem = call i8* @malloc(i64 128)
  %pStruct.as.pp8 = bitcast %struct.test01a* %pStruct to i8**
  store i8* %mem, i8** %pStruct.as.pp8
  ret void
}
; This case gets treated as safe by DTrans because the i8* will be compatible
; with the element zero type of the structure.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b


%struct.test02a = type{ %struct.test02b, %struct.test02b, %struct.test02b }
%struct.test02b = type { i32*, i32*, i32* }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %local = alloca i16
  %pStruct.as.pp16 = bitcast %struct.test02a* %pStruct to i16**
  store i16* %local, i16** %pStruct.as.pp16
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b


%struct.test03a = type{ %struct.test03b, %struct.test03b, %struct.test03b }
%struct.test03b = type { i32*, i32*, i32* }
define void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !10 {
  %local = alloca i64
  %pStruct.as.pp64 = bitcast %struct.test03a* %pStruct to i64**
  store i64* %local, i64** %pStruct.as.pp64
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test03b


%struct.test04a = type { %struct.test04b, %struct.test04b, %struct.test04b }
%struct.test04b = type { %struct.test04b*, i32*, i32* }
%struct.test04c = type { %struct.test04c*, i16* }
define void @test04(%struct.test04a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !16 {
  %local = alloca %struct.test04c
  %pStruct.as.ppS4c = bitcast %struct.test04a* %pStruct to %struct.test04c**
  store %struct.test04c* %local, %struct.test04c** %pStruct.as.ppS4c
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: Safety data: Bad casting | Unsafe pointer store | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Bad casting | Unsafe pointer store | Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test04b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04c
; CHECK: Safety data: Bad casting | Unsafe pointer store | Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test04c

declare !intel.dtrans.func.type !18 "intel_dtrans_func_index"="1" i8* @malloc(i64)


!1 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!4 = distinct !{!3}
!5 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = distinct !{!6}
!8 = !{%struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!9 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!10 = distinct !{!9}
!11 = !{%struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!12 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!13 = !{%struct.test04c zeroinitializer, i32 1}  ; %struct.test04c*
!14 = !{i16 0, i32 1}  ; i16*
!15 = !{%struct.test04a zeroinitializer, i32 1}  ; %struct.test04a*
!16 = distinct !{!15}
!17 = !{i8 0, i32 1}  ; i8*
!18 = distinct !{!17}
!19 = !{!"S", %struct.test01a zeroinitializer, i32 3, !1, !1, !1} ; { %struct.test01b, %struct.test01b, %struct.test01b }
!20 = !{!"S", %struct.test01b zeroinitializer, i32 3, !2, !2, !2} ; { i32*, i32*, i32* }
!21 = !{!"S", %struct.test02a zeroinitializer, i32 3, !5, !5, !5} ; { %struct.test02b, %struct.test02b, %struct.test02b }
!22 = !{!"S", %struct.test02b zeroinitializer, i32 3, !2, !2, !2} ; { i32*, i32*, i32* }
!23 = !{!"S", %struct.test03a zeroinitializer, i32 3, !8, !8, !8} ; { %struct.test03b, %struct.test03b, %struct.test03b }
!24 = !{!"S", %struct.test03b zeroinitializer, i32 3, !2, !2, !2} ; { i32*, i32*, i32* }
!25 = !{!"S", %struct.test04a zeroinitializer, i32 3, !11, !11, !11} ; { %struct.test04b, %struct.test04b, %struct.test04b }
!26 = !{!"S", %struct.test04b zeroinitializer, i32 3, !12, !2, !2} ; { %struct.test04b*, i32*, i32* }
!27 = !{!"S", %struct.test04c zeroinitializer, i32 2, !13, !14} ; { %struct.test04c*, i16* }

!intel.dtrans.types = !{!19, !20, !21, !22, !23, !24, !25, !26, !27}
