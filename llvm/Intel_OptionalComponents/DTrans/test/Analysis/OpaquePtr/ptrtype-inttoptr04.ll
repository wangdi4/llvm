; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test inttoptr conversion of an integer constant to a
; pointer type. Because the conversion is not taking place
; on something that was previously identified as possibly
; being a pointer to a structure type of interest, it does
; not need to be marked as 'unhandled'

%struct.test = type { i32, i32, i8*, i32 }

define void @test(%struct.test* "intel_dtrans_func_index"="1" nonnull %in) !intel.dtrans.func.type !4 {
  %cmp = icmp eq %struct.test* %in, null
  br i1 %cmp, label %t, label %f

t:
  br label %join

f:
 %gep = getelementptr %struct.test, %struct.test* %in, i64 0, i32 2
 br label %join

join:
  %phi = phi i8** [ inttoptr (i64 8 to i8**), %t ], [ %gep, %f ]
  %ptr = load i8*, i8** %phi, align 8
  ret void
}

; CHECK-NONOPAQUE: %ptr = load i8*, i8** %phi
; CHECK-OPAQUE: %ptr = load ptr, ptr %phi
; CHECK-NEXT: LocalPointerInfo
; CHECK-NOT: <DEPENDS ON UNHANDLED>
; CHECK: ret void

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!4 = distinct !{!3}
!5 = !{!"S", %struct.test zeroinitializer, i32 4, !1, !1, !2, !1} ; { i32, i32, i8*, i32 }

!intel.dtrans.types = !{!5}
