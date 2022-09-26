; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; Test that identifies that the DTrans padded malloc optimization
; didn't find a malloc function.

; RUN: opt -whole-program-assume < %s -dtrans-paddedmalloc -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume < %s -passes=dtrans-paddedmalloc -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s

%struct.testStruct = type { i8* }

@globalstruct = internal global %struct.testStruct zeroinitializer, align 8

declare noalias i8* @malloc(i64)

declare void @free(i8* nocapture)

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

define i32 @main() {
  %1 = tail call noalias i8* @mallocFunc(i64 100, i8* (i64)* null)
  store i8* %1, i8** getelementptr inbounds (%struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0), align 8
  tail call void @free(i8* %1)
  store i8* null, i8** getelementptr inbounds (%struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0), align 8
  ret i32 0
}

; CHECK: dtrans-paddedmalloc: Trace for DTrans Padded Malloc
; CHECK: dtrans-paddedmalloc: Identifying alloc functions
; CHECK: No alloc functions found
