; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos -dtrans-free-functions=struct.Mem,1                                       \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos-arrays -dtrans-free-functions=struct.Mem,1                                \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume                                                                        \
; RUN:    -passes=soatoaos-arrays-methods-transform                                                             \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -dtrans-free-functions=struct.Mem,1                                                                   \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }
; CHECK-MOD-DAG: %__SOA_struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], %__SOA_EL_struct.Arr*, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr = type { float*, i32* }

; The following method should be classified as dtor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   ~Arr() { mem->deallocate(base); }
; CHECK:      Checking array's method _ZN3ArrIPiED2Ev
; CHECK-NEXT: Classification: Dtor method
; CHECK-TRANS: ; Dump instructions needing update. Total = 1
define void @_ZN3ArrIPiED2Ev(%struct.Arr* %this) {
entry:
; CHECK-MOD:   %mem = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 0
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
; CHECK-MOD:   %base = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 3
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Load of base pointer
; CHECK-TRANS-NEXT:  %tmp1 = load i32**, i32*** %base, align 8
; CHECK-MOD:   %tmp1 = load %__SOA_EL_struct.Arr*, %__SOA_EL_struct.Arr** %base, align 8
  %tmp1 = load i32**, i32*** %base, align 8
; CHECK-MOD:   %tmp2 = bitcast %__SOA_EL_struct.Arr* %tmp1 to i8*
  %tmp2 = bitcast i32** %tmp1 to i8*
  %tmp3 = bitcast %struct.Mem* %tmp to void (%struct.Mem*, i8*)***
  %vtable = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp3, align 8
  %vfn = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable, i64 1
  %tmp4 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn, align 8
  call void %tmp4(%struct.Mem* %tmp, i8* %tmp2)
  ret void
}
