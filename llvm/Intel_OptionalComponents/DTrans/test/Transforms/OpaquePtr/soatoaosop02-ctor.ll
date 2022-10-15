; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-TY %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
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
define void @_ZN3ArrIPiEC2EiP3Mem(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32 %c, %struct.Mem* "intel_dtrans_func_index"="2" %mem)  !intel.dtrans.func.type !9 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  store %struct.Mem* %mem, %struct.Mem** %mem2, align 8
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
; Direct store of paraeter 1 to integer field #1.
; CHECK-TY:      Store(Arg 1)
; CHECK-TY-NEXT:      (GEP(Arg 0)
; CHECK-TY-NEXT:           1)
; CHECK-TY-NEXT: store i32 %c, i32* %capacity, align 8
; CHECK-OP:      Store(Arg 1)
; CHECK-OP-NEXT:      (GEP(Arg 0)
; CHECK-OP-NEXT:           1)
; CHECK-OP-NEXT: store i32 %c, ptr %capacity, align 8
  store i32 %c, i32* %capacity, align 8
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
; CHECK-TY:      Store(Const)
; CHECK-TY-NEXT:      (GEP(Arg 0)
; CHECK-TY-NEXT:           3)
; CHECK-TY-NEXT: store i32** null, i32*** %base, align 8
; CHECK-OP:      Store(Const)
; CHECK-OP-NEXT:      (GEP(Arg 0)
; CHECK-OP-NEXT:           3)
; CHECK-OP-NEXT: store ptr null, ptr %base, align 8
  store i32** null, i32*** %base, align 8
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
; Const initialization of integer field #4
; CHECK-TY:      Store(Const)
; CHECK-TY-NEXT:      (GEP(Arg 0)
; CHECK-TY-NEXT:           4)
; CHECK-TY-NEXT: store i32 0, i32* %size, align 8
; CHECK-OP:      Store(Const)
; CHECK-OP-NEXT:      (GEP(Arg 0)
; CHECK-OP-NEXT:           4)
; CHECK-OP-NEXT: store i32 0, ptr %size, align 8
  store i32 0, i32* %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp3 = load i32, i32* %capacilty3, align 8
  %conv = sext i32 %tmp3 to i64
  %mul = mul i64 %conv, 8
  %call = call i8* @_ZN10MemManager8allocateEl(%struct.Mem* %mem, i64 %mul)
  %base5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %ctor_base = bitcast i32*** %base5 to i8**
; Store of malloc-ed pointer to base pointer field #3.
; CHECK-TY:      Store(Alloc size(Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                                1)))
; CHECK-TY-NEXT:                 (Func(Arg 2)))
; CHECK-TY-NEXT:      (GEP(Arg 0)
; CHECK-TY-NEXT:           3)
; CHECK-TY-NEXT: store i8* %call, i8** %ctor_base, align 8
; CHECK-OP:      Store(Alloc size(Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                                1)))
; CHECK-OP-NEXT:                 (Func(Arg 2)))
; CHECK-OP-NEXT:      (GEP(Arg 0)
; CHECK-OP-NEXT:           3)
; CHECK-OP-NEXT: store ptr %call, ptr %ctor_base, align 8
  store i8* %call, i8** %ctor_base, align 8
  ret void
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1"  i8* @_ZN10MemManager8allocateEl(%struct.Mem* "intel_dtrans_func_index"="2" nocapture readnone %this, i64 %size) align 2 !intel.dtrans.func.type !13 {
entry:
  %call = call i8* @malloc(i64 %size)
  ret i8* %call
}

declare !intel.dtrans.func.type !12 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; XCHECK: Deps computed: 17, Queries: 24

!intel.dtrans.types = !{!0, !4}

!0 = !{!"S", %struct.Mem zeroinitializer, i32 1, !1}
!1 = !{!2, i32 2}
!2 = !{!"F", i1 true, i32 0, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"S", %struct.Arr zeroinitializer, i32 6, !5, !3, !6, !8, !3, !6}
!5 = !{%struct.Mem zeroinitializer, i32 1}
!6 = !{!"A", i32 4, !7}
!7 = !{i8 0, i32 0}
!8 = !{i32 0, i32 2}
!9 = distinct !{!10, !5}
!10 = !{%struct.Arr zeroinitializer, i32 1}
!11 = !{i8 0, i32 1}
!12 = distinct !{!11}
!13 = distinct !{!11, !5}
