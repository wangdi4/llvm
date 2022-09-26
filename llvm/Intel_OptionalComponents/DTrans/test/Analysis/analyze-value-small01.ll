; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtransanalysis -indirectcallconv -disable-output -debug-only=dtransanalysis,dtrans-lpa < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtransanalysis>' -disable-output -debug-only=dtransanalysis,dtrans-lpa < %s 2>&1 | FileCheck %s

; Check that the -debug-only=dtransanalysis,dtrans-lpa does not print out
; the whole function in the 'analyzeValue' trace.

; CHECK: analyzeValue i32 ()* @foo
; CHECK: analyzeValue i32 ()* @bar

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

