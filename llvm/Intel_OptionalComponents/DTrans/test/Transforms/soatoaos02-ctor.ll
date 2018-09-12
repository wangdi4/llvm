; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      -dtrans-malloc-functions=struct.Mem,0 \
; RUN:      2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      -dtrans-malloc-functions=struct.Mem,0 \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; This test checks various approximations for side effects in ctor-like function.
; Arr(int c = 1, Mem *mem = nullptr)
;     : mem(mem), capacity(c), size(0), base(nullptr) {
;   base = (S *)mem->allocate(capacity * sizeof(S));
; }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP
define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* %this, i32 %c, %struct.Mem* %mem) {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
; Direct store of paraeter 1 to integer field #1.
; CHECK:      Store(Arg 1)
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           1)
; CHECK-NEXT: store i32 %c, i32* %capacity, align 8
  store i32 %c, i32* %capacity, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK:      Store(Const)
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           3)
; CHECK-NEXT: store i32** null, i32*** %base, align 8
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
; Const initialization of integer field #4
; CHECK:      Store(Const)
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           4)
; CHECK-NEXT: store i32 0, i32* %size, align 8
  store i32 0, i32* %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp3 = load i32, i32* %capacilty3, align 8
  %conv = sext i32 %tmp3 to i64
  %mul = mul i64 %conv, 8
  %conv4 = trunc i64 %mul to i32
  %tmp4 = bitcast %struct.Mem* %mem to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp4, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp5 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp5(%struct.Mem* %mem, i32 %conv4)
  %base5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %ctor_base = bitcast i32*** %base5 to i8**
; Store of malloc-ed pointer to base pointer field #3.
; CHECK:      Store(Alloc size(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                                1)))
; CHECK-NEXT:                 (Func(Arg 2)
; CHECK-NEXT:                      (Load(Func(Load(Arg 2))))))
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           3)
; CHECK-NEXT: store i8* %call, i8** %ctor_base, align 8
  store i8* %call, i8** %ctor_base, align 8
  ret void
}

; CHECK: Deps computed: 20, Queries: 26
