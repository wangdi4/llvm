; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Following the change to perform constant folding on GEP.x.0.0 to eliminate GEP
; operators, analysis of the effective types is complicated when a PHI or select
; node input is a global variable that is an aggregate type. The resolved type for
; the global could represent the type of the global variable or element zero
; within it. Adding both types as the resolved type can result in safety flags
; or uses of the merged variable not being able to be analyzed. Instead, we will
; use the context of the other values in the SelectInst/PHINode to resolve
; whether to use the type of the global variable, or the element zero type.

; This test is similar to ptrtype-globalarray-gep01, but uses nested structures.

%struct.outer = type { %struct.middle, i64 }
%struct.middle = type { %struct.inner, i64, i64 }
%struct.inner = type { i64, i64, i64 }

@Nest = internal global %struct.outer zeroinitializer

define void @test1() {
; CHECK-LABEL: define void @test1()
  %f1 = getelementptr %struct.outer, ptr @Nest, i64 0, i32 0, i32 0, i32 1
  %s = select i1 undef, ptr @Nest, ptr %f1
; CHECK: %s = select i1 undef, ptr @Nest, ptr %f1
; CHECK-NEXT:    LocalPointerInfo:
; CHECK-NEXT:      Aliased types:
; CHECK-NEXT:        i64*{{ *}}
; CHECK-NEXT:      Element pointees:
; CHECK-NEXT:        %struct.inner @ 0
; CHECK-NEXT:        %struct.inner @ 1

  %v1 = load i64, ptr %s
  ret void
}

!1 = !{%struct.middle zeroinitializer, i32 0}  ; %struct.middle
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.inner zeroinitializer, i32 0}  ; %struct.inner
!4 = !{!"S", %struct.outer zeroinitializer, i32 2, !1, !2} ; { %struct.middle, i64 }
!5 = !{!"S", %struct.middle zeroinitializer, i32 3, !3, !2, !2} ; { %struct.inner, i64, i64 }
!6 = !{!"S", %struct.inner zeroinitializer, i32 3, !2, !2, !2} ; { i64, i64, i64 }

!intel.dtrans.types = !{!4, !5, !6}
