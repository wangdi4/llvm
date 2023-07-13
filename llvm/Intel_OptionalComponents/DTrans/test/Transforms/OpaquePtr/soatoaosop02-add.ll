; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck  --check-prefix=CHECK %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This test checks various approximations for side effects in append-like function.
; in SOA-to-AOS.
; void add(const S &e) {
;   realloc(1);
;   base[size] = e;
;   ++size;
; }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP

%struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }

define void @_ZN3ArrIPiE3addERKS0_(ptr "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %e) !intel.dtrans.func.type !9 {
entry:
; CHECK:      Known call (Func(Arg 0))
; CHECK-NEXT: call void @_ZN3ArrIPiE7reallocEi(
  call void @_ZN3ArrIPiE7reallocEi(ptr %this, i32 1)
  %tmp1 = load ptr, ptr %e, align 8
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  %tmp2 = load ptr, ptr %base, align 8
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  %tmp3 = load i32, ptr %size, align 8
  %idxprom = sext i32 %tmp3 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %tmp2, i64 %idxprom
; Write parameter into array.
; CHECK:      Store(Load(Arg 1))
; CHECK-NEXT:      (Func(Load(GEP(Arg 0)
; CHECK-NEXT:                     3))
; CHECK-NEXT:           (Load(GEP(Arg 0)
; CHECK-NEXT:                     4)))
; CHECK-NEXT: store ptr %tmp1, ptr %arrayidx, align 8
  store ptr %tmp1, ptr %arrayidx, align 8
  %size2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  %tmp4 = load i32, ptr %size2, align 8
  %new_size = add nsw i32 %tmp4, 1
; Update iteger size field
; CHECK:      Store(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                     4)))
; CHECK-NEXT:      (GEP(Arg 0)
; CHECK-NEXT:           4)
; CHECK-NEXT: store i32 %new_size, ptr %size2, align 8
  store i32 %new_size, ptr %size2, align 8
  ret void
}

declare !intel.dtrans.func.type !11 void @_ZN3ArrIPiE7reallocEi(ptr nocapture "intel_dtrans_func_index"="1", i32)

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
!9 = distinct !{!10, !8}
!10 = !{%struct.Arr zeroinitializer, i32 1}
!11 = !{!10}

; XCHECK: Deps computed: 14, Queries: 19

