; REQUIRES: asserts

; This test checks that @foo is identified as a missing libfunc since there
; is a call from main. The function @bar should not be marked as libfunc since
; the user is an alias (@bar_alias), but there is no actual use for it. This
; test case represents an alias that we can see in Windows. It will use opt
; rather than llvm-lto because we want to check that the functions traversal
; is working correctly in the whole program analysis.

; RUN: opt < %s -disable-output -passes='require<wholeprogram>' -debug-only=whole-program-analysis 2>&1  | FileCheck %s

; CHECK:     WHOLE-PROGRAM-ANALYSIS
; CHECK: LIBFUNCS NOT FOUND: 1
; CHECK-NEXT:       foo
; CHECK-NOT:        bar

$bar_alias = comdat largest
$bar = comdat any

@anon.constant = private unnamed_addr constant { [1 x i8*] } { [1 x i8*] [i8* bitcast (i8* ()* @bar to i8*)] }, comdat($bar_alias)
@bar_alias = unnamed_addr alias i8*, getelementptr inbounds ({ [1 x i8*] }, { [1 x i8*] }* @anon.constant, i32 0, i32 0, i32 1)

declare void @foo()
declare noalias i8* @malloc(i64)

define weak_odr dso_local i8* @bar() unnamed_addr comdat align 2 {
  %call = call noalias i8* @malloc(i64 8)
  ret i8* %call
}

; Function Attrs: nounwind uwtable
define i32 @main()  {
entry:
  call void @foo()
  ret i32 0
}

