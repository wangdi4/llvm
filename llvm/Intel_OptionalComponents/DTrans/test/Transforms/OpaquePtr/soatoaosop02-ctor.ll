; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This test checks various approximations for side effects in ctor-like function.
; Arr(int c = 1, Mem *mem = nullptr)
;     : mem(mem), capacity(c), size(0), base(nullptr) {
;   base = (S *)mem->allocate(capacity * sizeof(S));
; }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP

%struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }

define void @_ZN3ArrIPiEC2EiP3Mem(ptr "intel_dtrans_func_index"="1" %this, i32 %c, ptr "intel_dtrans_func_index"="2" %mem) !intel.dtrans.func.type !9 {
entry:
  %mem2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 0
  store ptr %mem, ptr %mem2, align 8
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 1
; Direct store of paraeter 1 to integer field #1.
; CHECK:      Store(Arg 1)
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           1)
; CHECK-NEXT: store i32 %c, ptr %capacity, align 8
  store i32 %c, ptr %capacity, align 8
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
; CHECK:      Store(Const)
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           3)
; CHECK-NEXT: store ptr null, ptr %base, align 8
  store ptr null, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
; Const initialization of integer field #4
; CHECK:      Store(Const)
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           4)
; CHECK-NEXT: store i32 0, ptr %size, align 8
  store i32 0, ptr %size, align 8
  %capacilty3 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 1
  %tmp3 = load i32, ptr %capacilty3, align 8
  %conv = sext i32 %tmp3 to i64
  %mul = mul i64 %conv, 8
  %call = call ptr @_ZN10MemManager8allocateEl(ptr %mem, i64 %mul)
  %base5 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  %ctor_base = bitcast ptr %base5 to ptr
; Store of malloc-ed pointer to base pointer field #3.
; CHECK:      Store(Alloc size(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                                1)))
; CHECK-NEXT:                 (Func(Arg 2)))
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           3)
; CHECK-NEXT: store ptr %call, ptr %ctor_base, align 8
  store ptr %call, ptr %ctor_base, align 8
  ret void
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1" ptr @_ZN10MemManager8allocateEl(ptr nocapture readnone "intel_dtrans_func_index"="2" %this, i64 %size) align 2 !intel.dtrans.func.type !11 {
entry:
  %call = call ptr @malloc(i64 %size)
  ret ptr %call
}

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

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
!11 = distinct !{!12, !5}
!12 = !{i8 0, i32 1}
!13 = distinct !{!12}

; XCHECK: Deps computed: 17, Queries: 24
