; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -dtransanalysis -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-arrays-with-const-entries -passes='require<dtransanalysis>' -debug-only=dtrans-arrays-with-const-entries-verbose -disable-output 2>&1 | FileCheck %s

; This test case checks that field 1 for %class.TestClass is invalid
; for arrays with constant integers since we are using memset without 0.

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

define void @setdata(%class.TestClass* %0) {
  %tmp0 = bitcast %class.TestClass* %0 to i8*
  call void @llvm.memset.p0i8.i64(i8* nonnull align 8 dereferenceable(40) %tmp0, i8 1, i64 40, i1 false)
  ret void
}

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

; CHECK: Type: %class.TestClass = type <{ i32, [4 x i32] }>
; CHECK:  Is structure available: NO
; CHECK:   Field number: 1
; CHECK:    Field available: NO
; CHECK:    Constants: No constant data found
