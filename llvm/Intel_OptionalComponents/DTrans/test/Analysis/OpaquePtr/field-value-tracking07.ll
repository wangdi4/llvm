; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that a whole structure store marks all field members as 'incomplete' for
; the value collection. This could be improved to try to determine when new
; values are being stored, as seen in @test01 versus when existing values are
; being copied, as seen in @test02. However, that is not currently necessary for
; the cases of interest.

%struct.test01 = type { i32, i32 }
define void @test01() {
  %tmp = alloca %struct.test01
  %f0 = getelementptr %struct.test01, %struct.test01* %tmp, i64 0, i32 0
  %f1 = getelementptr %struct.test01, %struct.test01* %tmp, i64 0, i32 1
  store i32 99, i32* %f0
  store i32 98, i32* %f1

  store %struct.test01 { i32 100, i32 0 }, %struct.test01* %tmp
  ret void
}
; CHECK-LABEL:  LLVMType: %struct.test01
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 99 ] <incomplete>
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 98 ] <incomplete>
; CHECK:  Safety data: Whole structure reference | Local instance{{ *}}
; CHECK:  End LLVMType: %struct.test01


%struct.test02 = type { i32, i32 }
define void @test02(%struct.test02 %in) {
  %tmp = alloca %struct.test02
  %f0 = getelementptr %struct.test02, %struct.test02* %tmp, i64 0, i32 0
  %f1 = getelementptr %struct.test02, %struct.test02* %tmp, i64 0, i32 1
  store i32 99, i32* %f0
  store i32 98, i32* %f1

  store %struct.test02 %in, %struct.test02* %tmp
  ret void
}
; CHECK-LABEL:  LLVMType: %struct.test02
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 99 ] <incomplete>
; CHECK:  1)Field LLVM Type: i32
; CHECK:    Multiple Value: [ 98 ] <incomplete>
; CHECK:  Safety data: Whole structure reference | Local instance{{ *}}
; CHECK:  End LLVMType: %struct.test02


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!3 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!2, !3}
