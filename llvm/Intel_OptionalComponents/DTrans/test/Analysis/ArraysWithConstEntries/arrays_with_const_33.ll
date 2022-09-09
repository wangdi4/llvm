; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -dtransanalysis -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -passes='require<dtransanalysis>' -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s

; This test case checks that the result was printed correctly when the
; structure doesn't have a name.

define void @foo() {
  %tmp1 = alloca { i64, i8* }
  %tmp2 = getelementptr inbounds { i64, i8* }, { i64, i8* }* %tmp1, i64 0, i32 0
  store i64 16, i64* %tmp2
  ret void
}

; CHECK: Result after data collection:
; CHECK: Type: { i64, i8* }
; CHECK:   Is structure available: YES
; CHECK:   Field number: 0
; CHECK:     Field available: NO
; CHECK:     Constants: No constant data found

; CHECK: Analyzing results:
; CHECK:   Removing: <unnamed struct>
; CHECK:   Reason: None of the fields qualify as array with constant entries
