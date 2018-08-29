; RUN: opt -S < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-approx-typename=struct.Arr                                                           \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 \
; RUN:    -debug-only=dtrans-soatoaos \
; RUN:  2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

$_ZN3ArrIPiE3setEiS0_ = comdat any

; The following method should be classified as set-like method.
;   void set(int i, S val) { base[i] = val; }
; CHECK:      Checking array's method _ZN3ArrIPiE3setEiS0_
; CHECK-NEXT: Classification: Set element method
define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* nocapture readonly %this, i32 %i, i32* %val) #0 comdat align 2 {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp, i64 %idxprom
  store i32* %val, i32** %arrayidx, align 8
  ret void
}

attributes #0 = { noinline norecurse nounwind uwtable }
