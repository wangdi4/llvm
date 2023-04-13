; RUN: opt -passes="loop(loop-rotate)" %s -S | FileCheck %s
; CHECK-NOT: invoke{{.*}}baz
; CHECK-NOT: invoke{{.*}}pluto
; CHECK-NOT: invoke{{.*}}snork
; CHECK: invoke{{.*}}wibble

; The IR corresponds very roughly to this C++ code.
; The catch has 5 incoming edges:
; The calls to x, y, assert, print, a.
; These may throw exceptions that reach the catch.
; The problem can be seen if we notice which stmts are inside and outside
; the loop.
; Any stmt that is post-dominated by abort(), is outside the loop.
; The catch is outside the loop, and it has edges from both inside and
; outside the loop (edge from print() is outside).
; This makes the loop unrotatable. We can fix it by deleting the edges from
; inside the loop, as they are illegal.

; void foo() {
; #pragma omp parallel for
;  for (int i = 0; i < 100; i++) {
;    try {
;      x();
;      y();
;      if (assert(0)) {
;        // OUTSIDE THE LOOP
;        print(); abort();
;      }
;      a();
;    }
;    // OUTSIDE THE LOOP
;    catch (...) { abort(); }
;  }
;}


target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.28.29334"



%struct.ham = type { i8**, i8*, [20 x i8] }
@global = external dso_local global %struct.ham
declare dso_local i32 @__CxxFrameHandler3(...)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

define dso_local void @foo(%struct.ham* %arg) #1 align 2 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
; this just needs to be enough for loop rotation to recognize it
  %tmp = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 undef), "QUAL.OMP.SCHEDULE.STATIC"(i32 0) ]
  br label %bb2

bb2:                                              ; preds = %bb11, %bb1
  invoke void @baz()
          to label %bb3 unwind label %bb12

bb3:                                              ; preds = %bb2
  invoke void @pluto()
          to label %bb5 unwind label %bb12

bb5:
  br i1 undef, label %bb6, label %bb11

; THIS IS OUTSIDE THE LOOP
bb6:                                              ; preds = %bb5
  invoke void @wibble() #2
          to label %bb7 unwind label %bb12

bb7:                                              ; preds = %bb6
  unreachable

bb11:                                             ; preds = %bb10
  invoke void @snork()
          to label %bb2 unwind label %bb12

; THIS IS OUTSIDE THE LOOP
bb12:                                             ; preds = %bb11, %bb6, %bb4, %bb3, %bb2
  %tmp13 = catchswitch within none [label %bb14] unwind to caller

bb14:                                             ; preds = %bb12
  %tmp15 = catchpad within %tmp13 [%struct.ham* @global, i32 8, %struct.ham** undef]
  unreachable
}

declare hidden void @zot() #1

declare dso_local void @baz() #1 align 2

declare dso_local void @wibble() #1

declare dso_local void @pluto() #1 align 2

declare dso_local void @spam() #1 align 2

declare hidden void @wibble.5() #1

declare dso_local void @snork() #1 align 2

attributes #0 = { nounwind }
attributes #1 = { "unsafe-fp-math"="true" }
attributes #2 = { noreturn }

