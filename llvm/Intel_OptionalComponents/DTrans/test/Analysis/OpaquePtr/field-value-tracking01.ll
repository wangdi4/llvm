; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify field value collection of structure fields is
; handled when storing a value to the field.

; Store a constant value to the first two fields, and no writes to the last
; field.
%struct.test01 = type { i32, i32, i32 }
define void @test01() {
  %local = alloca %struct.test01
  %pA = getelementptr %struct.test01, %struct.test01* %local, i64 0, i32 0
  %pB = getelementptr %struct.test01, %struct.test01* %local, i64 0, i32 1
  store i32 1, i32* %pA
  store i32 2, i32* %pB

  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Single Value: i32 1
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Single Value: i32 2
; CHECK:  2)Field LLVM Type: i32
; CHECK:    No Value
; CHECK:  Safety data: Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Store a constant value to each field, but with multiple writers of the field.
%struct.test02 = type { i32, i32, i32 }
define void @test02() {
  %localA = alloca %struct.test02
  %localB = alloca %struct.test02
  %pA = getelementptr %struct.test02, %struct.test02* %localA, i64 0, i32 0
  %pB = getelementptr %struct.test02, %struct.test02* %localA, i64 0, i32 1
  %pC = getelementptr %struct.test02, %struct.test02* %localA, i64 0, i32 2
  store i32 1, i32* %pA
  store i32 2, i32* %pB
  store i32 4, i32* %pC

  %oA = getelementptr %struct.test02, %struct.test02* %localA, i64 0, i32 0
  %oB = getelementptr %struct.test02, %struct.test02* %localA, i64 0, i32 1
  %oC = getelementptr %struct.test02, %struct.test02* %localA, i64 0, i32 2
  store i32 1, i32* %oA
  store i32 2, i32* %oB
  store i32 4, i32* %oC

  ret void
}
; CHECK-LABEL: LLVMType: %struct.test02
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Single Value: i32 1
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Single Value: i32 2
; CHECK:  2)Field LLVM Type: i32
; CHECK:    Single Value: i32 4
; CHECK:  Safety data: Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test02


; Store multiple constant values to each field.
%struct.test03 = type { i32, i32, i32 }
define void @test03() {
  %localA = alloca %struct.test03
  %localB = alloca %struct.test03
  %pA = getelementptr %struct.test03, %struct.test03* %localA, i64 0, i32 0
  %pB = getelementptr %struct.test03, %struct.test03* %localA, i64 0, i32 1
  %pC = getelementptr %struct.test03, %struct.test03* %localA, i64 0, i32 2
  store i32 1, i32* %pA
  store i32 2, i32* %pB
  store i32 4, i32* %pC

  %oA = getelementptr %struct.test03, %struct.test03* %localA, i64 0, i32 0
  %oB = getelementptr %struct.test03, %struct.test03* %localA, i64 0, i32 1
  %oC = getelementptr %struct.test03, %struct.test03* %localA, i64 0, i32 2
  store i32 8, i32* %oA
  store i32 16, i32* %oB
  store i32 32, i32* %oC

  ret void
}
; Note: Values are sorted lexically when there are multiple values.
; CHECK-LABEL: LLVMType: %struct.test03
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 1, 8 ] <complete>
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 16, 2 ] <complete>
; CHECK:  2)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 32, 4 ] <complete>
; CHECK:  Safety data: Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test03


; Store constant and non-constant values
; Field 0, first gets non-const, then const
; Field 1, first gets const, then non-const
; Field 2, only gets non-const
%struct.test04 = type { i32, i32, i32 }
define void @test04(i32 %in0, i32 %in1, i32 %in2) {
  %localA = alloca %struct.test04
  %localB = alloca %struct.test04
  %pA = getelementptr %struct.test04, %struct.test04* %localA, i64 0, i32 0
  %pB = getelementptr %struct.test04, %struct.test04* %localA, i64 0, i32 1
  %pC = getelementptr %struct.test04, %struct.test04* %localA, i64 0, i32 2
  store i32 %in0, i32* %pA
  store i32 2, i32* %pB
  store i32 %in2, i32* %pC

  %oA = getelementptr %struct.test04, %struct.test04* %localA, i64 0, i32 0
  %oB = getelementptr %struct.test04, %struct.test04* %localA, i64 0, i32 1
  %oC = getelementptr %struct.test04, %struct.test04* %localA, i64 0, i32 2
  store i32 8, i32* %oA
  store i32 %in1, i32* %oB
  store i32 %in2, i32* %oC

  ; Store field 0 again, to get multiple constants
  store i32 1, i32* %oA

  ret void
}
; CHECK-LABEL: LLVMType: %struct.test04
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 1, 8 ] <incomplete>
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 2 ] <incomplete>
; CHECK:  2)Field LLVM Type: i32
; CHECK:    Multiple Value: [ ] <incomplete>
; CHECK:  Safety data: Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test04


; Test with GEPs that index into an array of structures
%struct.test05 = type { i32, i32, i32 }
define void @test05() {
  %localA = alloca [2 x %struct.test05]
  %pA = getelementptr [2 x %struct.test05], [2 x %struct.test05]* %localA, i64 0, i64 0, i32 0
  %pB = getelementptr [2 x %struct.test05], [2 x %struct.test05]* %localA, i64 0, i64 0, i32 1
  %pC = getelementptr [2 x %struct.test05], [2 x %struct.test05]* %localA, i64 0, i64 0, i32 2
  store i32 1, i32* %pA
  store i32 2, i32* %pB
  store i32 4, i32* %pC

  %oA = getelementptr [2 x %struct.test05], [2 x %struct.test05]* %localA, i64 0, i64 1, i32 0
  %oB = getelementptr [2 x %struct.test05], [2 x %struct.test05]* %localA, i64 0, i64 1, i32 1
  %oC = getelementptr [2 x %struct.test05], [2 x %struct.test05]* %localA, i64 0, i64 1, i32 2
  store i32 8, i32* %oA
  store i32 16, i32* %oB
  store i32 32, i32* %oC

  ret void
}
; CHECK-LABEL: LLVMType: %struct.test05
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 1, 8 ] <complete>
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 16, 2 ] <complete>
; CHECK:  2)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 32, 4 ] <complete>
; CHECK:  Safety data: Local instance{{ *$}}
; CHECK: End LLVMType: %struct.test05


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!3 = !{!"S", %struct.test02 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!4 = !{!"S", %struct.test03 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!5 = !{!"S", %struct.test04 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!6 = !{!"S", %struct.test05 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }

!intel.dtrans.types = !{!2, !3, !4, !5, !6}
