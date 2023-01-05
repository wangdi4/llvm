; CMPLRLLVM-40427: This test verifies that bar2 is not inlined during pre-LTO
; compilation.

; RUN: opt -opaque-pointers -disable-output -passes='module(dtrans-force-inline-op),cgscc(inline)' -inline-report=0xe807 -prepare-for-lto -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain -pre-lto-inline-cost < %s 2>&1 | FileCheck -check-prefix=CHECK-SUPP %s
; RUN: opt -opaque-pointers -disable-output -passes='module(dtrans-force-inline-op),cgscc(inline)' -inline-report=0xe807 -dtrans-inline-heuristics -intel-libirc-allowed -inline-for-xmain < %s 2>&1 | FileCheck -check-prefix=CHECK-NINL %s

; CHECK-SUPP: COMPILE FUNC: foo
; CHECK-SUPP: bar2 {{.*}}Callsite preferred for SOA-to-AOS

; CHECK-NINL: COMPILE FUNC: foo
; CHECK-NINL: INLINE{{.*}}bar2

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Arr = type { i8, i32 }

define void @foo(ptr "intel_dtrans_func_index"="1" %value) !intel.dtrans.func.type !2 {
entry:
  call void @bar1(ptr null, ptr %value)
  call void @bar2(ptr %value)
  ret void
}

declare !intel.dtrans.func.type !6 void @bar1(ptr "intel_dtrans_func_index"="1" , ptr "intel_dtrans_func_index"="2")

define linkonce_odr void @bar2(ptr "intel_dtrans_func_index"="1" %implementationFeatures) !intel.dtrans.func.type !5 {
entry:
  store i32 20, ptr %implementationFeatures
  ret void
}

!intel.dtrans.types = !{!7}

!1 = !{i8 0, i32 1}
!2 = distinct !{!1}  ; foo(i8*)
!3 = !{i16 0, i32 1}
!4 = !{i32 0, i32 1}
!5 = distinct !{!3}   ; bar2(i16*)
!6 = distinct !{!10, !4}   ; bar1(%struct.Arr*, i32*)
!7 = !{!"S", %struct.Arr zeroinitializer, i32 2, !8, !9}
!8 = !{i8 0, i32 0}
!9 = !{i32 0, i32 0}
!10 = !{%struct.Arr zeroinitializer, i32 1}
