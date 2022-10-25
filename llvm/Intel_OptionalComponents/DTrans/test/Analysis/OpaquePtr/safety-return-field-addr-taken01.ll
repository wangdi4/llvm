; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-outofboundsok=true -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test that returning a pointer which is the address of a field results in the
; "Field address taken return" safety flag.

; Test with returning a field address
%struct.test01 = type { i32, i32 }
define "intel_dtrans_func_index"="1" i32* @test01(%struct.test01* "intel_dtrans_func_index"="2" %pStruct) !intel.dtrans.func.type !4 {
  %addr = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 1
  ret i32* %addr
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Field address taken return{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Test with returning a field from within a nested structure
%struct.test02a = type { i32, %struct.test02b }
%struct.test02b = type { i32, i32 }
define "intel_dtrans_func_index"="1" i32* @test02(%struct.test02a* "intel_dtrans_func_index"="2" %pStruct) !intel.dtrans.func.type !7 {
  %addr = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 1, i32 1
  ret i32* %addr
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: 0)Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Nested structure | Field address taken return{{ *$}}
; CHECK: End LLVMType: %struct.test02b


; Test with returning an address that is the nested structure's address
%struct.test03a = type { i32, %struct.test03b }
%struct.test03b = type { i32, i32 }
define "intel_dtrans_func_index"="1" %struct.test03b* @test03(%struct.test03a* "intel_dtrans_func_index"="2" %pStruct) !intel.dtrans.func.type !11 {
  %addr = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 1
  ret %struct.test03b* %addr
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: 0)Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: %struct.test03b
; CHECK: DTrans Type: %struct.test03b
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Contains nested structure | Field address taken return{{ *$}}
; CHECK: End LLVMType: %struct.test03a

; This is marked as "Field address taken return" due to "-dtrans-outofboundsok=true",
; otherwise it is safe.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: Safety data: Nested structure | Field address taken return{{ *$}}
; CHECK: End LLVMType: %struct.test03b


; Test with returning the address obtained using a GEPOperator
%struct.test04a = type { i32, %struct.test04b }
%struct.test04b = type { i32, i32 }
@var04 = internal global %struct.test04a zeroinitializer
define "intel_dtrans_func_index"="1" %struct.test04b* @test04() !intel.dtrans.func.type !14 {
  ret %struct.test04b* getelementptr (%struct.test04a, %struct.test04a* @var04, i64 0, i32 1)
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04a
; CHECK: 0)Field LLVM Type: i32
; CHECK: DTrans Type: i32
; CHECK-NEXT: Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: %struct.test04b
; CHECK: DTrans Type: %struct.test04b
; CHECK-NEXT: Field info: ComplexUse AddressTaken{{ *$}}
; CHECK: Safety data: Global instance | Contains nested structure | Field address taken return{{ *$}}
; CHECK: End LLVMType: %struct.test04a

; This is marked as "Field address taken return" due to "-dtrans-outofboundsok=true",
; otherwise it is safe.
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test04b
; CHECK: Safety data: Global instance | Nested structure | Field address taken return{{ *$}}
; CHECK: End LLVMType: %struct.test04b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i32 0, i32 1}  ; i32*
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!2, !3}
!5 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!6 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!7 = distinct !{!2, !6}
!8 = !{%struct.test03b zeroinitializer, i32 0}  ; %struct.test03b
!9 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!10 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!11 = distinct !{!9, !10}
!12 = !{%struct.test04b zeroinitializer, i32 0}  ; %struct.test04b
!13 = !{%struct.test04b zeroinitializer, i32 1}  ; %struct.test04b*
!14 = distinct !{!13}
!15 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!16 = !{!"S", %struct.test02a zeroinitializer, i32 2, !1, !5} ; { i32, %struct.test02b }
!17 = !{!"S", %struct.test02b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!18 = !{!"S", %struct.test03a zeroinitializer, i32 2, !1, !8} ; { i32, %struct.test03b }
!19 = !{!"S", %struct.test03b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!20 = !{!"S", %struct.test04a zeroinitializer, i32 2, !1, !12} ; { i32, %struct.test04b }
!21 = !{!"S", %struct.test04b zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!15, !16, !17, !18, !19, !20, !21}
