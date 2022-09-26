; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; Tests to check that the call graph was not seen to be too large for
; DTransAnalysis to run.

; RUN: opt < %s -whole-program-assume -dtransanalysis -debug-only=dtransanalysis -dtrans-maxcallsitecount=100 -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtransanalysis -dtrans-maxcallsitecount=100 -disable-output 2>&1 | FileCheck %s

; CHECK-NOT: Call graph too large

; RUN: opt < %s -whole-program-assume -dtransanalysis -debug-only=dtransanalysis -dtrans-maxinstructioncount=100 -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtransanalysis -dtrans-maxinstructioncount=100 -disable-output 2>&1 | FileCheck %s

; RUN: opt < %s -whole-program-assume -dtransanalysis -debug-only=dtransanalysis -dtrans-maxcallsitecount=100 -dtrans-maxinstructioncount=100 -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -debug-only=dtransanalysis -dtrans-maxcallsitecount=100 -dtrans-maxinstructioncount=100 -disable-output 2>&1 | FileCheck %s

define internal dso_local i32 @foo() local_unnamed_addr {
  ret i32 0
}

define dso_local i32 @main() local_unnamed_addr {
  %value = call i32 @foo()
  ret i32 %value
}


