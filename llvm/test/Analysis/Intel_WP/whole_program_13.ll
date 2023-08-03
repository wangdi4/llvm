; REQUIRES: asserts
; RUN: llvm-as < %s > %t1
; RUN: llvm-lto -exported-symbol=main -debug-only=whole-program-analysis -whole-program-trace-libfuncs -o %t2 %t1 2>&1 | FileCheck %s

; Check that @for_set_reentrancy is not recognized as a libFunc because it is a
; Fortran libFunc called from a non-Fortran main

; CHECK:  TOTAL LIBFUNCS: 1
; CHECK:  LIBFUNCS FOUND: 0
; CHECK:  LIBFUNCS NOT FOUND: 1
; CHECK:    for_set_reentrancy
; CHECK: WHOLE PROGRAM SEEN:  NOT DETECTED

@hello_ = common unnamed_addr global [4 x i8] zeroinitializer, align 32
@0 = internal unnamed_addr constant i32 2

declare i32 @for_set_reentrancy(ptr) #1

define void @main() #0 {
alloca_0:
  %func_result = call i32 @for_set_reentrancy(ptr @0)
  store i32 5, ptr @hello_, align 1
  ret void
}

attributes #0 = { nounwind uwtable }
attributes #1 = { nounwind uwtable "intel-lang"="fortran" }


