; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes="require<dtrans-safetyanalyzer>,hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir>" -hir-complete-unroll-force-constprop -disable-output < %s 2>&1 | FileCheck %s

; This test case checks that entries 0 and 1 in the field 1 for
; %class.TestClass, which is an array, are collected as constants.
; And later are substituted during HIR Pre Vec Complete Unroll pass.

; Function: bar
;     BEGIN REGION { }
;        + DO i1 = 0, 3, 1   <DO_LOOP>
;        |   %tmp3 = (%0)[0].1.0[i1];
;        |   (@B)[0][i1] = %tmp3;
;        + END LOOP
;     END REGION

; CHECK: Function: bar
; CHECK: BEGIN REGION { modified }
; CHECK: (@B)[0][0] = 1;
; CHECK: (@B)[0][1] = 2;
; CHECK: (@B)[0][2] = %tmp3;
; CHECK: (@B)[0][3] = %tmp3;

target triple = "x86_64-unknown-linux-gnu"
%class.TestClass = type <{i32, %"class.boost::array"}>
%"class.boost::array" = type <{[4 x i32]}>
@B = dso_local global [4 x i32] zeroinitializer, align 16

define void @foo(ptr "intel_dtrans_func_index"="1" %0, i32 %var) !intel.dtrans.func.type !5 {
  %tmp0 = getelementptr inbounds %class.TestClass, ptr %0, i64 0, i32 1
  %tmp1 = getelementptr inbounds %"class.boost::array", ptr %tmp0, i64 0, i32 0
  %tmp2 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 0
  store i32 1, ptr %tmp2
  %tmp3 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 1
  store i32 2, ptr %tmp3
  %tmp4 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 2
  store i32 %var, ptr %tmp4
  %tmp5 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 3
  store i32 %var, ptr %tmp5
  ret void
}

define i32 @bar(ptr "intel_dtrans_func_index"="1" %0) !intel.dtrans.func.type !6 {
entry:
  %tmp0 = getelementptr inbounds %class.TestClass, ptr %0, i64 0, i32 1
  %tmp1 = getelementptr inbounds %"class.boost::array", ptr %tmp0, i64 0, i32 0
  br label %bb1

bb1:
  %phi1 = phi i32 [ 0, %entry], [ %var, %bb1]
  %tmp2 = getelementptr inbounds [4 x i32], ptr %tmp1, i64 0, i32 %phi1
  %tmp3 = load i32, ptr %tmp2
  %arrayidx = getelementptr inbounds [4 x i32], ptr @B, i64 0, i32 %phi1
  store i32 %tmp3, ptr %arrayidx, align 4
  %var = add nuw i32 1, %phi1
  %tmp4 = icmp eq i32 4, %var
  br i1 %tmp4, label %bb2, label %bb1

bb2:
  ret i32 %tmp3
}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%"class.boost::array" zeroinitializer, i32 0}  ; %"class.boost::array"
!3 = !{!"A", i32 4, !1}  ; [4 x i32]
!4 = !{%class.TestClass zeroinitializer, i32 1}  ; %class.TestClass*
!5 = distinct !{!4}
!6 = distinct !{!4}
!7 = !{!"S", %class.TestClass zeroinitializer, i32 2, !1, !2} ; <{i32, %"class.boost::array"}>
!8 = !{!"S", %"class.boost::array" zeroinitializer, i32 1, !3} ; <{[4 x i32]}>

!intel.dtrans.types = !{!7, !8}

; end INTEL_FEATURE_SW_DTRANS
