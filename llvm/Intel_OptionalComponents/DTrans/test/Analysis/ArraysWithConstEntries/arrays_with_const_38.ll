; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -dtransanalysis -debug-only=dtrans-arrays-with-const-entries -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -passes='require<dtransanalysis>' -debug-only=dtrans-arrays-with-const-entries -disable-output 2>&1 | FileCheck %s

; This test case checks that the constant for arrays aren't collected
; since the field of interest is passed to a function.

; NOTE: This is conservative, we need to expand the analysis to handle this case.

%class.TestClass = type <{i32, [4 x i32]}>

define void @baz([4 x i32]* %arr, i32 %var) {
  %tmp1 = getelementptr inbounds [4 x i32], [4 x i32]* %arr, i64 0, i32 0
  store i32 1, i32* %tmp1
  %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %arr, i64 0, i32 1
  store i32 2, i32* %tmp2
  %tmp3 = getelementptr inbounds [4 x i32], [4 x i32]* %arr, i64 0, i32 2
  store i32 %var, i32* %tmp3
  %tmp4 = getelementptr inbounds [4 x i32], [4 x i32]* %arr, i64 0, i32 3
  store i32 %var, i32* %tmp4
  ret void
}

define void @foo(%class.TestClass* %0, i32 %var) {
  %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i64 0, i32 1
  call void @baz([4 x i32]* %tmp1, i32 %var)
  ret void
}

define i32 @bar(%class.TestClass* nocapture readonly %0) {
entry:
  %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i64 0, i32 1
  br label %bb1

bb1:
  %phi1 = phi i32 [ 0, %entry], [ %var, %bb1]
  %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 %phi1
  %tmp3 = load i32, i32* %tmp2
  %var = add nuw nsw i32 1, %phi1
  %tmp4 = icmp eq i32 4, %var
  br i1 %tmp4, label %bb2, label %bb1

bb2:
  ret i32 %tmp3
}

; CHECK: Analyzing fields that are arrays with constant entries

; CHECK: Final result for fields that are arrays with constant entries:
; CHECK: No structure found