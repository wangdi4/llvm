; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Verify that the indirect call analysis handles vararg function calls.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32 }
; CHECK: Safety data: Global instance | Has initializer list | Address taken{{ *$}}
; CHECK: End LLVMType: %struct.MYSTRUCT

%struct.MYSTRUCT = type { i32, i32 }

@myarg = internal global %struct.MYSTRUCT { i32 3, i32 5 }, align 4
@fp1 = internal global ptr @target1, align 8, !intel_dtrans_type !0
@fp2 = internal global ptr @target2, align 8, !intel_dtrans_type !4

declare !intel.dtrans.func.type !7 i32 @target1(ptr "intel_dtrans_func_index"="1", ...)

declare !intel.dtrans.func.type !8 "intel_dtrans_func_index"="1" ptr @target2(...)

define dso_local i32 @main() {
  %i = load ptr, ptr @fp1, align 8
  %call = tail call i32 (...) %i(ptr @myarg, i32 7, i32 9), !intel_dtrans_type !9
  ret i32 0
}

!intel.dtrans.types = !{!6}

!0 = !{!1, i32 1}
!1 = !{!"F", i1 true, i32 1, !2, !3}
!2 = !{i32 0, i32 0}
!3 = !{%struct.MYSTRUCT zeroinitializer, i32 1}
!4 = !{!5, i32 1}
!5 = !{!"F", i1 true, i32 0, !3}
!6 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !2, !2}
!7 = distinct !{!3}
!8 = distinct !{!3}
!9 = !{!"F", i1 true, i32 0, !2}
