; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that union yields "Multiple Value"

%union.MYINNERSTRUCT = type { i32 }

@coxglobalstruct = internal dso_local global %union.MYINNERSTRUCT { i32 8 }, align 4

define dso_local i32 @main() {
  store float 9.000000e+00, float* bitcast (%union.MYINNERSTRUCT* @coxglobalstruct to float*), align 4
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %union.MYINNERSTRUCT = type { i32 }
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value

