; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -intel-ind-call-force-dtrans -passes=indirectcallconv -S < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that field single value indirect call specialization not specialize
; when there are no function targets.

; CHECK:{{.*}}call i32 %t0()
; CHECK-NOT:{{.*}}call i32 @foo()
; CHECK:{{.*}}call i32 %t1()
; CHECK-NOT:{{.*}}call i32 @bar()

define dso_local i32 @foo() {
  ret i32 5
}

define dso_local i32 @bar() {
  ret i32 5
}

%struct.MYSTRUCT = type { ptr, ptr }

@globstruct = internal global %struct.MYSTRUCT zeroinitializer, align 8

define dso_local i32 @main() {
  %t0 = load ptr, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct, i32 0, i32 0), align 8
  %call = call i32 %t0(), !intel_dtrans_type !1
  %t1 = load ptr, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct, i32 0, i32 1), align 8
  %call1 = call i32 %t1(), !intel_dtrans_type !1
  %add = add nsw i32 %call, %call1
  ret i32 %add
}
!1 = !{!"F", i1 false, i32 0, !2}  ; i32 ()
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 1}  ; i32 ()*
!4 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !3, !3} ; { i32 ()*, i32 ()* }

!intel.dtrans.types = !{!4}

