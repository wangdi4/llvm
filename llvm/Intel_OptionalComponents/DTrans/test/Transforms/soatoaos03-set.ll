; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3                                                                       \
; RUN:    -debug-only=dtrans-soatoaos                                                                           \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3                                                                       \
; RUN:    -debug-only=dtrans-soatoaos-arrays                                                                    \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume                                                                        \
; RUN:    -passes=soatoaos-arrays-methods-transform -dtrans-soatoaos-base-ptr-off=3                             \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK-MOD: %__SOA_struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], %__SOA_EL_struct.Arr*, i32, [4 x i8] }>
; CHECK-MOD: %__SOA_EL_struct.Arr = type { float*, i32* }
%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; The following method should be classified as set-like method.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   void set(int i, S val) { base[i] = val; }
; CHECK:      Checking array's method _ZN3ArrIPiE3setEiS0_
; CHECK-NEXT: Classification: Set element method
; CHECK-TRANS: ; Dump instructions needing update. Total = 3
define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* %this, i32 %i, i32* %val) {
entry:
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:   %tmp = load i32**, i32*** %base, align 8
; CHECK-MOD:          %tmp = load %__SOA_EL_struct.Arr*, %__SOA_EL_struct.Arr** %base, align 8
  %tmp = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i to i64
; CHECK-TRANS:      ; MemInstGEP: Element set from arg
; CHECK-TRANS-NEXT:   %arrayidx = getelementptr inbounds i32*, i32** %tmp, i64 %idxprom
; CHECK-MOD:          %arrayidx = getelementptr inbounds %__SOA_EL_struct.Arr, %__SOA_EL_struct.Arr* %tmp, i64 %idxprom
; CHECK-MOD-NEXT:     %elem = getelementptr inbounds %__SOA_EL_struct.Arr, %__SOA_EL_struct.Arr* %arrayidx, i64 0, i32 1
  %arrayidx = getelementptr inbounds i32*, i32** %tmp, i64 %idxprom
; CHECK-TRANS:      ; MemInst: Element set from arg
; CHECK-TRANS-NEXT:   store i32* %val, i32** %arrayidx, align 8
  store i32* %val, i32** %arrayidx, align 8
  ret void
}
