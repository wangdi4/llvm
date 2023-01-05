; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt  < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes='require<dtrans-safetyanalyzer>' -disable-output -dtrans-print-immutable-types 2>&1 | FileCheck %s

; This test case checks that the information for arrays with constant entries
; was printed correctly for the DTrans immutable analysis. The goal of this
; test case is to catch the arrays with constant entries in special structures,
; even if the special structure doesn't pass the DTrans safety violations.

%arr.struct = type <{ [4 x i32] }>
%boost.arr = type <{ [4 x i32] }>
%class.TestClass = type <{ i32, %boost.arr }>

define void @foo(ptr "intel_dtrans_func_index"="1" %0, i32 "intel_dtrans_func_index"="2" %var) !intel.dtrans.func.type !4 {
  %tmp0 = getelementptr inbounds %class.TestClass, ptr %0, i64 0, i32 1
  %tmp1 = getelementptr inbounds %boost.arr, ptr %tmp0, i64 0, i32 0
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
  %tmp0 = getelementptr inbounds %class.TestClass, ptr %0, i64 0, i32 1
  %tmp1 = getelementptr inbounds %boost.arr, ptr %tmp0, i64 0, i32 0
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

define void @bas(ptr "intel_dtrans_func_index"="1" %0, ptr "intel_dtrans_func_index"="2" %1) !intel.dtrans.func.type !10 {
  store ptr %0, ptr %1
  ret void
}

!intel.dtrans.types = !{!2, !6, !8}

!0 = !{i32 0, i32 0}
!1 = !{!"A", i32 4, !0}
!2 = !{!"S", %class.TestClass zeroinitializer, i32 2, !0, !7}
!3 = !{%class.TestClass zeroinitializer, i32 1}
!4 = distinct !{!3, !0}
!5 = distinct !{!3}
!6 = !{!"S", %boost.arr zeroinitializer, i32 1, !1}
!7 = !{%boost.arr zeroinitializer, i32 0}
!8 = !{!"S", %arr.struct zeroinitializer, i32 1, !1}
!9 = !{%boost.arr zeroinitializer, i32 1}
!10 = distinct !{!9, !0}

; CHECK:StructType: %class.TestClass = type <{ i32, %boost.arr }>
; CHECK:  Field 1:
; CHECK:    Constant entries in the array: Index: 0  Constant: 1 | Index: 1  Constant: 2
