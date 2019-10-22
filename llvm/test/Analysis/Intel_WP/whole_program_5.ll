; REQUIRES: assert
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

; Function Attrs: nounwind
declare void @free(i8*)

; Function Attrs: nounwind uwtable
define i32 @main()  {
entry:
  %ptr1 = alloca i8*, align 8
  %call = call i8* @allocate()
  store i8* %call, i8** %ptr1, align 8
  %0 = load i8*, i8** %ptr1, align 8
  call void @dealloc(i8* %0)
  ret i32 0
}

declare void @dealloc(i8*)
