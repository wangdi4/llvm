; RUN: llvm-as %s -o %t.bc
; RUN: opt -resolve %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;
;  Hello CVCC.
;
;  About the test:
;  In this test we check that the function-call "masked_load"
;  is inlined properly (resolved).
;  In this test we check that the transformed code does not contain
;  the original function-call.
;  We place a 'CHECK-NOT' directive between two 'CHECK' directives.
;  If the CHECK-NOT directive finds the word 'masked', then the test fails.
;
;  About the test-utils
;  This test uses 3 lines which start with RUN. These lines are executed by
;  the shell. We do not use pipes because they are not supported by
;  windows. The first line assembles this file to an llvm bitcode file.
;  The second line applies the optimization which we test (the resolver).
;  The third line interperts the 'CHECK' directives and actually tests
;  the results of the script.
;  The FileCheck utility, which comes with LLVM, is the test verifier.


; CHECK: @main
define void @main(float* %ptr0, float* %ptr1, i1 %pred) {
; CHECK-NOT: @masked
  %f = call float @masked_load(i1 %pred, float* %ptr0)
  ret void
; CHECK: ret
}

declare float @masked_load(i1 %pred, float* %ptr0)


