; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that an updated field does not yield a single value.

%struct.MYSTRUCT = type { i32, float }

@coxglobalstruct = internal dso_local global %struct.MYSTRUCT { i32 6, float 8.000000e+00 }, align 4

define dso_local i32 @main() {
  store i32 6, i32* getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct, i64 0, i32 0), align 4
  %1 = load float, float* getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct, i64 0, i32 1), align 4
  %2 = fadd float %1, 8.000000e+00
  store float %2, float* getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct, i64 0, i32 1), align 4
  %3 = fadd float %2, 6.000000e+00
  %4 = fptosi float %3 to i32
  ret i32 %4
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 6
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value

