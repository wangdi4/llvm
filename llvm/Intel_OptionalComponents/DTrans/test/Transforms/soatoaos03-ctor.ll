; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos -dtrans-malloc-functions=struct.Mem,0                                     \
; RUN:  2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos-arrays -dtrans-malloc-functions=struct.Mem,0                              \
; RUN:  2>&1 | FileCheck --check-prefix=CHECK-TRANS %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; The following method should be classified as ctor.
; Instructions to transform are shown.
;   Arr(int c = 1, Mem *mem = nullptr)
;       : mem(mem), capacilty(c), size(0), base(nullptr) {
;     base = (S *)mem->allocate(capacilty * sizeof(S));
;   }
; CHECK:      Checking array's method _ZN3ArrIPiEC2EiP3Mem
; CHECK-NEXT: Classification: Ctor method
; CHECK-TRANS: ; Dump instructions needing update. Total = 3
define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %this, i32 %c, %struct.Mem* %mem) {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacilty = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %c, i32* %capacilty, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Nullify base pointer
; CHECK-TRANS-NEXT:  store i32** null, i32*** %base, align 8
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
  %call = call i8* %tmp2(%struct.Mem* %mem, i32 %conv4)
  %tmp3 = bitcast i8* %call to i32**
  %base5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TRANS:     ; BasePtrInst: Init base pointer with allocated memory
; CHECK-TRANS-NEXT:  store i32** %tmp3, i32*** %base5, align 8
  store i32** %tmp3, i32*** %base5, align 8
  ret void
}
