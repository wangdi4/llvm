; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos -dtrans-malloc-functions=struct.Mem,0                                     \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output                                                           \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos-arrays -dtrans-malloc-functions=struct.Mem,0                              \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; RUN: opt -S < %s -whole-program-assume                                                                        \
; RUN:    -passes=soatoaos-arrays-methods-transform                                                             \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -dtrans-malloc-functions=struct.Mem,0                                                                 \
; RUN:  | FileCheck --check-prefix=CHECK-MOD %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }
; CHECK-MOD-DAG: %__SOA_struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], %__SOA_EL_struct.Arr*, i32, [4 x i8] }>
; CHECK-MOD-DAG: %__SOA_EL_struct.Arr = type { float*, i32* }

; The following method should be classified as ctor.
; Instructions to transform are shown.
; Transformed instructions are shown.
;   Arr(int c = 1, Mem *mem = nullptr)
;       : mem(mem), capacilty(c), size(0), base(nullptr) {
;     base = (S *)mem->allocate(capacilty * sizeof(S));
;   }
; CHECK:      Checking array's method _ZN3ArrIPiEC2EiP3Mem
; CHECK-NEXT: Classification: Ctor method
; CHECK-TRANS: ; Dump instructions needing update. Total = 3
define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %this, i32 %c, %struct.Mem* %mem) {
entry:
; CHECK-MOD:  %mem2 = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 0
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
; CHECK-MOD:  %capacilty = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 1
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
; CHECK-MOD:  %base = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 3
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Nullify base pointer
; CHECK-TRANS-NEXT:  store i32** null, i32*** %base, align 8
; CHECK-MOD:  store %__SOA_EL_struct.Arr* null, %__SOA_EL_struct.Arr** %base, align 8
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  store i32 0, i32* %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp = load i32, i32* %capacilty3, align 8
  %conv = sext i32 %tmp to i64
  %mul = mul i64 %conv, 8
  %conv4 = trunc i64 %mul to i32
  %tmp1 = bitcast %struct.Mem* %mem to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp1, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp2 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
; CHECK-TRANS:     ; BasePtrInst: Allocation call
; CHECK-TRANS-NEXT:  %call = call i8* %tmp2(%struct.Mem* %mem, i32 %conv4)
; CHECK-MOD:       %nsz = zext i32 %conv4 to i64
; CHECK-MOD-NEXT:  %nsz1 = mul nuw i64 %nsz, 2
; CHECK-MOD-NEXT:  %nsz2 = trunc i64 %nsz1 to i32
; CHECK-MOD-NEXT:  %call = call i8* %tmp2(%struct.Mem* %mem, i32 %nsz2)
  %call = call i8* %tmp2(%struct.Mem* %mem, i32 %conv4)
; CHECK-MOD-NEXT:  %tmp3 = bitcast i8* %call to %__SOA_EL_struct.Arr*
  %tmp3 = bitcast i8* %call to i32**
; CHECK-MOD-NEXT:  %base5 = getelementptr inbounds %__SOA_struct.Arr, %__SOA_struct.Arr* %this, i32 0, i32 3
  %base5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-TRANS-NEXT:  store i32** %tmp3, i32*** %base5, align 8
; CHECK-MOD-NEXT:  store %__SOA_EL_struct.Arr* %tmp3, %__SOA_EL_struct.Arr** %base5, align 8
  store i32** %tmp3, i32*** %base5, align 8
  ret void
}
