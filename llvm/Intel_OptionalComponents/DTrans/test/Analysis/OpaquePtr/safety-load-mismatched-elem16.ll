; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; This test is to verify that the DTrans safety analyzer does not crash
; when marking mismatch element access when one of the type aliases appears
; to be an opaque structure type. (CMPLRLLVM-38766)

%struct.foo = type opaque
%struct.test = type { ptr, i32 }

%struct.bar = type { ptr, i32 }
%struct.test2 = type { ptr, i32 }

; void @test(%struct.test*)
define void @test(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  ; Result is %struct.foo*.
  %val0 = load ptr, ptr %in

  ; Result is set an opaque structure type.
  %elem_zero_val = load ptr, ptr %val0

  ; Use the %elem_zero_val as a %struct.bar*, instead of the opaque structure type.
  %use_as_bar = getelementptr %struct.bar, ptr %elem_zero_val, i64 0, i32 1

  ; Load with a type mismatch of the element zero of %struct.bar to trigger the
  ; mismatched element access. However, the type also aliases the opaque structure
  ; type.
  %elem_zero2_val = load i32, ptr %elem_zero_val

  ret void
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.bar
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched element access{{ *$}}
; CHECK: End LLVMType: %struct.bar

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.foo
; CHECK: Safety data: Bad casting | Ambiguous GEP | Mismatched element access | No fields in structure{{ *$}}
; CHECK: End LLVMType: %struct.foo

!1 = !{%struct.foo zeroinitializer, i32 1}  ; %struct.foo*
!2 = !{i32 0, i32 0}  ; i32
!3 = !{%struct.bar zeroinitializer, i32 1}  ; %struct.bar*
!4 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!5 = distinct !{!4}
!6 = !{!"S", %struct.foo zeroinitializer, i32 0} ; opaque
!7 = !{!"S", %struct.test zeroinitializer, i32 2, !1, !2} ; { %struct.foo*, i32 }
!8 = !{!"S", %struct.bar zeroinitializer, i32 2, !3, !2} ; { %struct.bar*, i32 }
!9 = !{!"S", %struct.test2 zeroinitializer, i32 2, !3, !2} ; { %struct.bar*, i32 }

!intel.dtrans.types = !{!6, !7, !8, !9}
