; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-arrays-with-const-entries 2>&1 | FileCheck %s

; This test case checks that no constant was collected because there is
; no use for %tmp2.

%boost.arr = type <{ [4 x i32] }>
%class.TestClass = type <{ i32, %boost.arr }>

define void @foo(ptr "intel_dtrans_func_index"="1" %0, i32 "intel_dtrans_func_index"="2" %var) !intel.dtrans.func.type !4 {
  %tmp0 = getelementptr inbounds %class.TestClass, ptr %0, i64 0, i32 1
  %tmp1 = getelementptr inbounds %boost.arr, ptr %tmp0, i64 0, i32 0
  %tmp2 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 0
  ret void
}

!intel.dtrans.types = !{!2, !6}

!0 = !{i32 0, i32 0}
!1 = !{!"A", i32 4, !0}
!2 = !{!"S", %class.TestClass zeroinitializer, i32 2, !0, !7}
!3 = !{%class.TestClass zeroinitializer, i32 1}
!4 = distinct !{!3, !0}
!5 = distinct !{!3}
!6 = !{!"S", %boost.arr zeroinitializer, i32 1, !1}
!7 = !{%boost.arr zeroinitializer, i32 0}

; CHECK: Final result for fields that are arrays with constant entries:
; CHECK:   No structure found
; CHECK: End of arrays with constant entries analysis
