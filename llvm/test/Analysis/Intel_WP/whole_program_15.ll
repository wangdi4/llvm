; REQUIRES: asserts

; This test checks that @foo is identified as a missing libfunc since there
; is a call from main. The function @bar should not be marked as missing
; libfunc since @baz doesn't reach main. This test case will use opt rather
; than llvm-lto because we want to check that the functions traversal is
; working correctly.

; RUN: opt < %s -disable-output -passes='require<wholeprogram>' -debug-only=whole-program-analysis 2>&1  | FileCheck %s

; CHECK:     WHOLE-PROGRAM-ANALYSIS
; CHECK: LIBFUNCS NOT FOUND: 1
; CHECK-NEXT:       foo
; CHECK-NOT:        bar

define i32 @baz() {
  call void @bar()
  ret i32 0
}

; Function Attrs: nounwind uwtable
define i32 @main()  {
entry:
  call void @foo()
  ret i32 0
}

declare void @foo()
declare void @bar()