; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check for all fields being Single Value, except for the embedded union.

%struct.MYOUTERSTRUCT = type { i32, float, %union.MYINNERUNION }
%union.MYINNERUNION = type { i32 }

@coxglobalstruct = internal dso_local global %struct.MYOUTERSTRUCT { i32 1, float 2.000000e+00, %union.MYINNERUNION { i32 3 } }, align 4

define i32 @main() {
  store i32 1, i32* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 0), align 4
  store float 2.000000e+00, float* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 1), align 4
  %1 = load i32, i32* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 0), align 4
  %conv = sitofp i32 %1 to float
  %2 = load float, float* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 1), align 4
  %add = fadd float %conv, %2
  %conv1 = fptosi float %add to i32
  ret i32 %conv1
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYOUTERSTRUCT = type { i32, float, %union.MYINNERUNION }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 1
; CHECK: Field LLVM Type: float
; CHECK: Single Value: float 2.000000e+00
; CHECK: Field LLVM Type: %union.MYINNERUNION = type { i32 }
; CHECK: Multiple Value

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %union.MYINNERUNION = type { i32 }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 3


