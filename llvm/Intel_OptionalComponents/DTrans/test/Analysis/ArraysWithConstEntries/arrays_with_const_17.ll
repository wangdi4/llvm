; REQUIRES: asserts

; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -dtransanalysis -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -passes='require<dtransanalysis>' -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s

; This test case checks that field 1 for %class.TestClass is invalid because
; its pointer is passed to function @bar.

%class.TestClass = type <{i32, [4 x i32]}>

define void @foo(%class.TestClass* %0, i32 %var) {
  %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i64 0, i32 1
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

define void @baz(%class.TestClass* %0) {
  %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i64 0, i32 1
  %tmp2 = call i32 @bar([4 x i32]* %tmp1)
  ret void
}

define i32 @bar([4 x i32]* nocapture readonly %0) {
entry:
  br label %bb1

bb1:
  %phi1 = phi i32 [ 0, %entry], [ %var, %bb1]
  %tmp1 = getelementptr inbounds [4 x i32], [4 x i32]* %0, i64 0, i32 %phi1
  %tmp2 = load i32, i32* %tmp1
  %var = add nuw nsw i32 1, %phi1
  %tmp4 = icmp eq i32 4, %var
  br i1 %tmp4, label %bb2, label %bb1

bb2:
  ret i32 %tmp2
}

; CHECK: Result after data collection: No structure found