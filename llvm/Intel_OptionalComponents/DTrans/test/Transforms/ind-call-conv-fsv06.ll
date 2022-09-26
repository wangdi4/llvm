; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -intel-ind-call-force-dtrans -dtransanalysis -indirectcallconv < %s -S 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -intel-ind-call-force-dtrans -passes=indirectcallconv < %s  -S 2>&1 | FileCheck %s

; Check that field single value indirect call specialization will specialize
; when the indirect call is done through a GEPOperator. A memset is used to
; write bad values into the struct containing the function pointers.  This
; will cause the specialization to be incomplete, and a fallback test will
; need to be generated.

; CHECK: [[ADDR1:%[a-z0-9]+]] = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
; CHECK: [[CMP1:%.[a-z0-9.]+]] = icmp eq i32 ()* [[ADDR1]], @foo
; CHECK: br i1 [[CMP1]], label %[[LABEL1:.[a-z0-9.]+]], label %[[LABEL2:.[a-z0-9.]+]]
; CHECK: [[LABEL1]]:
; CHECK: {{.}}call i32 @foo()
; CHECK: br label %[[LABEL3:.[a-z0-9.]+]]
; CHECK: [[LABEL2]]:
; CHECK: {{.}}call i32 [[ADDR1]]()
; CHECK: br label %[[LABEL3]]
; CHECK: [[LABEL3]]:
; CHECK: [[ADDR2:%[a-z0-9]+]] = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
; CHECK: [[CMP2:%.[a-z0-9.]+]] = icmp eq i32 ()* [[ADDR2]], @bar
; CHECK: br i1 [[CMP2]], label %[[LABEL4:.[a-z0-9.]+]], label %[[LABEL5:.[a-z0-9.]+]]
; CHECK: [[LABEL4]]:
; CHECK: {{.}}call i32 @bar()
; CHECK: br label %[[LABEL6:.[a-z0-9.]+]]
; CHECK: [[LABEL5]]:
; CHECK: {{.}}call i32 [[ADDR2]]()
; CHECK: br label %[[LABEL6]]
; CHECK: [[LABEL6]]:

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
  call void @llvm.memset.p0i8.i64(i8* %t2, i8 127, i64 16, i1 false)
  store i32 ()* @foo, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
  store i32 ()* @bar, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
  %t0 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 0), align 8
  %call = call i32 %t0()
  %t1 = load i32 ()*, i32 ()** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globstruct, i32 0, i32 1), align 8
  %call1 = call i32 %t1()
  %add = add nsw i32 %call, %call1
  ret i32 %add
}