; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases of store instructions that should be identified as "safe"

%struct.test01 = type { i32, i32, i32 }
define void @test01(%struct.test01* "intel_dtrans_func_index"="1" %pStruct, i32 %value) !intel.dtrans.func.type !3 {
  %pField = getelementptr %struct.test01, %struct.test01* %pStruct, i64 0, i32 0
  store i32 %value, i32* %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01
; CHECK: 0)Field LLVM Type: i32
; CHECK:   Field info: Written{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test01


%struct.test02 = type { i32*, i32*, i32* }
define void @test02(%struct.test02* "intel_dtrans_func_index"="1" %pStruct, i32* "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !6 {
  %pField = getelementptr %struct.test02, %struct.test02* %pStruct, i64 0, i32 1
  store i32* %value, i32** %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test02
; CHECK: 0)Field LLVM Type: i32*
; CHECK:   Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32*
; CHECK:   Field info: Written{{ *$}}
; CHECK: 2)Field LLVM Type: i32*
; CHECK:   Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test02


%struct.test03a = type { %struct.test03b*, %struct.test03b*, %struct.test03b* }
%struct.test03b = type { i32, i32, i32 }
define void @test03(%struct.test03a* "intel_dtrans_func_index"="1" %pStruct, %struct.test03b* "intel_dtrans_func_index"="2" %value) !intel.dtrans.func.type !9 {
  %pField = getelementptr %struct.test03a, %struct.test03a* %pStruct, i64 0, i32 2
  store %struct.test03b* %value, %struct.test03b** %pField
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03a
; CHECK: 0)Field LLVM Type: %struct.test03b*
; CHECK:   Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: %struct.test03b*
; CHECK:   Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: %struct.test03b*
; CHECK:   Field info: Written{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test03b
; CHECK: 0)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: 1)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: 2)Field LLVM Type: i32
; CHECK:   Field info:{{ *$}}
; CHECK: Safety data: No issues found
; CHECK: End LLVMType: %struct.test03b


!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i32 0, i32 1}  ; i32*
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5, !4}
!7 = !{%struct.test03b zeroinitializer, i32 1}  ; %struct.test03b*
!8 = !{%struct.test03a zeroinitializer, i32 1}  ; %struct.test03a*
!9 = distinct !{!8, !7}
!10 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!11 = !{!"S", %struct.test02 zeroinitializer, i32 3, !4, !4, !4} ; { i32*, i32*, i32* }
!12 = !{!"S", %struct.test03a zeroinitializer, i32 3, !7, !7, !7} ; { %struct.test03b*, %struct.test03b*, %struct.test03b* }
!13 = !{!"S", %struct.test03b zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!10, !11, !12, !13}
