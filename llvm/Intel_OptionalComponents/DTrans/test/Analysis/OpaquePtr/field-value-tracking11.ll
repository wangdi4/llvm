; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that a call to calloc updates the field value info.

; Test calloc to allocate 1 copy of a structure
%struct.test01 = type { i32, float }
define "intel_dtrans_func_index"="1" %struct.test01* @test01() !intel.dtrans.func.type !4 {
  %ptr = call i8* @calloc(i64 1, i64 8)

  %obj = bitcast i8* %ptr to %struct.test01*
  ret %struct.test01* %obj
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   1)Field LLVM Type: float
; CHECK:     Single Value: float 0.000000e+00
; CHECK:   Safety data: No issues found{{ *$}}
; CHECK: End LLVMType: %struct.test01


; Test calloc to allocate %n copies of a structure
%struct.test02 = type { i32, float }
define "intel_dtrans_func_index"="1" %struct.test02* @test02(i64 %n) !intel.dtrans.func.type !6 {
  %ptr = call i8* @calloc(i64 %n, i64 8)

  %obj = bitcast i8* %ptr to %struct.test02*
  ret %struct.test02* %obj
}
; CHECK-LABEL: LLVMType: %struct.test02
; CHECK:   0)Field LLVM Type: i32
; CHECK:     Single Value: i32 0
; CHECK:   1)Field LLVM Type: float
; CHECK:     Single Value: float 0.000000e+00
; CHECK:   Safety data: No issues found{{ *$}}
; CHECK: End LLVMType: %struct.test02


declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{float 0.0e+00, i32 0}  ; float
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = !{i8 0, i32 1}  ; i8*
!8 = distinct !{!7}
!9 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { i32, float }
!10 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !2} ; { i32, float }

!intel.dtrans.types = !{!9, !10}
