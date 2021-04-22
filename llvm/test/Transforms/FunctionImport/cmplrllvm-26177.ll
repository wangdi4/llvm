; RUN: opt -module-summary %s -o %t1.o
; RUN: opt -module-summary %S/Inputs/cmplrllvm-26177b.ll -o %t2.o
; RUN: llvm-lto2 run %t1.o %t2.o -r %t1.o,myifunc,p -r %t2.o,myifunc,x -r %t1.o,myfunc1,p -r %t2.o,main,p -save-temps -o %t3
; RUN: llvm-dis -o - %t3.0.2.internalize.bc | FileCheck %s

; CMPLRLLVM-26177: Ensure that the definition of myfunc1 is not improperly
; eliminated because GlobalIFuncs are incompletely handled in the Bitcode
; Reader and Writer.

; CHECK: define{{.*}}myfunc1

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define internal i32 (...)* @mydispatch() {
entry:
  ret i32 (...)* bitcast (i32 ()* @myfunc1 to i32 (...)*)
}

@myifunc = dso_local ifunc i32 (...), bitcast (i32 (...)* ()* @mydispatch to i32 (...)*)

define dso_local i32 @myfunc1() {
entry:
  ret i32 5
}

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"ThinLTO", i32 0}
!2 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
