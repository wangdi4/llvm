; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed  -intel-ind-call-force-dtrans -passes=indirectcallconv -S < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that field single value indirect call specialization will specialize
; when the indirect call is done through a GEPOperator. Check that the 0th
; struct field will be specialized with a fallback, because the field has
; been assigned a variable value, while the 1st struct field can be
; specialized without a fallback, because it is not assigned a variable value.

; CHECK: [[ADDR1:%[a-z0-9]+]] = load ptr, ptr @globstruct1, align 8
; CHECK: [[CMP1:%.[a-z0-9.]+]] = icmp eq ptr [[ADDR1]], @foo
; CHECK: br i1 [[CMP1]], label %[[LABEL1:.[a-z0-9.]+]], label %[[LABEL2:.[a-z0-9.]+]]
; CHECK: [[LABEL1]]:
; CHECK: {{.}}call i32 @foo()
; CHECK: br label %[[LABEL3:.[a-z0-9.]+]]
; CHECK: [[LABEL2]]:
; CHECK: {{.}}call i32 [[ADDR1]]()
; CHECK: br label %[[LABEL3]]
; CHECK: [[LABEL3]]:
; CHECK-NOT{{.*}}icmp{{.*}}
; CHECK:{{.*}}call i32 @bar()

define dso_local i32 @foo() {
  ret i32 5
}

define dso_local i32 @bar() {
  ret i32 5
}

%struct.MYSTRUCT = type { ptr, ptr }

@globstruct1 = internal global %struct.MYSTRUCT { i32 ()* @foo, i32 ()* @bar }, align 8

@globstruct2 = internal global %struct.MYSTRUCT { i32 ()* @foo, i32 ()* @bar }, align 8

define dso_local i32 @main() {
  %t3 = load ptr, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct2, i32 0, i32 0), align 8
  store ptr %t3, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct1, i32 0, i32 0), align 8
  %t0 = load ptr, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct1, i32 0, i32 0), align 8
  %call = call i32 %t0(), !intel_dtrans_type !1
  %t1 = load ptr, ptr getelementptr inbounds (%struct.MYSTRUCT, ptr @globstruct1, i32 0, i32 1), align 8
  %call1 = call i32 %t1(), !intel_dtrans_type !1
  %add = add nsw i32 %call, %call1
  ret i32 %add
}
!1 = !{!"F", i1 false, i32 0, !2}  ; i32 ()
!2 = !{i32 0, i32 0}  ; i32
!3 = !{!1, i32 1}  ; i32 ()*
!4 = !{!"S", %struct.MYSTRUCT zeroinitializer, i32 2, !3, !3} ; { i32 ()*, i32 ()* }

!intel.dtrans.types = !{!4}

