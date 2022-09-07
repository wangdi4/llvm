; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check union nested within an outer struct.
; Expecting Mutiple Value on the union member, as well as the outer struct.

%struct.MYOUTERSTRUCT = type { i32, float, %union.MYINNERUNION }
%union.MYINNERUNION = type { i32 }

@coxglobalstruct = internal dso_local global %struct.MYOUTERSTRUCT { i32 1, float 2.000000e+00, %union.MYINNERUNION { i32 3 } }, align 4

define i32 @main() {
  store i32 1, i32* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 0), align 4
  store float 2.000000e+00, float* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 1), align 4
  store float 4.000000e+00, float* bitcast (%union.MYINNERUNION* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 2) to float*), align 4
  %1 = load float, float* bitcast (%union.MYINNERUNION* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 2) to float*), align 4
  %2 = load i32, i32* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 0), align 4
  %conv = sitofp i32 %2 to float
  %add = fadd float %1, %conv
  %3 = load float, float* getelementptr inbounds (%struct.MYOUTERSTRUCT, %struct.MYOUTERSTRUCT* @coxglobalstruct, i32 0, i32 1), align 4
  %add1 = fadd float %add, %3
  %conv2 = fptosi float %add1 to i32
  ret i32 %conv2
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYOUTERSTRUCT = type { i32, float, %union.MYINNERUNION }
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value
; CHECK: Field LLVM Type: %union.MYINNERUNION = type { i32 }
; CHECK: Multiple Value

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %union.MYINNERUNION = type { i32 }
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value


