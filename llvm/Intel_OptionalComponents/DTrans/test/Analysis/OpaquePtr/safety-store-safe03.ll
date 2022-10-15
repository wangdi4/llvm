; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Store element zero of a nested structure using a pointer to the structure
%struct.test01a = type { %struct.test01b }
%struct.test01b = type { %struct.test01c, i64 }
%struct.test01c = type { i32, i32 }
define void @test01(%struct.test01a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !6 {
  %pField = getelementptr %struct.test01a, %struct.test01a* %pStruct, i64 0, i32 0
  %pField.as.p32 = bitcast %struct.test01b* %pField to i32*
  store i32 1, i32* %pField.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Nested structure | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01c
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written NonGEPAccess{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01c


; Store element zero of a nested structure using a pointer to the structure, when
; element zero is an array
%struct.test02a = type { [2 x %struct.test02b] }
%struct.test02b = type { [6 x %struct.test02c], i64 }
%struct.test02c = type { i32, i32 }
define void @test02(%struct.test02a* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !12 {
  %pField = getelementptr %struct.test02a, %struct.test02a* %pStruct, i64 0, i32 0
  %pField.as.p32 = bitcast [2 x %struct.test02b]* %pField to i32*
  store i32 2, i32* %pField.as.p32
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02a
; CHECK: Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02b
; CHECK: Safety data: Nested structure | Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02b

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02c
; CHECK: 0)Field LLVM Type: i32
; CHECK: Field info: Written NonGEPAccess{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK: Field info:{{ *$}}
; CHECK: Safety data: Nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test02c


!1 = !{%struct.test01b zeroinitializer, i32 0}  ; %struct.test01b
!2 = !{%struct.test01c zeroinitializer, i32 0}  ; %struct.test01c
!3 = !{i64 0, i32 0}  ; i64
!4 = !{i32 0, i32 0}  ; i32
!5 = !{%struct.test01a zeroinitializer, i32 1}  ; %struct.test01a*
!6 = distinct !{!5}
!7 = !{!"A", i32 2, !8}  ; [2 x %struct.test02b]
!8 = !{%struct.test02b zeroinitializer, i32 0}  ; %struct.test02b
!9 = !{!"A", i32 6, !10}  ; [6 x %struct.test02c]
!10 = !{%struct.test02c zeroinitializer, i32 0}  ; %struct.test02c
!11 = !{%struct.test02a zeroinitializer, i32 1}  ; %struct.test02a*
!12 = distinct !{!11}
!13 = !{!"S", %struct.test01a zeroinitializer, i32 1, !1} ; { %struct.test01b }
!14 = !{!"S", %struct.test01b zeroinitializer, i32 2, !2, !3} ; { %struct.test01c, i64 }
!15 = !{!"S", %struct.test01c zeroinitializer, i32 2, !4, !4} ; { i32, i32 }
!16 = !{!"S", %struct.test02a zeroinitializer, i32 1, !7} ; { [2 x %struct.test02b] }
!17 = !{!"S", %struct.test02b zeroinitializer, i32 2, !9, !3} ; { [6 x %struct.test02c], i64 }
!18 = !{!"S", %struct.test02c zeroinitializer, i32 2, !4, !4} ; { i32, i32 }

!intel.dtrans.types = !{!13, !14, !15, !16, !17, !18}
