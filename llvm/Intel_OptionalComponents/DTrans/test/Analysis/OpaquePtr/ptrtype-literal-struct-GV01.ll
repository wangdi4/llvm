; It verifies that "foo" global variable is analyzed completely and
; { i8**, i64 }* is computed as aliased type for the literal struct by
; inferring type initializer even though metadata is not attached to it.

; REQUIRES: asserts
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; CHECK: @foo = internal global { ptr, i64 } { ptr @A1, i64 5 }
; CHECK-NEXT:    LocalPointerInfo: CompletelyAnalyzed{{ *$}}
; CHECK-NEXT:     Aliased types:
; CHECK-NEXT:        { i8**, i64 }*

%struct.test = type { i8 }

@GV = hidden unnamed_addr constant { [7 x ptr] } { [7 x ptr] [ptr null, ptr null, ptr null, ptr null, ptr null, ptr null, ptr null] }, !intel_dtrans_type !4

@A1 = unnamed_addr alias ptr, getelementptr inbounds ({ [7 x ptr] }, ptr @GV, i32 0, i32 0, i32 1)

@foo = internal global { ptr, i64 } { ptr @A1, i64 5 }, align 8

define void @reset() {
  %1 = tail call ptr @bar(i64 0)
  store ptr @foo, ptr %1, align 8
  ret void
}

declare !intel.dtrans.func.type !2 "intel_dtrans_func_index"="1" ptr @bar(i64)

!intel.dtrans.types = !{!0}

!0 = !{!"S", %struct.test zeroinitializer, i32 1, !1}
!1 = !{i8 0, i32 0}
!2 = distinct !{!3}
!3 = !{i8 0, i32 1}
!4 = !{!"L", i32 1, !5}
!5 = !{!"A", i32 7, !3}

