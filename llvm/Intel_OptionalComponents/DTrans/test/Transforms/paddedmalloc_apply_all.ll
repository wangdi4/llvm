; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

target triple = "x86_64-unknown-linux-gnu"

; Test that identifies if the DTrans padded malloc optimization was applied.
; In order to apply padded malloc, the optimization must find a malloc
; function and a search loop.

; This test case is similar to paddedmalloc_apply.ll, but also explicitly
; exercises dtransanalysis and -padded-pointer-prop.

; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>,padded-pointer-prop,dtrans-paddedmalloc' -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s

%struct.testStruct = type { i8* }

@globalstruct = internal global %struct.testStruct zeroinitializer, align 8
@arr1 = internal global [10 x i32] zeroinitializer, align 16
@arr2 = internal global [10 x i32] zeroinitializer, align 16

declare noalias i8* @malloc(i64) #0

declare void @free(i8* nocapture) #1

; Malloc function
define internal noalias i8* @mallocFunc(i64) {
  %2 = tail call noalias i8* @malloc(i64 %0)
  ret i8* %2
}

; Search loop
define internal zeroext i1 @searchloop() #5 {
  br label %3

; <label>:1:                                      ; preds = %3
  %2 = icmp ult i64 %10, 10
  br i1 %2, label %3, label %11

; <label>:3:                                      ; preds = %1, %0
  %4 = phi i64 [ 0, %0 ], [ %10, %1 ]
  %5 = getelementptr inbounds [10 x i32], [10 x i32]* @arr1, i64 0, i64 %4
  %6 = load i32, i32* %5, align 4
  %7 = getelementptr inbounds [10 x i32], [10 x i32]* @arr2, i64 0, i64 %4
  %8 = load i32, i32* %7, align 4
  %9 = icmp eq i32 %6, %8
  %10 = add nuw nsw i64 %4, 1
  br i1 %9, label %11, label %1

; <label>:11:                                     ; preds = %3, %1
  %12 = phi i1 [ true, %3 ], [ false, %1 ]
  ret i1 %12
}

define i32 @main() {
  %1 = tail call noalias i8* @mallocFunc(i64 100)
  store i8* %1, i8** getelementptr inbounds (%struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0), align 8
  %2 = getelementptr inbounds %struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0
  %3 = load i8*, i8** %2, align 4
  tail call void @free(i8* %1)
  store i8* null, i8** getelementptr inbounds (%struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0), align 8
  call zeroext i1 @searchloop()
  ret i32 0
}

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }


; CHECK: dtrans-paddedmalloc: Trace for DTrans Padded Malloc
; CHECK: dtrans-paddedmalloc: Identifying alloc functions
; CHECK: Alloc Function: mallocFunc
; CHECK: dtrans-paddedmalloc: Identifying search loops
; CHECK: Search loop found in: searchloop
; CHECK: dtrans-paddedmalloc: Global variable: __Intel_PaddedMallocCounter
; CHECK: dtrans-paddedmalloc: Interface function: __Intel_PaddedMallocInterface
; CHECK: dtrans-paddedmalloc: Applying padded malloc
; CHECK: Function updated: mallocFunc

