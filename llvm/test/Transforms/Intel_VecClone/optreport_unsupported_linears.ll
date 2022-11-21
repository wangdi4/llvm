; RUN: opt -enable-new-pm=0 -vec-clone -intel-opt-report=high -intel-ir-optreport-emitter -enable-intel-advanced-opts < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace --check-prefix=OPT-REPORT
; RUN: opt -passes='vec-clone,intel-ir-optreport-emitter' -intel-opt-report=high -enable-intel-advanced-opts < %s -disable-output 2>&1 | FileCheck %s --strict-whitespace --check-prefix=OPT-REPORT

; RUN: opt -enable-new-pm=0 -vec-clone  < %s -S 2>&1 | FileCheck %s --check-prefix=VEC-CLONE
; RUN: opt -passes='vec-clone' < %s -S 2>&1 | FileCheck %s --check-prefix=VEC-CLONE

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

;; #pragma omp declare simd linear(a:2) simdlen(4)
;; #pragma omp declare simd linear(ref(a):4) simdlen(4)
;; #pragma omp declare simd linear(uval(a):8) simdlen(4)
;; double add1(int& a, int& b)
;; {
;;     return a + b;
;; }

; OPT-REPORT:        Global optimization report for : _Z4add1RiS_S_
; OPT-REPORT-EMPTY:
; OPT-REPORT-NEXT:   FUNCTION REPORT BEGIN
; OPT-REPORT-NEXT:       remark: 'omp declare' vector variants with linear reference/uval()/val() or linear step passed as another argument were skipped as unsupported.
; OPT-REPORT-NEXT:   FUNCTION REPORT END
; OPT-REPORT-NEXT:   =================================================================

;; no vector variants to create
; VEC-CLONE-NOT: _ZGVb{{.*}}4L2v
; VEC-CLONE-NOT: _ZGVb{{.*}}4U8v

define double @_Z4add1RiS_S_(i32* align 4 dereferenceable(4) %a, i32* dereferenceable(4) %b) #0 {
entry:
  %0 = load i32, i32* %a, align 4
  %1 = load i32, i32* %b, align 4
  %add = add nsw i32 %1, %0
  %conv = sitofp i32 %add to double
  ret double %conv
}

attributes #0 = { "vector-variants"="_ZGVbN4L2v__Z4add1RiS_S_,_ZGVbM4L2v__Z4add1RiS_S_,_ZGVbN4R4v__Z4add1RiS_S_,_ZGVbM4R4v__Z4add1RiS_S_,_ZGVbN4U8v__Z4add1RiS_S_,_ZGVbM4U8v__Z4add1RiS_S_" }
