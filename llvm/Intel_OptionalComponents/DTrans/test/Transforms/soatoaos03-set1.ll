; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=2                                                                       \
; RUN:    -debug-only=dtrans-soatoaos-arrays,dtrans-soatoaos                                                    \
; RUN:  2>&1 | FileCheck %s
; RUN: opt -S < %s -whole-program-assume                                                                        \
; RUN:    -passes=soatoaos-arrays-methods-transform -dtrans-soatoaos-base-ptr-off=2                             \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; CHECK-MOD-DAG: %__SOA_struct.Arr = type <{ i32, [4 x i8], %__SOA_EL_struct.Arr*, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr = type { float*, i32* }
%struct.Arr = type <{ i32, [4 x i8], i32**, i32, [4 x i8] }>

; Special version of set method setting only the 0th element.

; CHECK:      Checking array's method _ZN3ArrIPiE3setEiS0_
; Transformed instructions are shown.
;   void set(S val) { base[0] = val; }
; CHECK-NEXT: Classification: Set element method
; CHECK-NEXT: ; Dump instructions needing update. Total = 2
define void @_ZN3ArrIPiE3setEiS0_(%struct.Arr* nocapture readonly %arg, i32 %arg1, i32* %arg2) align 2 {
bb:
  %tmp = getelementptr inbounds %struct.Arr, %struct.Arr* %arg, i64 0, i32 2
; CHECK:      ; BasePtrInst: Load of base pointer
; CHECK-NEXT:   %tmp3 = load i32**, i32*** %tmp, align 8
; CHECK-MOD:    %tmp3 = load %__SOA_EL_struct.Arr*, %__SOA_EL_struct.Arr** %tmp, align 8
  %tmp3 = load i32**, i32*** %tmp, align 8
; CHECK:      ; MemInst: Element set from arg
; CHECK-NEXT:   store i32* %arg2, i32** %tmp3, align 8
; CHECK-MOD-NEXT:  %elem = getelementptr inbounds %__SOA_EL_struct.Arr, %__SOA_EL_struct.Arr* %tmp3, i64 0, i32 1
; CHECK-MOD-NEXT:  store i32* %arg2, i32** %elem, align 8
  store i32* %arg2, i32** %tmp3, align 8
  ret void
}
