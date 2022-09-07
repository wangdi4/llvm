; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

%struct.MYSTRUCT = type { i32, float }

@coxglobalstruct = internal dso_local global %struct.MYSTRUCT { i32 6, float 8.000000e+00 }, align 4

define dso_local i32 @main() {
  %coxlocalstruct = alloca %struct.MYSTRUCT, align 4
  %myint = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %coxlocalstruct, i32 0, i32 0
  store i32 6, i32* %myint, align 4
  %myfloat = getelementptr inbounds %struct.MYSTRUCT, %struct.MYSTRUCT* %coxlocalstruct, i32 0, i32 1
  store float 8.000000e+00, float* %myfloat, align 4
  store i32 6, i32* getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct, i32 0, i32 0), align 4
  store float 8.000000e+00, float* getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @coxglobalstruct, i32 0, i32 1), align 4
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 6
; CHECK: Field LLVM Type: float
; CHECK: Single Value: float 8.000000e+00

