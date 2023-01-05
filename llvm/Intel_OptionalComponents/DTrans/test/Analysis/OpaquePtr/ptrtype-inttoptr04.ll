; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

; Test inttoptr conversion of an integer constant to a
; pointer type. Because the conversion is not taking place
; on something that was previously identified as possibly
; being a pointer to a structure type of interest, it does
; not need to be marked as 'unhandled'

%struct.test = type { i32, i32, ptr, i32 }

define void @test(ptr "intel_dtrans_func_index"="1" nonnull %in) !intel.dtrans.func.type !4 {
  %cmp = icmp eq ptr %in, null
  br i1 %cmp, label %t, label %f

t:
  br label %join

f:
 %gep = getelementptr %struct.test, ptr %in, i64 0, i32 2
 br label %join

join:
  %phi = phi ptr [ inttoptr (i64 8 to ptr), %t ], [ %gep, %f ]
  %ptr = load ptr, ptr %phi, align 8
  ret void
}

; CHECK: %ptr = load ptr, ptr %phi
; CHECK-NEXT: LocalPointerInfo
; CHECK-NOT: <DEPENDS ON UNHANDLED>
; CHECK: ret void

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test zeroinitializer, i32 4, !1, !1, !2, !1} ; { i32, i32, i8*, i32 }

!intel.dtrans.types = !{!5}
