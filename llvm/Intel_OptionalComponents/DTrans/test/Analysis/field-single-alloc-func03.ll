; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test that malloc wrapped in another function and assigned to a void*
; field is recognized as a Single Alloc Function

%struct.MYSTRUCT = type { i8* }

@globalstruct = internal global %struct.MYSTRUCT zeroinitializer, align 8

declare noalias i8* @malloc(i64)

declare void @free(i8* nocapture)

define internal noalias i8* @lzma_mymalloc(i64 %size) {
  %1 = tail call noalias i8* @malloc(i64 %size)
  ret i8* %1
}

define internal void @lzma_myfree(i8* nocapture) {
  tail call void @free(i8* %0)
  ret void
}

define i32 @main() {
  %1 = tail call noalias i8* @lzma_mymalloc(i64 100)
  store i8* %1, i8** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct, i64 0, i32 0), align 8
  tail call void @lzma_myfree(i8* %1)
  store i8* null, i8** getelementptr inbounds (%struct.MYSTRUCT, %struct.MYSTRUCT* @globalstruct, i64 0, i32 0), align 8
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.MYSTRUCT = type { i8* }
; CHECK: Number of fields: 1
; CHECK: Field LLVM Type: i8*
; CHECK: Field info: Written
; CHECK: Multiple Value
; CHECK: Single Alloc Function: i8* (i64)* @lzma_mymalloc
; CHECK: Safety data: Global instance
