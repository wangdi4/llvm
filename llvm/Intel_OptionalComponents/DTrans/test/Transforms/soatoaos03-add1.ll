; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3                                                                       \
; RUN:    -debug-only=dtrans-soatoaos                                                                           \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3                                                                       \
; RUN:    -debug-only=dtrans-soatoaos-arrays                                                                    \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume                                                                        \
; RUN:    -passes=soatoaos-arrays-methods-transform                                                             \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -dtrans-malloc-functions=struct.Mem,0                                                                 \
; RUN:    -dtrans-optbase-process-function-declaration                                                          \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }
; CHECK-MOD-DAG: %__SOA_struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], %__SOA_EL_struct.Arr*, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr = type { float*, i32* }

; The following method should be classified as append-like.
; Instructions to transform are shown.
;   void add(S e) {
;     realloc(1);
;
;     base[size] = e;
;     ++size;
;   }
; CHECK:      Checking array's method _ZN3ArrIPiE3addERKS0_
; CHECK-NEXT: Classification: Append element method

; CHECK-TRANS: ; Dump instructions needing update. Total = 3
; CHECK-MOD: @_ZN3ArrIPiE3addERKS0_{{.*}}(%__SOA_struct.Arr* %this, i32* %e, float* %0)
define void @_ZN3ArrIPiE3addERKS0_(%struct.Arr* %this, i32* %e) {
entry:
  call void @_ZN3ArrIPiE7reallocEi(%struct.Arr* %this, i32 1)
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:      ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT: %tmp1 = load i32**, i32*** %base, align 8
; CHECK-MOD:        %tmp1 = load %__SOA_EL_struct.Arr*, %__SOA_EL_struct.Arr** %base, align 8
  %tmp1 = load i32**, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp2 = load i32, i32* %size, align 8
  %idxprom = sext i32 %tmp2 to i64
; CHECK-TRANS:      ; MemInstGEP: Element set from arg
; CHECK-TRANS-NEXT: %arrayidx = getelementptr inbounds i32*, i32** %tmp1, i64 %idxprom
; CHECK-MOD:         %arrayidx = getelementptr inbounds %__SOA_EL_struct.Arr, %__SOA_EL_struct.Arr* %tmp1, i64 %idxprom
  %arrayidx = getelementptr inbounds i32*, i32** %tmp1, i64 %idxprom
; CHECK-MOD-NEXT:    %elem1 = getelementptr inbounds %__SOA_EL_struct.Arr, %__SOA_EL_struct.Arr* %arrayidx, i64 0, i32 1
; CHECK-MOD-NEXT:    %elem = getelementptr inbounds %__SOA_EL_struct.Arr, %__SOA_EL_struct.Arr* %arrayidx, i64 0, i32 0
; CHECK-TRANS:      ; MemInst: Element set from arg
; CHECK-TRANS-NEXT: store i32* %e, i32** %arrayidx, align 8
; CHECK-MOD-NEXT:    store float* %0, float** %elem, align 8
; CHECK-MOD-NEXT:    store i32* %e, i32** %elem1, align 8
  store i32* %e, i32** %arrayidx, align 8
  %size2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp3 = load i32, i32* %size2, align 8
  %inc = add nsw i32 %tmp3, 1
  store i32 %inc, i32* %size2, align 8
  ret void
}

declare void @_ZN3ArrIPiE7reallocEi(%struct.Arr*, i32)
