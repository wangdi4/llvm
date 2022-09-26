; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test that address taken function @target1, which is external, does not
; generate an AddressTaken safety check on MYSTRUCT, because there is no
; direct call to it, and no indirect call matches it.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32 }
; CHECK-NOT: Safety data:{{.*}}Address taken{{.*}}

%struct.MYSTRUCT = type { i32, i32 }

@myarg = internal dso_local global %struct.MYSTRUCT { i32 3, i32 5 }, align 4
@fp1 = internal dso_local global i32 (%struct.MYSTRUCT*)* @target1, align 8

define dso_local i32 @target2(i32 %arg1, i32 %arg2) {
  %t0 = add nsw i32 %arg1, %arg2
  ret i32 %t0;
}

@fp2 = internal dso_local global i32 (i32, i32)* @target2, align 8

declare dso_local i32 @target1(%struct.MYSTRUCT* %arg)

define dso_local i32 @main() #0 {
  %t0 = load i32 (i32, i32)*, i32 (i32, i32)** @fp2, align 8
  %call = call i32 %t0(i32 5, i32 6)
  ret i32 %call
}
