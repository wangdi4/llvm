; CMPLRLLVM-40763: This test verifies that foo is not inlined during pre-LTO
; compilation.

; RUN: opt -opaque-pointers -disable-output -passes='module(dtrans-force-inline-op),cgscc(inline)' -inline-report=0xe807 -prepare-for-lto -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain -pre-lto-inline-cost < %s 2>&1 | FileCheck -check-prefix=CHECK-SUPP %s
; RUN: opt -opaque-pointers -disable-output -passes='module(dtrans-force-inline-op),cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -inline-for-xmain < %s 2>&1 | FileCheck -check-prefix=CHECK-NINL %s

; CHECK-SUPP: COMPILE FUNC: bar
; CHECK-SUPP: foo {{.*}}Callsite preferred for SOA-to-AOS

; CHECK-NINL: COMPILE FUNC: bar
; CHECK-NINL: INLINE{{.*}}foo

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { ptr, i32 }
%struct.Arr = type { i8, i32 }

define void @bar() {
entry:
  %call10 = call ptr @baz()
  call void @foo(ptr %call10)
  ret void
}

define linkonce_odr "intel_dtrans_func_index"="1" ptr @baz() !intel.dtrans.func.type !5 {
entry:
  ret ptr null
}

define linkonce_odr void @foo(ptr "intel_dtrans_func_index"="1" %key) !intel.dtrans.func.type !7 {
entry:
  %fKey = getelementptr inbounds %struct.test01, ptr null, i64 0, i32 0
  store ptr %key, ptr %fKey, align 8
  ret void
}

!intel.dtrans.types = !{!0, !3}

!0 = !{!"S", %struct.Arr zeroinitializer, i32 2, !1, !2}
!1 = !{i8 0, i32 0}
!2 = !{i32 0, i32 0}
!3 = !{!"S", %struct.test01 zeroinitializer, i32 2, !4, !2}
!4 = !{i8 0, i32 1}
!5 = distinct !{!6}    ; %struct.Arr* baz();
!6 = !{%struct.Arr zeroinitializer, i32 1}
!7 = distinct !{!4}   ; foo(i8*)
