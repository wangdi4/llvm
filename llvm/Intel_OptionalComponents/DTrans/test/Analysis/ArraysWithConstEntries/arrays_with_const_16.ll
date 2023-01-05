; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -passes='require<dtransanalysis>' -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s

; This test case checks that field 1 in %class.TestClass is invalid since there
; is a load to entry 0 in the array in at @baz, but the field is accessed
; through a byte flattened GEP.

%class.TestClass = type <{i32, %"class.boost::array"}>
%"class.boost::array" = type <{[4 x i32]}>

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
  %var = add nuw i32 1, %phi1
  %tmp4 = icmp eq i32 4, %var
  br i1 %tmp4, label %bb2, label %bb1

bb2:
  ret i32 %tmp3
}

define i32 @baz(%class.TestClass* %0) {
  %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i32 8
  %tmp2 = bitcast %class.TestClass* %tmp1 to [4 x i32]*
  %tmp3 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp2, i64 0, i32 0
  %tmp4 = load i32, i32* %tmp3
  ret i32 %tmp4
}

; CHECK: Result after data collection: No structure found
