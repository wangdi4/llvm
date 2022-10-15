; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; This test is to verify field value collection of structure fields for
; the "zeroth-element access" idiom.

%struct.test01 = type { %struct.test01inner, i32 }
%struct.test01inner = type { i32, i32 }
define "intel_dtrans_func_index"="1" %struct.test01* @test01() !intel.dtrans.func.type !4 {
  %flat = call i8* @malloc(i64 12)
  %obj = bitcast i8* %flat to %struct.test01*

  ; Perform a direct write to the zeroth element of the nested structure using
  ; the address of the outer structure to verify the store tracks the value to
  ; the appropriate field.
  %elemZero = bitcast %struct.test01* %obj to i32*
  store i32 1, i32* %elemZero

  ret %struct.test01* %obj
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK:  0)Field LLVM Type: %struct.test01inner = type { i32, i32 }
; CHECK:    Field info:
; CHECK:    Multiple Value: [  ] <incomplete>
; CHECK:  1)Field LLVM Type: i32
; CHECK:    No Value
; CHECK:  Safety data: Contains nested structure{{ *$}}
; CHECK: End LLVMType: %struct.test01

; CHECK:  LLVMType: %struct.test01inner
; CHECK:  0)Field LLVM Type: i32
; CHECK:    Single Value: i32 1
; CHECK:  1)Field LLVM Type: i32
; CHECK:    No Value
; CHECK:  Safety data: Nested structure{{ *$}}
; CHECK:  End LLVMType: %struct.test01inner

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!1 = !{%struct.test01inner zeroinitializer, i32 0}  ; %struct.test01inner
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = distinct !{!3}
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !2} ; { %struct.test01inner, i32 }
!8 = !{!"S", %struct.test01inner zeroinitializer, i32 2, !2, !2} ; { i32, i32 }

!intel.dtrans.types = !{!7, !8}
