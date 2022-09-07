; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test that address taken function @target1, which is external, does
; generate an AddressTaken safety check on MYSTRUCT, because it is a varargs
; function and an indirect call matches it.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32 }
; CHECK: Safety data:{{.*}}Address taken{{.*}}

%struct.MYSTRUCT = type { i32, i32 }

@myarg = internal dso_local global %struct.MYSTRUCT { i32 3, i32 5 }, align 4
@fp1 = internal dso_local global i32 (%struct.MYSTRUCT*, ...)* @target1, align 8

declare dso_local i32 @target1(%struct.MYSTRUCT* %arg, ...)

define dso_local i32 @main() #0 {
  %t0 = load i32 (%struct.MYSTRUCT*, ...)*, i32 (%struct.MYSTRUCT*, ...)** @fp1, align 8
  %call = call i32 (%struct.MYSTRUCT*, ...) %t0(%struct.MYSTRUCT* @myarg, i32 7, i32 9)
  ret i32 %call
}
