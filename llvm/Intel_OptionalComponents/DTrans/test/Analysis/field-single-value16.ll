; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check whole structure assign

%struct.MYSTRUCT = type { i32, float }

@coxglobalstruct1 = internal dso_local global %struct.MYSTRUCT {i32 2, float 9.000000e+00}, align 8

@coxglobalstruct2 = internal dso_local global %struct.MYSTRUCT {i32 2, float 9.000000e+00}, align 8

define dso_local i32 @main(%struct.MYSTRUCT %s, %struct.MYSTRUCT* %p) {
  store %struct.MYSTRUCT %s, %struct.MYSTRUCT* %p
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 2
; CHECK: Field LLVM Type: float
; CHECK: float 9.000000e+00

