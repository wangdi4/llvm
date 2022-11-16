; This test verifies that CatchPad instruction is not treated as
; UNHANDLED.

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; CHECK: @"??_R0?AVXMLException@xercesc_2_7@@@8" = internal 
; CHECK-NOT: LocalPointerInfo: CompletelyAnalyzed <UNHANDLED>
; CHECK: @"??_7type_info@@6B@" = external
; CHECK: %1 = alloca ptr, i32 0, align 8
; CHECK-NOT: LocalPointerInfo: CompletelyAnalyzed <UNHANDLED>

%rtti.TypeDescriptor30 = type { ptr, ptr, [31 x i8] }

$"??_R0?AVXMLException@xercesc_2_7@@@8" = comdat any
@"??_R0?AVXMLException@xercesc_2_7@@@8" = internal global %rtti.TypeDescriptor30 { ptr @"??_7type_info@@6B@", ptr null, [31 x i8] c".?AVXMLException@xercesc_2_7@@\00" }, comdat
@"??_7type_info@@6B@" = external constant ptr

define void @"?checkContent@AbstractStringValidator@xercesc_2_7@@MEAAXQEBGQEAVValidationContext@2@_NQEAVMemoryManager@2@@Z"() personality ptr null {
  %1 = alloca ptr, i32 0, align 8, !intel_dtrans_type !5
  invoke void @bar()
    to label %6 unwind label %2

2:                                                ; No predecessors!
  %3 = catchswitch within none [label %4] unwind to caller

4:                                                ; preds = %2
  %5 = catchpad within %3 [ptr @"??_R0?AVXMLException@xercesc_2_7@@@8", i32 0, ptr %1]
  unreachable

6:
  ret void
}

declare void @bar()

!intel.dtrans.types = !{!0}

!0 = !{!"S", %rtti.TypeDescriptor30 zeroinitializer, i32 3, !1, !2, !3}
!1 = !{i8 0, i32 2}
!2 = !{i8 0, i32 1}
!3 = !{!"A", i32 31, !4}
!4 = !{i8 0, i32 0}
!5 = !{i32 0, i32 1}
