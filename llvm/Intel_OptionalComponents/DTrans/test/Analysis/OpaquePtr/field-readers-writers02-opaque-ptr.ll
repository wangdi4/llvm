; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that allocation via calloc is identified as a writer of the
; structure fields.

%struct.test01 = type { i32, i32 }
@var01 = internal global ptr zeroinitializer, !intel_dtrans_type !2
define internal void @test01(ptr "intel_dtrans_func_index"="1" %in, i64 %index) !intel.dtrans.func.type !3 {
  %st = call ptr @calloc(i64 16, i64 8)
  store ptr %st, ptr @var01
  ret void
}
; CHECK-LABEL: LLVMType: %struct.test01
; CHECK: 0)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers:{{ *$}}
; CHECK: Writers: test01
; CHECK: 1)Field
; CHECK: DTrans Type: i32
; CHECK: Field info: Written
; CHECK: Readers:{{ *$}}
; CHECK: Writers: test01
; CHECK: End LLVMType: %struct.test01

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = distinct !{!2}
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; {i32, i32 }

!intel.dtrans.types = !{!6}
