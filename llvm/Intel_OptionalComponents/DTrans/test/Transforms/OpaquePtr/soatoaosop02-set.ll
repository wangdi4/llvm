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

; This test checks various approximations for side effects in set-like function.
; void set(int i, S val) { base[i] = val; }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP

%struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }

define void @_ZN3ArrIPiE3setEiS0_(ptr "intel_dtrans_func_index"="1" %this, i32 %i, ptr "intel_dtrans_func_index"="2" %set_val) !intel.dtrans.func.type !9 {
entry:
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  %tmp1 = load ptr, ptr %base, align 8
  %idxprom = sext i32 %i to i64
  %arrayidx = getelementptr inbounds ptr, ptr %tmp1, i64 %idxprom
; Write parameter into array, address depends on base pointer (field #3) and parameter.
; CHECK:      Store(Arg 2)
; CHECK-NEXT:      (Func(Arg 1)
; CHECK-NEXT:           (Load(GEP(Arg 0)
; CHECK-NEXT:                     3)))
; CHECK-NEXT:  store ptr %set_val, ptr %arrayidx, align 8
  store ptr %set_val, ptr %arrayidx, align 8
  ret void
}

; XCHECK: Deps computed: 9, Queries: 9

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
!9 = distinct !{!10, !11}
!10 = !{%struct.Arr zeroinitializer, i32 1}
!11 = !{i32 0, i32 1}
