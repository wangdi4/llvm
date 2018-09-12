; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; This test checks various approximations for side effects in set-like function.
; void set(int i, S val) { base[i] = val; }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP
define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* %this, i32 %i, i32* %set_val) {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp1 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp1, i64 %idxprom
; Write parameter into array, address depends on base pointer (field #3) and parameter.
; CHECK:      Store(Arg 2)
; CHECK-NEXT:      (Func(Arg 1)
; CHECK-NEXT:           (Load(GEP(Arg 0)
; CHECK-NEXT:                     3)))
; CHECK-NEXT:  store i32* %set_val, i32** %arrayidx, align 8
  store i32* %set_val, i32** %arrayidx, align 8
  ret void
}

; CHECK: Deps computed: 9, Queries: 9
