; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check a case with a memmove. It should not invalidate Single Value fields.

%struct.MYSTRUCT = type { i32, float }

@coxglobalstruct1 = internal dso_local global %struct.MYSTRUCT { i32 6, float 8.000000e+00 }, align 4
@coxglobalstruct2 = internal dso_local global %struct.MYSTRUCT { i32 6, float 8.000000e+00 }, align 4

define i32 @main() {
  call void @llvm.memmove.p0i8.p0i8.i64(i8* bitcast (%struct.MYSTRUCT* @coxglobalstruct2 to i8*), i8* bitcast (%struct.MYSTRUCT* @coxglobalstruct1 to i8*), i64 8, i32 4, i1 false)
  ret i32 0
}

declare void @llvm.memmove.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1)

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 6
; CHECK: Field LLVM Type: float
; CHECK: Single Value: float 8.000000e+00
