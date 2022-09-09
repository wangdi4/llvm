; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -dtransanalysis -debug-only=dtrans-arrays-with-const-entries -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -passes='require<dtransanalysis>' -debug-only=dtrans-arrays-with-const-entries -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test case checks that entries 0 and 1 in the field 1 for
; %class.TestClass, which is an array, are collected as constants
; even if %class.TestClass is marked as FieldAddressTakenCall.

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

define void @cas(i32* %a) {
  ret void
}

define void @baz(%class.TestClass* %0) {
  %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i64 0, i32 0
  call void @cas(i32* %tmp1)
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

; CHECK: Final result for fields that are arrays with constant entries:
; CHECK: Type: %class.TestClass = type <{ i32, [4 x i32] }>
; CHECK:   Is structure available: YES
; CHECK:   Field number: 1
; CHECK:     Field available: YES
; CHECK:     Constants:
; CHECK:       Index: i32 1      Value: i32 2
; CHECK:       Index: i32 0      Value: i32 1

; CHECK: LLVMType: %class.TestClass = type <{ i32, [4 x i32] }>
; CHECK: Safety data: Field address taken call