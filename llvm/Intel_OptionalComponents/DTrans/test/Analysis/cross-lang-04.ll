; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -intel-libirc-allowed  -dtransanalysis -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -intel-libirc-allowed  -passes='require<dtransanalysis>' -dtrans-print-types -dtrans-usecrulecompat -disable-output < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test two structs with different numbers of elements.
; Check that AddressTaken is NOT set on MYSTRUCT and MYSTRUCTX as the C-rule
; applies, since there is no Fortran function.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, i32 }
; CHECK-NOT: Safety data:{{.*}}Address taken{{.*}}
; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCTX = type { i32, i32, i32 }
; CHECK-NOT: Safety data:{{.*}}Address taken{{.*}}

%struct.MYSTRUCT = type { i32, i32 }
%struct.MYSTRUCTX = type { i32, i32, i32 }

@myarg0 = internal dso_local global %struct.MYSTRUCT { i32 3, i32 5 }, align 4
@myarg1 = internal dso_local global %struct.MYSTRUCTX { i32 3, i32 5, i32 7 }, align 4
@fp = internal dso_local global i32 (%struct.MYSTRUCT*)* null, align 8
@fpp = internal dso_local global i32 (%struct.MYSTRUCTX*)* null, align 8

define dso_local i32 @target1(%struct.MYSTRUCT* %arg) {
  %my1 = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %arg, i32 0, i32 0
  %t1 = load i32, i32* %my1, align 4
  ret i32 %t1
}

define dso_local i32 @main() {
  %t0 = load i32 (%struct.MYSTRUCT*)*, i32 (%struct.MYSTRUCT*)** @fp, align 8
  %call0 = call i32 %t0(%struct.MYSTRUCT* @myarg0)
  %t1 = load i32 (%struct.MYSTRUCTX*)*, i32 (%struct.MYSTRUCTX*)** @fpp, align 8
  %call1 = call i32 %t1(%struct.MYSTRUCTX* @myarg1)
  %call2 = add nsw i32 %call0, %call1
  ret i32 %call2
}
