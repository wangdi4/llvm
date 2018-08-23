; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      -dtrans-soatoaos-approx-known-func=_ZN3ArrIPiE7reallocEi 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      -dtrans-soatoaos-approx-known-func=_ZN3ArrIPiE7reallocEi 2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; This test checks various approximations for side effects in append-like function.
; in SOA-to-AOS.
; void add(const S &e) {
;   realloc(1);
;   base[size] = e;
;   ++size;
; }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP
define void @_ZN3ArrIPiE3addERKS0_(%struct.Arr* %this, i32** %e) {
entry:
; CHECK:      Known call (Func(Arg 0))
; CHECK-NEXT: call void @_ZN3ArrIPiE7reallocEi(%struct.Arr* %this, i32 1)
  call void @_ZN3ArrIPiE7reallocEi(%struct.Arr* %this, i32 1)
  %tmp1 = load i32*, i32** %e, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp2 = load i32**, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp3 = load i32, i32* %size, align 8
  %idxprom = sext i32 %tmp3 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp2, i64 %idxprom
; Write parameter into array.
; CHECK:      Store(Load(Arg 1))
; CHECK-NEXT:      (Func(Load(GEP(Arg 0)
; CHECK-NEXT:                     3))
; CHECK-NEXT:           (Load(GEP(Arg 0)
; CHECK-NEXT:                     4)))
; CHECK-NEXT: store i32* %tmp1, i32** %arrayidx, align 8
  store i32* %tmp1, i32** %arrayidx, align 8
  %size2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp4 = load i32, i32* %size2, align 8
  %new_size = add nsw i32 %tmp4, 1
; Update iteger size field
; CHECK:      Store(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                     4)))
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           4)
; CHECK-NEXT: store i32 %new_size, i32* %size2, align 8
  store i32 %new_size, i32* %size2, align 8
  ret void
}

declare void @_ZN3ArrIPiE7reallocEi(%struct.Arr* nocapture, i32)

; CHECK: Deps computed: 14, Queries: 19
