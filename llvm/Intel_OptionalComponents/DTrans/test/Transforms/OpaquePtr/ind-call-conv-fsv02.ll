; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -intel-ind-call-force-dtrans -passes=indirectcallconv -S < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that field single value indirect call specialization will specialize
; when the indirect call is done through a GetElementPtrInst

; CHECK-NOT{{.*}}icmp{{.*}}
; CHECK:{{.*}}call i32 @foo()
; CHECK-NOT{{.*}}icmp{{.*}}
; CHECK:{{.*}}call i32 @bar()

define dso_local i32 @foo() {
  ret i32 5
}

define dso_local i32 @bar() {
  ret i32 5
}

%struct.MYSTRUCT = type { ptr, ptr }

@globstruct = internal global %struct.MYSTRUCT { i32 ()* @foo, i32 ()* @bar }, align 8

@globstructptr = internal global ptr @globstruct, align 8, !intel_dtrans_type !4

define dso_local i32 @main() {
  %t0 = load ptr, ptr @globstructptr, align 8
  %myfp1 = getelementptr inbounds %struct.MYSTRUCT, ptr %t0, i32 0, i32 0
  %t1 = load ptr, ptr %myfp1, align 8
  %call = call i32 %t1(), !intel_dtrans_type !1
  %t2 = load ptr, ptr @globstructptr, align 8
  %myfp2 = getelementptr inbounds %struct.MYSTRUCT, ptr %t2, i32 0, i32 1
  %t3 = load ptr, ptr %myfp2
  %call1 = call i32 %t3(), !intel_dtrans_type !1
  %add = add nsw i32 %call, %call1
  ret i32 %add
}
!1 = !{!"F", i1 false, i32 0, !2}  ; i32 ()
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 1}  ; i32 ()*
!4 = !{%struct.MYSTRUCT zeroinitializer, i32 1}  ; %struct.MYSTRUCT*
!5 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !3, !3} ; { i32 ()*, i32 ()* }

!intel.dtrans.types = !{!5}

