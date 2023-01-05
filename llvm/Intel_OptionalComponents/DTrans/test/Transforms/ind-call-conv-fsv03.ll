; UNSUPPORTED: enable-opaque-pointers

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -whole-program-assume -intel-libirc-allowed  -intel-ind-call-force-dtrans -passes=indirectcallconv < %s  -S 2>&1 | FileCheck %s

; Check that field single value indirect call specialization will specialize
; when the indirect call is done through a GEPOperator, even when the struct
; containing the function pointers is zero initialized.

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

%struct.MYSTRUCT = type { i32 ()*, i32 ()* }

@globstruct = internal global %struct.MYSTRUCT zeroinitializer, align 8

define dso_local i32 @main() {
  store i32 ()* @foo, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
  store i32 ()* @bar, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
  %t0 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
  %call = call i32 %t0()
  %t1 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
  %call1 = call i32 %t1()
  %add = add nsw i32 %call, %call1
  ret i32 %add
}
