; REQUIRES: asserts
; Test that checks the output of the libfuncs trace by itself (-whole-program-trace-libfuncs).

; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=main -debug-only=whole-program-analysis  -whole-program-trace-libfuncs -o %t2 %t1 2>&1 | FileCheck %s

; CHECK:     WHOLE-PROGRAM-ANALYSIS: LIBRARY FUNCTIONS TRACE
; CHECK:      TOTAL LIBFUNCS: 2
; CHECK:      LIBFUNCS FOUND: 1
; CHECK-NEXT:       malloc
; CHECK-NEXT: LIBFUNCS NOT FOUND: 1
; CHECK-NEXT:       dealloc

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

; Function Attrs: nounwind
declare void @free(ptr)

; Function Attrs: nounwind uwtable
define i32 @main()  {
entry:
  %ptr1 = alloca ptr, align 8
  %call = call ptr @allocate()
  store ptr %call, ptr %ptr1, align 8
  %0 = load ptr, ptr %ptr1, align 8
  call void @dealloc(ptr %0)
  ret i32 0
}

declare void @dealloc(ptr)
