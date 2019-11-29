; REQUIRES: assert
; This test checks that the globals @__ImageBase, @stderr and @.str aren't
; considered as visible outside LTO since they aren't functions.

; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=main -debug-only=whole-program-analysis -whole-program-trace-visibility -o %t2 %t1 2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: EXTERNAL FUNCTIONS TRACE
; CHECK:   VISIBLE OUTSIDE LTO: 0

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@__ImageBase = external dso_local constant i8

@stderr = external global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [10 x i8] c"ptr: %d \0A\00", align 1

; Function Attrs: nounwind uwtable
define i8* @allocate()  {
entry:
%call = call noalias i8* @malloc(i64 8)
ret i8* %call
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64)

; Function Attrs: nounwind uwtable
define void @assign(i8* %ptr)  {
entry:
%ptr.addr = alloca i8*, align 8
store i8* %ptr, i8** %ptr.addr, align 8
%0 = load i8*, i8** %ptr.addr, align 8
%1 = bitcast i8* %0 to i32*
store i32 10, i32* %1, align 4
ret void
}

; Function Attrs: nounwind uwtable
define void @dump(i8* %ptr)  {
entry:
%ptr.addr = alloca i8*, align 8
store i8* %ptr, i8** %ptr.addr, align 8
%0 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8
%1 = load i8*, i8** %ptr.addr, align 8
%2 = bitcast i8* %1 to i32*
%3 = load i32, i32* %2, align 4
%call = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %0, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i32 0, i32 0), i32 %3)
ret void
}

declare i32 @fprintf(%struct._IO_FILE*, i8*, ...)


; Function Attrs: nounwind uwtable
define void @dealloc(i8* %ptr)  {
entry:
%ptr.addr = alloca i8*, align 8
store i8* %ptr, i8** %ptr.addr, align 8
%0 = load i8*, i8** %ptr.addr, align 8
call void @free(i8* %0)
ret void
}

; Function Attrs: nounwind
declare void @free(i8*)

; Function Attrs: nounwind uwtable
define i32 @main()  {
entry:
%ptr1 = alloca i8*, align 8
%call = call i8* @allocate()
store i8* %call, i8** %ptr1, align 8
%0 = load i8*, i8** %ptr1, align 8
call void @assign(i8* %0)
%1 = load i8*, i8** %ptr1, align 8
call void @dump(i8* %1)
%2 = load i8*, i8** %ptr1, align 8
call void @dealloc(i8* %2)
ret i32 0
}
