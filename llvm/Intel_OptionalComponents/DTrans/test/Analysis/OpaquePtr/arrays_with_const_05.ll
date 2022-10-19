; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-outofboundsok=true -disable-output -debug-only=dtrans-arrays-with-const-entries 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=true -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-arrays-with-const-entries 2>&1 | FileCheck %s

; This test case checks that entries 0 and 1 in the field 1 for
; %class.TestClass aren't collected since out of bounds is enabled.

%class.TestClass = type <{ i32, [4 x i32] }>

define void @foo(ptr "intel_dtrans_func_index"="1" %0, i32 "intel_dtrans_func_index"="2" %var) !intel.dtrans.func.type !4 {
  %tmp1 = getelementptr inbounds %class.TestClass, ptr %0, i64 0, i32 1
  %tmp2 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 0
  store i32 1, ptr %tmp2, align 4
  %tmp3 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 1
  store i32 2, ptr %tmp3, align 4
  %tmp4 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 2
  store i32 %var, ptr %tmp4, align 4
  %tmp5 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 3
  store i32 %var, ptr %tmp5, align 4
  ret void
}

define i32 @bar(ptr nocapture readonly "intel_dtrans_func_index"="1" %0) !intel.dtrans.func.type !5 {
entry:
  %tmp1 = getelementptr inbounds %class.TestClass, ptr %0, i64 0, i32 1
  br label %bb1

bb1:                                              ; preds = %bb1, %entry
  %phi1 = phi i32 [ 0, %entry ], [ %var, %bb1 ]
  %tmp2 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 %phi1
  %tmp3 = load i32, ptr %tmp2, align 4
  %var = add nuw nsw i32 1, %phi1
  %tmp4 = icmp eq i32 4, %var
  br i1 %tmp4, label %bb2, label %bb1

bb2:                                              ; preds = %bb1
  ret i32 %tmp3
}

!intel.dtrans.types = !{!2}

!0 = !{i32 0, i32 0}
!1 = !{!"A", i32 4, !0}
!2 = !{!"S", %class.TestClass zeroinitializer, i32 2, !0, !1}
!3 = !{%class.TestClass zeroinitializer, i32 1}
!4 = distinct !{!3, !0}
!5 = distinct !{!3}

; CHECK: Final result for fields that are arrays with constant entries
; CHECK:   No structure found
; CHECK: End of arrays with constant entries analysis
