; REQUIRES: asserts
; Test for checking that libfuncs aren't repeated in the trace.

; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=main -debug-only=whole-program-analysis -whole-program-trace-libfuncs -o %t2 %t1 2>&1 | FileCheck %s

; CHECK:     WHOLE-PROGRAM-ANALYSIS: LIBRARY FUNCTIONS TRACE
; CHECK:      TOTAL LIBFUNCS: 2
; CHECK:      LIBFUNCS FOUND: 2
; CHECK-NEXT:       malloc
; CHECK-NEXT:       free
; CHECK-NEXT: LIBFUNCS NOT FOUND: 0

; Function Attrs: nounwind uwtable
define ptr @allocate()  {
entry:
  %call = call noalias ptr @malloc(i64 8)
  ret ptr %call
}

; Function Attrs: nounwind
declare noalias ptr @malloc(i64)

; Function Attrs: nounwind uwtable
define void @dealloc(ptr %ptr)  {
entry:
  %ptr.addr = alloca ptr, align 8
  store ptr %ptr, ptr %ptr.addr, align 8
  %0 = load ptr, ptr %ptr.addr, align 8
  call void @free(ptr %0)
  ret void
}

; Function Attrs: nounwind uwtable
define ptr @allocate2()  {
entry:
  %call = call noalias ptr @malloc(i64 8)
  ret ptr %call
}

; Function Attrs: nounwind uwtable
define void @dealloc2(ptr %ptr)  {
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
  call void @dealloc(ptr %0)

  %ptr2 = alloca ptr, align 8
  %call2 = call ptr @allocate2()
  store ptr %call2, ptr %ptr2, align 8
  %1 = load ptr, ptr %ptr2, align 8
  call void @dealloc2(ptr %1)

  ret i32 0
}
