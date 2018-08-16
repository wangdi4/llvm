; RUN: opt -S < %s -whole-program-assume -disable-output \
; RUN:    -passes='require<dtransanalysis>,function(require<soatoaos-approx>,require<soatoaos-array-methods>)'  \
; RUN:    -dtrans-soatoaos-approx-typename=struct.Arr                                                           \
; RUN:    -dtrans-soatoaos-base-ptr-off=3 -dtrans-soatoaos-mem-off=0                                            \
; RUN:    -debug-only=dtrans-soatoaos -dtrans-free-functions=struct.Mem,1 \
; RUN:  2>&1 | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

$_ZN3ArrIPiED2Ev = comdat any

; The following method should be classified as ctor.
;   ~Arr() { mem->deallocate(base); }
; CHECK:      Checking array's method _ZN3ArrIPiED2Ev
; CHECK-NEXT: Classification: Dtor method
define void @_ZN3ArrIPiED2Ev(%struct.Arr* nocapture readonly %this) unnamed_addr #0 comdat align 2 {
entry:
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp1 = load i32**, i32*** %base, align 8
  %tmp2 = bitcast i32** %tmp1 to i8*
  %tmp3 = bitcast %struct.Mem* %tmp to void (%struct.Mem*, i8*)***
  %vtable = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp3, align 8
  %vfn = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable, i64 1
  %tmp4 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn, align 8
  call void %tmp4(%struct.Mem* %tmp, i8* %tmp2) #1
  ret void
}

attributes #0 = { noinline nounwind uwtable }
attributes #1 = { nounwind }
