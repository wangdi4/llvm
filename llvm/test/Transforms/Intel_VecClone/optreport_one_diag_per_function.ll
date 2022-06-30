; RUN: opt -enable-new-pm=0 -vec-clone -intel-opt-report=high -intel-ir-optreport-emitter -enable-intel-advanced-opts < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace --check-prefix=CHECK-OPT-REPORT
; RUN: opt -passes='vec-clone,intel-ir-optreport-emitter' -intel-opt-report=high -enable-intel-advanced-opts < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace --check-prefix=CHECK-OPT-REPORT

; RUN: opt -enable-new-pm=0 -vec-clone  < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-VEC-CLONE
; RUN: opt -passes='vec-clone' < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-VEC-CLONE

;; Check that the diagnostic is issued only for the original function,
;; and is not duplicated for the remaining vector variants.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;; #pragma omp declare simd uniform(s) linear(a:s) simdlen(4)
;; #pragma omp declare simd uniform(s) simdlen(4)
;; double add1(int a, int b, int s)
;; {
;;   return a + b;
;; }

; CHECK-OPT-REPORT:        Global optimization report for : _Z4add1iii
; CHECK-OPT-REPORT-EMPTY:
; CHECK-OPT-REPORT-NEXT:   FUNCTION REPORT BEGIN
; CHECK-OPT-REPORT-NEXT:       remark: 'omp declare' vector variants with linear reference/ref()/uval()/val() or linear step passed as another argument were skipped as unsupported.
; CHECK-OPT-REPORT-NEXT:   FUNCTION REPORT END

;; no more remarks
; CHECK-OPT-REPORT-NOT:    remark:

; CHECK-VEC-CLONE: "vector-variants"="_ZGVeN4vvu__Z4add1iii,_ZGVeM4vvu__Z4add1iii"

define double @_Z4add1iii(i32 %a, i32 %b, i32 %s) #0 {
entry:
  %add = add nsw i32 %b, %a
  %conv = sitofp i32 %add to double
  ret double %conv
}

attributes #0 = { "vector-variants"="_ZGVeN4vvu__Z4add1iii,_ZGVeM4vvu__Z4add1iii,_ZGVeN4ls2vu__Z4add1iii,_ZGVeM4ls2vu__Z4add1iii" }

