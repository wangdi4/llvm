; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check case with two instances of a global variable of the same structure,
; where one is assigned to the other as a whole structure assign in the
; C/C++ source.

%struct.MYSTRUCT = type { i32, float }

@coxglobalstruct1 = internal dso_local global %struct.MYSTRUCT { i32 6, float 8.000000e+00 }, align 4
@coxglobalstruct2 = internal dso_local global %struct.MYSTRUCT { i32 6, float 8.000000e+00 }, align 4

define dso_local i32 @main() {
  store i32 6, i32* getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct1, i64 0, i32 0), align 4
  store i32 6, i32* getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct2, i64 0, i32 0), align 4
  %1 = load i32, i32* bitcast (float* getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct1, i64 0, i32 1) to i32*), align 4
  store i32 %1, i32* bitcast (float* getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct2, i64 0, i32 1) to i32*), align 4
  %2 = bitcast i32 %1 to float
  %3 = fadd float %2, 6.000000e+00
  %4 = fptosi float %3 to i32
  ret i32 %4
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value

