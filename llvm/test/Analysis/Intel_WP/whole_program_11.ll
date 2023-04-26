; REQUIRES: asserts
; This test checks that the globals @__ImageBase, @stderr and @.str aren't
; considered as visible outside LTO since they aren't functions.

; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=main -debug-only=whole-program-analysis -whole-program-trace-visibility -o %t2 %t1 2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: EXTERNAL FUNCTIONS TRACE
; CHECK:   VISIBLE OUTSIDE LTO: 0

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }

@__ImageBase = external dso_local constant i8

@stderr = external global ptr, align 8
@.str = private unnamed_addr constant [10 x i8] c"ptr: %d \0A\00", align 1

; Function Attrs: nounwind uwtable
define ptr @allocate()  {
entry:
%call = call noalias ptr @malloc(i64 8)
ret ptr %call
}

; Function Attrs: nounwind
declare noalias ptr @malloc(i64)

; Function Attrs: nounwind uwtable
define void @assign(ptr %ptr)  {
entry:
%ptr.addr = alloca ptr, align 8
store ptr %ptr, ptr %ptr.addr, align 8
%0 = load ptr, ptr %ptr.addr, align 8
%1 = bitcast ptr %0 to ptr
store i32 10, ptr %1, align 4
ret void
}

; Function Attrs: nounwind uwtable
define void @dump(ptr %ptr)  {
entry:
%ptr.addr = alloca ptr, align 8
store ptr %ptr, ptr %ptr.addr, align 8
%0 = load ptr, ptr @stderr, align 8
%1 = load ptr, ptr %ptr.addr, align 8
%2 = bitcast ptr %1 to ptr
%3 = load i32, ptr %2, align 4
%call = call i32 (ptr, ptr, ...) @fprintf(ptr %0, ptr getelementptr inbounds ([10 x i8], ptr @.str, i32 0, i32 0), i32 %3)
ret void
}

declare i32 @fprintf(ptr, ptr, ...)


; Function Attrs: nounwind uwtable
define void @dealloc(ptr %ptr)  {
entry:
%ptr.addr = alloca ptr, align 8
store ptr %ptr, ptr %ptr.addr, align 8
%0 = load ptr, ptr %ptr.addr, align 8
call void @free(ptr %0)
ret void
}

; Function Attrs: nounwind
declare void @free(ptr)

; Function Attrs: nounwind uwtable
define i32 @main()  {
entry:
%ptr1 = alloca ptr, align 8
%call = call ptr @allocate()
store ptr %call, ptr %ptr1, align 8
%0 = load ptr, ptr %ptr1, align 8
call void @assign(ptr %0)
%1 = load ptr, ptr %ptr1, align 8
call void @dump(ptr %1)
%2 = load ptr, ptr %ptr1, align 8
call void @dealloc(ptr %2)
ret i32 0
}
