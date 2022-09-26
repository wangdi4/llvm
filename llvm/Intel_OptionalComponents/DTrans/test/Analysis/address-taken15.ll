; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test address taken function @target1, which is external. Since a pointer
; to MYSTRUCT is passed as an argument, MYSTRUCT should be marked as
; Address taken.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32 }
; CHECK: Safety data:{{.*}}Address taken{{.*}}

%struct.MYSTRUCT = type { i32, i32 }
%struct.MYSTRUCTX = type { i32, i32 }

@myarg = internal dso_local global %struct.MYSTRUCT { i32 3, i32 5 }, align 4
@fp = internal dso_local global i32 (%struct.MYSTRUCT*)* @target1, align 8

declare dso_local i32 @target1(%struct.MYSTRUCT* %arg)

define dso_local i32 @main() #0 {
  %t0 = load i32 (%struct.MYSTRUCT*)*, i32 (%struct.MYSTRUCT*)** @fp, align 8
  %call = call i32 %t0(%struct.MYSTRUCT* @myarg)
  ret i32 %call
}
