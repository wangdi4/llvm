; REQUIRES: asserts
; This test expects LTO to NOT detect whole program since IR
; is not available for 'dealloc' user defined routine. LTO doesn't
; expect IR for library routines like malloc, free, fprintf.


; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=main -debug-only=whole-program-analysis -o %t2 %t1  2>&1 | FileCheck %s

; CHECK:   UNRESOLVED CALLSITES: 1
; CHECK:   LIBFUNCS NOT FOUND: 1
; CHECK:       dealloc
; CHECK:  WHOLE PROGRAM RESULT:
; CHECK:    MAIN DEFINITION:  DETECTED
; CHECK:    LINKING AN EXECUTABLE:  NOT DETECTED
; CHECK:    WHOLE PROGRAM READ:  NOT DETECTED
; CHECK:    WHOLE PROGRAM SEEN:  NOT DETECTED
; CHECK:    WHOLE PROGRAM SAFE:  NOT DETECTED

%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type { ptr, ptr, i32 }

@stderr = external global ptr, align 8
@.str = private unnamed_addr constant [10 x i8] c"ptr: %d \0A\00", align 1

; Function Attrs: nounwind uwtable
define ptr @allocate() {
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

declare void @dealloc(ptr)

