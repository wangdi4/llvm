; RUN: opt -S < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      -dtrans-soatoaos-approx-typename=struct.Arr.0          \
; RUN:      -dtrans-free-functions=struct.Mem,1 \
; RUN:      2>&1 | FileCheck %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr.0 = type <{ %struct.Mem*, i32, [4 x i8], i8**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

$_ZN3ArrIPvED2Ev = comdat any

; This test checks various approximations for side effects in dtor-like function.
;   ~Arr() { mem->deallocate(base); }
define void @_ZN3ArrIPvED2Ev(%struct.Arr.0* nocapture readonly %this) unnamed_addr #0 comdat align 2 {
entry:
  %mem = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 0
  %tmp = load %struct.Mem*, %struct.Mem** %mem, align 8
  %base = getelementptr inbounds %struct.Arr.0, %struct.Arr.0* %this, i32 0, i32 3
  %tmp1 = load i8**, i8*** %base, align 8
  %tmp2 = bitcast i8** %tmp1 to i8*
  %tmp3 = bitcast %struct.Mem* %tmp to void (%struct.Mem*, i8*)***
  %vtable = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp3, align 8
  %vfn = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable, i64 1
  %tmp4 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn, align 8
; Free 'almost' base pointer, note Func.
; CHECK:      Free ptr(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                        3)))
; CHECK-NEXT:         (Func(Load(GEP(Arg 0)
; CHECK-NEXT:                        0))
; CHECK-NEXT:              (Load(Func(Load(Load(GEP(Arg 0)
; CHECK-NEXT:                                       0))))))
; CHECK-NEXT: call void %tmp4(%struct.Mem* %tmp, i8* %tmp2) #1
  call void %tmp4(%struct.Mem* %tmp, i8* %tmp2) #1
  ret void
}

attributes #0 = { noinline nounwind uwtable }
attributes #1 = { nounwind }

; CHECK: Deps computed: 12, Queries: 12
; CHECK-NOT: Unknown Dep
