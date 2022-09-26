; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Check that a non-nullptr initializer invalidates the single alloc function
; designation for a field.

%struct.MYSTRUCT = type { i8* }

@globalstruct1 = internal global %struct.MYSTRUCT zeroinitializer, align 8

@globalstruct2 = internal global %struct.MYSTRUCT { i8* bitcast (%struct.MYSTRUCT* @globalstruct1 to i8*) }, align 8

declare noalias i8* @malloc(i64)

declare void @free(i8* nocapture)

define i32 @main() {
  %1 = tail call noalias i8* @malloc(i64 200)
  store i8* %1, i8** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct2, i64 0, i32 0), align 8
  tail call void @free(i8* %1)
  store i8* null, i8** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct2, i64 0, i32 0), align 8
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i8* }
; CHECK: Number of fields: 1
; CHECK: Field LLVM Type: i8*
; CHECK: Field info: Written
; CHECK: Multiple Value
; CHECK: Bottom Alloc Function
; CHECK: Safety data: Bad casting | Global instance | Has initializer list
