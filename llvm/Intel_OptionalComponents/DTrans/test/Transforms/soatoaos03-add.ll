; RUN: opt -S < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-approx-typename=struct.Arr                                                           \
; RUN:    -dtrans-soatoaos-approx-known-func=_ZN3ArrIPiE7reallocEi  -dtrans-soatoaos-base-ptr-off=3             \
; RUN:    -debug-only=dtrans-soatoaos \
; RUN:  2>&1 | FileCheck %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

$_ZN3ArrIPiE3addERKS0_ = comdat any

; The following method should be classified as append-like.
;   void add(const S &e) {
;     realloc(1);
;
;     base[size] = e;
;     ++size;
;   }
; CHECK:      Checking array's method _ZN3ArrIPiE3addERKS0_
; CHECK-NEXT: Classification: Append element method
define void @_ZN3ArrIPiE3addERKS0_(%struct.Arr* nocapture %this, i32** nocapture readonly dereferenceable(8) %e) #0 comdat align 2 {
entry:
  call void @_ZN3ArrIPiE7reallocEi(%struct.Arr* %this, i32 1)
  %tmp = load i32*, i32** %e, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp1 = load i32**, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp2 = load i32, i32* %size, align 8
  %idxprom = sext i32 %tmp2 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp1, i64 %idxprom
  store i32* %tmp, i32** %arrayidx, align 8
  %size2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp3 = load i32, i32* %size2, align 8
  %inc = add nsw i32 %tmp3, 1
  store i32 %inc, i32* %size2, align 8
  ret void
}

; Function Attrs: noinline uwtable
declare void @_ZN3ArrIPiE7reallocEi(%struct.Arr* nocapture, i32) #0 align 2

attributes #0 = { noinline uwtable }
