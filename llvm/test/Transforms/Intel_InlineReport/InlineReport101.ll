; RUN: opt -inline -argpromotion -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-NO-MD
; RUN: opt -passes='cgscc(inline)',argpromotion -inline-report=0xe807 < %s -S 2>&1 | FileCheck %s --check-prefixes=CHECK-NO-MD

; RUN: opt -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -argpromotion -inline-report=0xe886 -S | opt  -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)',argpromotion -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD

; This test case checks that the result for the inline report is correct
; when argument promotion is applied after inlining. In this case we aren't
; looking if inlining was applied or not (which it won't be since function
; @bar is set as no-inline). The goal of this test case is to make sure that
; the inlining information is preserved when argument promotion is applied.

; Check that argument promotion was applied and the result of
; the traditional inline report is correct.

; CHECK-NO-MD:      define internal float @bar(float %.0.val) #0 {
; CHECK-NO-MD-NEXT:   ret float %.0.val
; CHECK-NO-MD-NEXT: }

; CHECK-NO-MD:      define float @foo(i64 %0) #1 {
; CHECK-NO-MD-NEXT:   %2 = alloca float, i64 %0, align 4
; CHECK-NO-MD-NEXT:   %.val = load float, float* %2, align 4
; CHECK-NO-MD-NEXT:   %3 = call float @bar(float %.val)

; CHECK-NO-MD: COMPILE FUNC: foo
; CHECK-NO-MD:   bar{{.*}}Callee has noinline attribute


; Check that argument promotion was applied and the result of the
; metadata based inline report is correct.

; CHECK-MD: COMPILE FUNC: foo
; CHECK-MD:   bar{{.*}}Callee has noinline attribute

; CHECK-MD:      define internal float @bar(float %.0.val) #0 !intel.function.inlining.report !0 {
; CHECK-MD-NEXT:   ret float %.0.val

; CHECK-MD:      define float @foo(i64 %0) #1 !intel.function.inlining.report !8 {
; CHECK-MD-NEXT:   %2 = alloca float, i64 %0, align 4
; CHECK-MD-NEXT:   %.val = load float, float* %2, align 4
; CHECK-MD-NEXT:   %3 = call float @bar(float %.val), !intel.callsite.inlining.report !11


target datalayout = "E-p:64:64:64-a0:0:8-f32:32:32-f64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-v64:64:64-v128:128:128"

define internal float @bar(float *%0) #0 {
  %2 = load float, float* %0
  ret float %2
}

define float @foo(i64 %0) nounwind  {
  %2 = alloca float, i64 %0
  %3 = call float @bar(float *%2)
  ret float %3
}

attributes #0 = { noinline }
