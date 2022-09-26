; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  -whole-program-assume -intel-ind-call-force-dtrans -dtransanalysis -indirectcallconv < %s -S 2>&1 | FileCheck %s
; RUN: opt  -whole-program-assume -intel-ind-call-force-dtrans -passes=indirectcallconv < %s -S 2>&1 | FileCheck %s

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

%struct.MYSTRUCT = type { i32 ()*, i32 ()* }

@globstruct = internal global %struct.MYSTRUCT { i32 ()* @foo, i32 ()* @bar }, align 8

@globstructptr = internal global %struct.MYSTRUCT* @globstruct, align 8

define dso_local i32 @main() {
  %t0 = load %struct.MYSTRUCT*, %struct.MYSTRUCT** @globstructptr, align 8
  %myfp1 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %t0, i32 0, i32 0
  %t1 = load i32 ()*, i32 ()** %myfp1, align 8
  %call = call i32 %t1()
  %t2 = load %struct.MYSTRUCT*, %struct.MYSTRUCT** @globstructptr, align 8
  %myfp2 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %t2, i32 0, i32 1
  %t3 = load i32 ()*, i32 ()** %myfp2
  %call1 = call i32 %t3()
  %add = add nsw i32 %call, %call1
  ret i32 %add
}