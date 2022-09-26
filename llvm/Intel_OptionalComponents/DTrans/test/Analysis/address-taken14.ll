; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Check arrays of two structs with different types.
; Check with an external call, rather than an indirect call.
; Check that AddressTaken is NOT set on 10 x MYSTRUCT, as 10 x MYSTRUCTX is NOT
; a compatible type. But the call to @target1 is external, so MYSTRUCT should
; have AddressTaken set.

; CHECK: DTRANS_ArrayInfo:
; CHECK: LLVMType: [10 x %struct.MYSTRUCT]
; CHECK: Number of elements: 10
; CHECK: Element LLVM Type: %struct.MYSTRUCT = type { i32, i32 }
; CHECK: Safety data:{{.*}}Address taken{{.*}}

%struct.MYSTRUCT = type { i32, i32 }
%struct.MYSTRUCTX = type { float, i32 }

@myarg = internal dso_local global [10 x %struct.MYSTRUCT] zeroinitializer, align 4
@fp = internal dso_local global i32 ([10 x %struct.MYSTRUCT]*)* null, align 8

declare dso_local i32 @target1([10 x %struct.MYSTRUCT]* %arg)

define dso_local i32 @target2([10 x %struct.MYSTRUCTX]* %arg) {
  %my1 = getelementptr inbounds [10 x %struct.MYSTRUCTX], [10 x %struct.MYSTRUCTX]* %arg, i64 1
  %my2 = getelementptr inbounds [10 x %struct.MYSTRUCTX], [10 x %struct.MYSTRUCTX]* %my1, i32 0, i32 0
  %my3 = getelementptr inbounds %struct.MYSTRUCTX, %struct.MYSTRUCTX* %my2, i32 0, i32 1
  %t1 = load i32, i32* %my3, align 4
  ret i32 %t1
}

define dso_local i32 @main() {
  %call = call i32 @target1([10 x %struct.MYSTRUCT]* @myarg)
  ret i32 %call
}
