; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Test that identifies that the DTrans padded malloc optimization didn't
; apply the optimization since it didn't find a malloc function, even
; though a search loop is available.

; RUN: opt -whole-program-assume < %s -dtrans-paddedmalloc -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume < %s -passes=dtrans-paddedmalloc -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s

%struct.testStruct = type { i8* }

@globalstruct = internal global %struct.testStruct zeroinitializer, align 8
@arr1 = internal global [10 x i32] zeroinitializer, align 16
@arr2 = internal global [10 x i32] zeroinitializer, align 16

declare noalias i8* @malloc(i64)

declare void @free(i8* nocapture)

; Malloc function
define internal noalias i8* @mallocFunc(i64 %size, i8* (i64)* %func) {
 %tobool = icmp ne i8* (i64)* %func, null
  br i1 %tobool, label %if.then, label %if.else

if.then:
  %call = call i8* %func(i64 %size)
  br label %if.end

if.else:
  %call1 = call noalias i8* @malloc(i64 %size) #2
  br label %if.end

if.end:
  %ptr.0 = phi i8* [ %call, %if.then ], [ %call1, %if.else ]
  ret i8* %ptr.0
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
  %1 = tail call noalias i8* @mallocFunc(i64 100, i8* (i64)* null)
  store i8* %1, i8** getelementptr inbounds (%struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0), align 8
  tail call void @free(i8* %1)
  store i8* null, i8** getelementptr inbounds (%struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0), align 8
  call zeroext i1 @searchloop()
  ret i32 0
}

; CHECK: dtrans-paddedmalloc: Trace for DTrans Padded Malloc
; CHECK: dtrans-paddedmalloc: Identifying alloc functions
; CHECK: No alloc functions found
