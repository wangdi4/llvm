; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Test that identifies if the DTrans padded malloc optimization identified
; a malloc function but there is no search loop.

; RUN: opt -whole-program-assume < %s -dtrans-paddedmalloc -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume < %s -passes=dtrans-paddedmalloc -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s

%struct.testStruct = type { i8* }

@globalstruct = internal global %struct.testStruct zeroinitializer, align 8

declare noalias i8* @malloc(i64)

declare void @free(i8* nocapture)

define internal noalias i8* @mallocFunc(i64) {
  %2 = tail call noalias i8* @malloc(i64 %0)
  ret i8* %2
}

define i32 @main() {
  %1 = tail call noalias i8* @mallocFunc(i64 100)
  store i8* %1, i8** getelementptr inbounds (%struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0), align 8
  tail call void @free(i8* %1)
  store i8* null, i8** getelementptr inbounds (%struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0), align 8
  ret i32 0
}

; CHECK: dtrans-paddedmalloc: Trace for DTrans Padded Malloc
; CHECK: dtrans-paddedmalloc: Identifying alloc functions
; CHECK: Alloc Function: mallocFunc
; CHECK: dtrans-paddedmalloc: Identifying search loops
; CHECK: No search loops found
