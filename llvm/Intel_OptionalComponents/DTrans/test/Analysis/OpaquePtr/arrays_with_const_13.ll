; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-outofboundsok=false -disable-output -debug-only=dtrans-arrays-with-const-entries 2>&1 | FileCheck %s
; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -disable-output -debug-only=dtrans-arrays-with-const-entries 2>&1 | FileCheck %s

; This test case checks that entries 0 and 1 in the field 1 for
; %class.TestClass and %class.TestClass.base weren't collected because the
; information mismatch between both structures.


%class.TestClass = type <{ i32, [4 x i32], [4 x i8]}>
%class.TestClass.base = type <{ i32, [4 x i32]}>

; Function @foo will assign information to %class.TestClass
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

; Function @baz will assign information to %class.TestClass.base
define void @baz(ptr "intel_dtrans_func_index"="1" %0, i32 "intel_dtrans_func_index"="2" %var) !intel.dtrans.func.type !10 {
  %tmp1 = getelementptr inbounds %class.TestClass.base, ptr %0, i64 0, i32 1
  %tmp2 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 0
  store i32 3, ptr %tmp2, align 4
  %tmp3 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 1
  store i32 4, ptr %tmp3, align 4
  %tmp4 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 2
  store i32 %var, ptr %tmp4, align 4
  %tmp5 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 3
  store i32 %var, ptr %tmp5, align 4
  ret void
}

!intel.dtrans.types = !{!2, !6}

!0 = !{i32 0, i32 0}
!1 = !{!"A", i32 4, !0}
!2 = !{!"S", %class.TestClass zeroinitializer, i32 3, !0, !1, !7}
!3 = !{%class.TestClass zeroinitializer, i32 1}
!4 = distinct !{!3, !0}
!5 = distinct !{!3}
!6 = !{!"S", %class.TestClass.base zeroinitializer, i32 2, !0, !1}
!7 = !{!"A", i32 4, !8}
!8 = !{i8 0, i32 0}
!9 = !{%class.TestClass.base zeroinitializer, i32 1}
!10 = distinct !{!9, !0}

; CHECK: Final result for fields that are arrays with constant entries
; CHECK:   No structure found
; CHECK: End of arrays with constant entries analysis
