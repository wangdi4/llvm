; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that rewritten structure field is Multiple Value

%struct.MYINNERSTRUCT = type { i32, float }

@coxglobalstruct = internal dso_local global %struct.MYINNERSTRUCT { i32 6, float 8.000000e+00 }, align 4

define dso_local i32 @main() #0 {
  store i32 6, i32* getelementptr inbounds (%struct.MYINNERSTRUCT, %struct.MYINNERSTRUCT* @coxglobalstruct, i64 0, i32 0), align 4
  store float 9.000000e+00, float* getelementptr inbounds (%struct.MYINNERSTRUCT, %struct.MYINNERSTRUCT* @coxglobalstruct, i64 0, i32 1), align 4
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYINNERSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 6
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value

