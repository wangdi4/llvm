; RUN: opt < %s -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa  -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s  -passes='require<wholeprogram>,require<anders-aa>,function(aa-eval)' -aa-pipeline=anders-aa -evaluate-loopcarried-alias -disable-basic-aa -print-all-alias-modref-info -whole-program-assume -disable-output 2>&1 | FileCheck %s

; Test ModRef modeling for library function calls

; Test with library call that is marked as not modifying anything.
@glob01 = internal global double zeroinitializer
define internal double @test01() {
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %ar1 = bitcast i8* %call1 to double*
  %ld.ar1 = load double, double* %ar1

  %arindex = getelementptr double, double* %ar1, i64 10
  %ld.arindex = load double, double* %arindex
  %val = load double, double* %arindex
  %res = call double @sin(double %val)

  %res2 = load double, double* @glob01
  ret double %res2
}
; CHECK-LABEL: Function: test01:
; CHECK: NoModRef:  Ptr: i8* %call1    <->  %res = call double @sin(double %val)
; CHECK: NoModRef:  Ptr: double* %ar1  <->  %res = call double @sin(double %val)
; CHECK: NoModRef:  Ptr: double* %arindex      <->  %res = call double @sin(double %val)
; CHECK: NoModRef:  Ptr: double* @glob01        <->  %res = call double @sin(double %val)

; Test with calls that just operate on the arguments
define internal void @test02() {
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1
  %call2 = tail call noalias i8* @malloc(i64 1024)
  %ld.call2 = load i8, i8* %call2

  %t1 = call i8* @memset(i8* %call1, i32 1, i64 1024)
  %ld.t1 = load i8, i8* %t1
  %t2 = call i8* @strncpy(i8* %call2, i8* %call1, i64 1024)
  %ld.t2 = load i8, i8* %t2
  ret void
}
; CHECK-LABEL: Function: test02:
; CHECK:  Both ModRef:  Ptr: i8* %call1 <->  %t1 = call i8* @memset(i8* %call1, i32 1, i64 1024)
; CHECK:  NoModRef:  Ptr: i8* %call2    <->  %t1 = call i8* @memset(i8* %call1, i32 1, i64 1024)
; CHECK:  Both ModRef:  Ptr: i8* %t1    <->  %t1 = call i8* @memset(i8* %call1, i32 1, i64 1024)
; CHECK:  NoModRef:  Ptr: i8* %t2       <->  %t1 = call i8* @memset(i8* %call1, i32 1, i64 1024)
; CHECK:  Just Ref:  Ptr: i8* %call1    <->  %t2 = call i8* @strncpy(i8* %call2, i8* %call1, i64 1024)
; CHECK:  Both ModRef:  Ptr: i8* %call2 <->  %t2 = call i8* @strncpy(i8* %call2, i8* %call1, i64 1024)
; CHECK:  Just Ref:  Ptr: i8* %t1       <->  %t2 = call i8* @strncpy(i8* %call2, i8* %call1, i64 1024)
; CHECK:  Both ModRef:  Ptr: i8* %t2    <->  %t2 = call i8* @strncpy(i8* %call2, i8* %call1, i64 1024)

; Test call to function marked as GMOD/GREF when there are escaped variables.
declare i8* @foo(i8*)
define internal void @test03() {
  %call1 = tail call noalias i8* @malloc(i64 1024)
  %ld.call1 = load i8, i8* %call1

  %call = tail call i8* @foo(i8* %call1)
  %ld.call = load i8, i8* %call
  call void @exit(i32 1)

  ret void
}
; CHECK-LABEL: Function: test03:
; CHECK: Both ModRef:   call void @exit(i32 1) <->   %call1 = tail call noalias i8* @malloc(i64 1024)
; CHECK: Both ModRef:   call void @exit(i32 1) <->   %call = tail call i8* @foo(i8* %call1)

define dso_local i32 @main(i32 %argc, i8** nocapture readonly %argv) {
  %tmp1 = call double @test01()
  call void @test02()
  call void @test03()
  ret i32 0
}

declare dso_local noalias i8* @malloc(i64)
declare dso_local double @sin(double)
declare dso_local void @exit(i32)
declare dso_local i8* @memset(i8*, i32, i64)
declare dso_local i8* @strncpy(i8* returned, i8* nocapture readonly, i64)

