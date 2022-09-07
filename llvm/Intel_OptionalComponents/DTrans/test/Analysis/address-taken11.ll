; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Check arrays of structs, where the underlying structs are not identical.
; Check that AddressTaken is NOT set on 10 x MYSTRUCT, as 10 x MYSTRUCTX is NOT
; a compatible type.

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [10 x %struct.MYSTRUCT]
; CHECK: Number of elements: 10
; CHECK: Element LLVM Type: %struct.MYSTRUCT = type { i32, i32 }
; CHECK-NOT: Safety data:{{.*}}Address taken{{.*}}

%struct.MYSTRUCT = type { i32, i32 }
%struct.MYSTRUCTX = type { float, i32 }

@myarg = internal dso_local global [10 x %struct.MYSTRUCT] zeroinitializer, align 4
@fp = internal dso_local global i32 ([10 x %struct.MYSTRUCT]*)* null, align 8

define dso_local i32 @target1([10 x %struct.MYSTRUCT]* %arg) {
  %my1 = getelementptr inbounds [10 x %struct.MYSTRUCT], [10 x %struct.MYSTRUCT]* %arg, i64 1
  %my2 = getelementptr inbounds [10 x %struct.MYSTRUCT], [10 x %struct.MYSTRUCT]* %my1, i32 0, i32 0
  %my3 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %my2, i32 0, i32 1
  %t1 = load i32, i32* %my3, align 4
  ret i32 %t1
}

define dso_local i32 @target2([10 x %struct.MYSTRUCTX]* %arg) {
  %my1 = getelementptr inbounds [10 x %struct.MYSTRUCTX], [10 x %struct.MYSTRUCTX]* %arg, i64 1
  %my2 = getelementptr inbounds [10 x %struct.MYSTRUCTX], [10 x %struct.MYSTRUCTX]* %my1, i32 0, i32 0
  %my3 = getelementptr inbounds %struct.MYSTRUCTX, %struct.MYSTRUCTX* %my2, i32 0, i32 1
  %t1 = load i32, i32* %my3, align 4
  ret i32 %t1
}

define dso_local i32 @main() {
  %t0 = load i32 ([10 x %struct.MYSTRUCT]*)*, i32 ([10 x %struct.MYSTRUCT]*)** @fp, align 8
  %call = call i32 %t0([10 x %struct.MYSTRUCT]* @myarg)
  ret i32 %call
}
