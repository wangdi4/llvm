; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Check the padded type
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -dtransanalysis -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s --check-prefixes=CHECKPADDED
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -passes='require<dtransanalysis>' -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s --check-prefixes=CHECKPADDED

; Check the base type
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -dtransanalysis -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s --check-prefixes=CHECKBASE
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -passes='require<dtransanalysis>' -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s --check-prefixes=CHECKBASE

; This test case checks that entries 0 and 1 in the field 1 for
; %class.TestClass aren't collected as constants since the information
; for the related type will affect it.
;
; NOTE: The test case runs twice because the results can be printed out
; of order. One run will check that %class.TestClass.base is disabled, and
; the other run will check if %class.TestClass is disabled.

%class.TestClass = type <{i32, [4 x i32], [4 x i8]}>
%class.TestClass.base = type <{i32, [4 x i32]}>

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

define void @baz(%class.TestClass.base* %0, i32 %var) {
  %tmp1 = getelementptr inbounds %class.TestClass.base, %class.TestClass.base* %0, i64 0, i32 1
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

; CHECK: Result after data collection:
; CHECKBASE: Type: %class.TestClass.base = type <{ i32, [4 x i32] }>
; CHECKBASE:   Is structure available: YES
; CHECKBASE:   Field number: 1
; CHECKBASE:     Field available: YES
; CHECKBASE:     Constants:
; CHECKBASE:       Index: i32 1      Value: i32 2
; CHECKBASE:       Index: i32 0      Value: i32 1

; CHECKPADDED: Type: %class.TestClass = type <{ i32, [4 x i32], [4 x i8] }>
; CHECKPADDED:   Is structure available: YES
; CHECKPADDED:   Field number: 1
; CHECKPADDED:     Field available: YES
; CHECKPADDED:     Constants:
; CHECKPADDED:       Index: i32 1      Value: i32 2
; CHECKPADDED:       Index: i32 0      Value: i32 1

; CHECK: Analyzing results:
; CHECKBASE:   Removing Field: 1 from class.TestClass.base
; CHECKBASE:   Mismatch with related type class.TestClass

; CHECKPADDED:   Removing Field: 1 from class.TestClass
; CHECKPADDED:   Mismatch with related type class.TestClass.base

; CHECKBASE:   Removing: class.TestClass.base
; CHECKBASE:   None of the fields qualify as array with constant entries

; CHECKPADDED:   Removing: class.TestClass
; CHECKPADDED:   None of the fields qualify as array with constant entries

; CHECK: Final result for fields that are arrays with constant entries:
; CHECK: No structure found