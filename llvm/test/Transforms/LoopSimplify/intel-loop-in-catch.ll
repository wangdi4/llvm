; OMP PARALLEL {
;   ...
;   catch (int x) { // unwind to cleanup
;     loop {
;       foo(); // unwind to cleanup
;     }
;   }
; }
; cleanup:
;
; loop-simplify tries to split the exit edge between foo() and cleanup,
; but it cannot be done easily, as the catch and its child calls such as
; foo() must have the same handler.

; RUN: opt -S -passes="loop-simplify" %s | FileCheck %s
; CHECK: bb13:
; CHECK-NEXT: invoke{{.*}}barney.8
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27045"

%struct.foo = type { i8**, i8*, [3 x i8] }
%struct.pluto = type { i8 }
%struct.wombat = type { i8 }

@global = external dso_local global double, align 8
@global.1 = external dso_local global double, align 8
@global.2 = external dso_local global double, align 8
@global.3 = external dso_local global double, align 8
@global.4 = external dso_local global i32, align 4
@global.5 = external dso_local global i32, align 4
@global.6 = external global %struct.foo
@global.7 = external global %struct.foo

define dso_local void @barney() local_unnamed_addr #0 align 2 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
bb:
  br label %bb1

bb1:                                              ; preds = %bb
  %tmp = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.MAX"(double* undef), "QUAL.OMP.SHARED"(double* @global), "QUAL.OMP.SHARED"(double* @global.1), "QUAL.OMP.SHARED"(double* @global.2), "QUAL.OMP.SHARED"(double* @global.3), "QUAL.OMP.SHARED"(i32* @global.4), "QUAL.OMP.SHARED"(i32* @global.5), "QUAL.OMP.PRIVATE"(%struct.pluto* undef), "QUAL.OMP.PRIVATE"(%struct.wombat* undef), "QUAL.OMP.PRIVATE"(%struct.pluto* undef), "QUAL.OMP.PRIVATE"(%struct.wombat* undef), "QUAL.OMP.JUMP.TO.END.IF"(i1* undef) ]
  invoke void @hoge()
          to label %bb2 unwind label %bb9

bb2:
  invoke void @widget()
          to label %bb8 unwind label %bb14

bb8:
  ret void

bb9:
  %tmp10 = catchswitch within none [label %bb11] unwind label %bb14

bb11:
  %tmp12 = catchpad within %tmp10 [%struct.foo* @global.6, i32 0, i8* null]
  br label %bb13

; LOOP (single-block). loop-simplify tries to break the %bb13-%bb14 edge from
; the other preds.
bb13:                                             ; preds = %bb13, %bb11
  invoke void @barney.8() [ "funclet"(token %tmp12) ]
          to label %bb13 unwind label %bb14

bb14:                                           ; preds = %bb2, %bb9, %bb13
  %tmp15 = cleanuppad within none []
  cleanupret from %tmp15 unwind to caller
}

declare dso_local void @widget() unnamed_addr #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...) #2

declare dso_local void @hoge() unnamed_addr #0

declare dso_local void @barney.8() local_unnamed_addr #0

attributes #0 = { "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nofree }

