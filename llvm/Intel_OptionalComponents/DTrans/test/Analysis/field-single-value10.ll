; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that a structure assigned from calloc is treated like a whole structure
; assign with all zeroes, yielding Multiple Value when the fields are then
; assigned something other than zero.

%struct.MYOUTERSTRUCT = type { i32, float, %struct.MYINNERSTRUCT }
%struct.MYINNERSTRUCT = type { i32, float }

@coxglobalstruct = internal dso_local global %struct.MYOUTERSTRUCT* null, align 8

define dso_local i32 @main() {
  %1 = tail call noalias i8* @calloc(i64 1, i64 16) #2
  store i8* %1, i8** bitcast (%struct.MYOUTERSTRUCT** @coxglobalstruct to i8**), align 8
  %2 = bitcast i8* %1 to i32*
  store i32 1, i32* %2, align 4
  %3 = getelementptr inbounds i8, i8* %1, i64 4
  %4 = bitcast i8* %3 to float*
  store float 2.000000e+00, float* %4, align 4
  %5 = getelementptr inbounds i8, i8* %1, i64 12
  %6 = bitcast i8* %5 to float*
  store float 4.000000e+00, float* %6, align 4
  ret i32 7
}

declare noalias i8* @calloc(i64, i64) local_unnamed_addr #1

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYINNERSTRUCT = type { i32, float }
; CHECK: Field LLVM Type: i32
; CHECK: Single Value: i32 0
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYOUTERSTRUCT = type { i32, float, %struct.MYINNERSTRUCT }
; CHECK: Field LLVM Type: i32
; CHECK: Multiple Value
; CHECK: Field LLVM Type: float
; CHECK: Multiple Value
; CHECK: Field LLVM Type: %struct.MYINNERSTRUCT = type { i32, float }
; CHECK: Multiple Value


