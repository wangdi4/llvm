; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

; Test two identical structs with different names.
; Check that AddressTaken is set on MYSTRUCT, as MYSTRUCTX is a compatible
; type, so @target1 and @target2 are targets for @fp.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32 }
; CHECK: Safety data:{{.*}}Address taken{{.*}}

%struct.MYSTRUCT = type { i32, i32 }
%struct.MYSTRUCTX = type { i32, i32 }

@myarg = internal dso_local global %struct.MYSTRUCT { i32 3, i32 5 }, align 4
@fp = internal dso_local global i32 (%struct.MYSTRUCT*)* null, align 8

define dso_local i32 @target1(%struct.MYSTRUCT* %arg) {
  %my1 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %arg, i32 0, i32 0
  %t1 = load i32, i32* %my1, align 4
  ret i32 %t1
}

define dso_local i32 @target2(%struct.MYSTRUCTX* %arg) {
entry:
  %my2 = getelementptr inbounds %struct.MYSTRUCTX, %struct.MYSTRUCTX* %arg, i32 0, i32 1
  %t1 = load i32, i32* %my2, align 4
  ret i32 %t1
}

define dso_local i32 @main() #0 {
  %t0 = load i32 (%struct.MYSTRUCT*)*, i32 (%struct.MYSTRUCT*)** @fp, align 8
  %call = call i32 %t0(%struct.MYSTRUCT* @myarg)
  ret i32 %call
}
