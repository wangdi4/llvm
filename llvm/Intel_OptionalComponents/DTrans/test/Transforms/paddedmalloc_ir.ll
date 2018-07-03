; Test that identifies if the DTrans padded malloc optimization built the
; global counter and the interface corectly, and modified the malloc
; function successfully.

; RUN: opt < %s -dtrans-paddedmalloc -S 2>&1 | FileCheck %s

%struct.testStruct = type { i8* }

@globalstruct = internal global %struct.testStruct zeroinitializer, align 8
@arr1 = internal global [10 x i32] zeroinitializer, align 16
@arr2 = internal global [10 x i32] zeroinitializer, align 16

declare noalias i8* @malloc(i64)

declare void @free(i8* nocapture)

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
  tail call void @free(i8* %1)
  store i8* null, i8** getelementptr inbounds (%struct.testStruct,
    %struct.testStruct* @globalstruct, i64 0, i32 0), align 8
  call zeroext i1 @searchloop()
  ret i32 0
}

; Verify that the counter was set correctly
; CHECK: @PaddedMallocCounter = internal global i32 0

; Verify that the malloc function was modified correctly
; CHECK: define internal noalias i8* @mallocFunc(i64) {
; CHECK:  %2 = load i32, i32* @PaddedMallocCounter
; CHECK:  %3 = icmp ult i32 %2, 250
; CHECK:  br i1 %3, label %BBif, label %BBelse
;
; CHECK: BBif:
; CHECK:   %4 = add i64 %0, 32
; CHECK:   %5 = tail call noalias i8* @malloc(i64 %4)
; CHECK:   %6 = add i32 1, %2
; CHECK:   store i32 %6, i32* @PaddedMallocCounter
; CHECK:   br label %8
;
; CHECK: BBelse:
; CHECK:   %7 = tail call noalias i8* @malloc(i64 %0)
; CHECK:   br label %8
;
; CHECK: ; <label>:8:
; CHECK:  %9 = phi i8* [ %5, %BBif ], [ %7, %BBelse ]
; CHECK:  ret i8* %9
; CHECK:}

; Verify that the interface was created correctly
; CHECK: define i1 @PaddedMallocInterface() {
; CHECK: entry:
; CHECK:   %0 = load i32, i32* @PaddedMallocCounter
; CHECK:   %1 = icmp ult i32 %0, 250
; CHECK:   ret i1 %1
; CHECK: }
