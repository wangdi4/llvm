; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test cases where an void pointer is passed to a function that should be marked
; as "Mismatched argument use" by the safety analyzer.

; Test the simple case where the void pointer argument gets used as a different
; type in the callee than the caller.
%struct.test01a = type { i32, i32 }
%struct.test01b = type { i64 }
define void @use_test01(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !4 {
  ; This is needed to establish %struct.test01b* as an aliased type.
  %field = getelementptr %struct.test01b, ptr %p, i64 0, i32 0
  store i64 0, ptr %field
  ret void
}
define void @test01() {
  %p = call ptr @malloc(i64 8)
  ; This is needed to establish %struct.test01a* as an aliased type.
  %field = getelementptr %struct.test01a, ptr %p, i64 0, i32 1
  store i32 0, ptr %field

  ; This is the instruction we're actually interested in.
  call void @use_test01(ptr %p)
  call void @free(ptr %p)
  ret void
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01a
; CHECK: Safety data: Mismatched argument use{{ *$}}
; CHECK: End LLVMType: %struct.test01a

; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test01b
; CHECK: Safety data: Mismatched argument use{{ *$}}
; CHECK: End LLVMType: %struct.test01b

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !6 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i8 0, i32 1}  ; i8*
!4 = distinct !{!3}
!5 = distinct !{!3}
!6 = distinct !{!3}
!7 = !{!"S", %struct.test01a zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!8 = !{!"S", %struct.test01b zeroinitializer, i32 1, !2} ; { i64 }

!intel.dtrans.types = !{!7, !8}
