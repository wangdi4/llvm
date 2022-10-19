; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; RUN: opt -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -dtransanalysis -hir-ssa-deconstruction -hir-pre-vec-complete-unroll -hir-complete-unroll-force-constprop -print-after=hir-pre-vec-complete-unroll -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -passes="require<dtransanalysis>,hir-ssa-deconstruction,hir-pre-vec-complete-unroll,print<hir>" -hir-complete-unroll-force-constprop -disable-output < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

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

%class.TestClass = type <{i32, %"class.boost::array"}>
%"class.boost::array" = type <{[4 x i32]}>
@B = dso_local global [4 x i32] zeroinitializer, align 16

define void @foo(%class.TestClass* %0, i32 %var) {
  %tmp0 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i64 0, i32 1
  %tmp1 = getelementptr inbounds %"class.boost::array", %"class.boost::array"* %tmp0, i64 0, i32 0
  %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 0
  store i32 1, i32* %tmp2
  %tmp3 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 1
  store i32 2, i32* %tmp3
  %tmp4 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 2
  store i32 %var, i32* %tmp4
  %tmp5 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 3
  store i32 %var, i32* %tmp5
  ret void
}

define i32 @bar(%class.TestClass* %0) {
entry:
  %tmp0 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i64 0, i32 1
  %tmp1 = getelementptr inbounds %"class.boost::array", %"class.boost::array"* %tmp0, i64 0, i32 0
  br label %bb1

bb1:
  %phi1 = phi i32 [ 0, %entry], [ %var, %bb1]
  %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 %phi1
  %tmp3 = load i32, i32* %tmp2
  %arrayidx = getelementptr inbounds [4 x i32], [4 x i32]* @B, i64 0, i32 %phi1
  store i32 %tmp3, i32* %arrayidx, align 4
  %var = add nuw i32 1, %phi1
  %tmp4 = icmp eq i32 4, %var
  br i1 %tmp4, label %bb2, label %bb1

bb2:
  ret i32 %tmp3
}

; end INTEL_FEATURE_SW_DTRANS
