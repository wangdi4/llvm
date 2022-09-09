; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -whole-program-assume -dtrans-arrays-with-const-entries -dtransanalysis -dtrans-print-types -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-arrays-with-const-entries -passes='require<dtransanalysis>' -dtrans-print-types -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s

; This test case checks that the information for entries 0 and 1 in the
; array at field 1 of %class.TestClass isn't generated. The reason is
; the DTrans analysis produces a Bad pointer manipulation. The goal
; of this test is to check the conditions for generate the metadata.

; NOTE: -dtrans-outofboundsok=false is not added in this test case
; since we want to create Bad pointer manipulation.

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

define i32 @bar(%class.TestClass* %0, i32 %var) {
entry:
  %tmp1 = getelementptr inbounds %class.TestClass, %class.TestClass* %0, i64 0, i32 1
  %tmp2 = getelementptr inbounds [4 x i32], [4 x i32]* %tmp1, i64 0, i32 %var
  %tmp3 = load i32, i32* %tmp2
  ret i32 %tmp3
}


; CHECK: Result after data collection: No structure found

; CHECK-LABEL: %class.TestClass = type <{ i32, [4 x i32] }>
; CHECK: Safety data: {{.*}}Bad pointer manipulation{{.*}}
