; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -intel-ind-call-force-dtrans -dtransanalysis -indirectcallconv < %s -S 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -intel-ind-call-force-dtrans -passes=indirectcallconv < %s  -S 2>&1 | FileCheck %s

; Check that field single value indirect call specialization will specialize
; when the indirect call is done through a GEPOperator, even when memset
; is used to zero out the struct containing the function pointers.

; CHECK-NOT{{.*}}icmp{{.*}}
; CHECK:{{.*}}call i32 @foo()
; CHECK-NOT{{.*}}icmp{{.*}}
; CHECK:{{.*}}call i32 @bar()

; Needed to specify we are using 8 byte pointers (for memset)
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

define dso_local i32 @foo() {
  ret i32 5
}

define dso_local i32 @bar() {
  ret i32 5
}

%struct.MYSTRUCT = type { i32 ()*, i32 ()* }

@globstruct = internal global %struct.MYSTRUCT zeroinitializer, align 8

declare void @llvm.memset.p0i8.i64(i8*, i8, i64, i1)

define dso_local i32 @main() {
  %t2 = bitcast %struct.MYSTRUCT* @globstruct to i8*
  call void @llvm.memset.p0i8.i64(i8* %t2, i8 0, i64 16, i1 false)
  store i32 ()* @foo, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
  store i32 ()* @bar, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
  %t0 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
  %call = call i32 %t0()
  %t1 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
  %call1 = call i32 %t1()
  %add = add nsw i32 %call, %call1
  ret i32 %add
}