; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -intel-ind-call-force-dtrans -dtransanalysis -indirectcallconv < %s -S 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -intel-ind-call-force-dtrans -passes=indirectcallconv < %s  -S 2>&1 | FileCheck %s

; Check that field single value indirect call specialization will specialize
; when the indirect call is done through a GetElementPtrInst, even when the
; struct containing the function pointers is zero initialized.

; Use a GetElementPtrInst with more than 2 indices.

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
%struct.MYOSTRUCT = type { i32, i32, %struct.MYSTRUCT }

@globstruct = internal global %struct.MYOSTRUCT zeroinitializer, align 8

@globstructptr = internal global %struct.MYOSTRUCT* @globstruct, align 8

define dso_local i32 @main() {
  store i32 ()* @foo, i32 ()** getelementptr inbounds (%struct.MYOSTRUCT, %struct.MYOSTRUCT* @globstruct, i32 0, i32 2, i32 0), align 8
  store i32 ()* @bar, i32 ()** getelementptr inbounds (%struct.MYOSTRUCT, %struct.MYOSTRUCT* @globstruct, i32 0, i32 2, i32 1), align 8
  %t0 = load %struct.MYOSTRUCT*, %struct.MYOSTRUCT** @globstructptr, align 8
  %myfp1 = getelementptr inbounds %struct.MYOSTRUCT, %struct.MYOSTRUCT* %t0, i32 0, i32 2, i32 0
  %t1 = load i32 ()*, i32 ()** %myfp1, align 8
  %call = call i32 %t1()
  %t2 = load %struct.MYOSTRUCT*, %struct.MYOSTRUCT** @globstructptr, align 8
  %myfp2 = getelementptr inbounds %struct.MYOSTRUCT, %struct.MYOSTRUCT* %t2, i32 0, i32 2, i32 1
  %t3 = load i32 ()*, i32 ()** %myfp2
  %call1 = call i32 %t3()
  %add = add nsw i32 %call, %call1
  ret i32 %add
}