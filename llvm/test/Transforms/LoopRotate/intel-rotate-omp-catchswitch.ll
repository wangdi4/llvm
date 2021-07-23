; RUN: opt -passes="loop-rotate" -S %s | FileCheck %s
; When compiled for Windows, the catch becomes a catchswitch, with an
; edge to the catch handler and an edge to a cleanup handler outside the
; loop.
; The cleanup handler is also used to clean up the "throw" inside the catch
; handler.
; Because the cleanup handler is a loop exit with 1 pred inside the loop
; and 1 pred outside the loop, loop rotation must split it.
; But, because catchswitches and catch handlers cannot have different unwind
; blocks, this is impossible.
; It can be solved by deleting all unwind edges from the catch body, as these
; are undefined exits.
; The unwind block will then only have 1 pred, from inside the loop, and
; rotation can continue.
;
;extern int g;
;int foo()
;{
;  #pragma omp parallel for
;  for (int j = 0; j < 100; j++) {
;    try {
;      if (g) throw 5;
;    }
;    // Catchswitch is inside the loop. It has 2 successors: the catch handler
;    // and the cleanup handler below.
;    catch (float f) { // unwind to UNWIND:
;      if (f > 0) {
;        // This throw unwinds to the cleanup handler. The throw is outside
;        // the loop.
;        throw 4; // unwind to UNWIND:
;      }
;    }
;  }
;  UNWIND:
;    // Hidden cleanup handler here.
;    // It has 2 predecessors: the catchswitch and the catch handler's throw.
;
;  return 0;
;}

; "loopexit" labels are caused by loop rotation and splitting.
; CHECK: catchswitch within none{{.*}}unwind label{{.*}}loopexit

; Check that the unwind edge from "invoke @spam" was broken.
; CHECK: call void @spam

; ModuleID = 'bp.ll'
source_filename = "cq139624.cpp"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.16.27045"

%struct.hoge = type { i32*, [4 x i8], i32, i32 }
@global = external dso_local global %struct.hoge, align 8
@global.1 = external dso_local unnamed_addr constant [1 x i8], align 1

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...) #0

declare dso_local void @widget() unnamed_addr #1

define dso_local void @main() local_unnamed_addr #1 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
bb4:
  br label %bb5

bb5:                                              ; preds = %bb4
  %tmp6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SHARED"(%struct.hoge* undef), "QUAL.OMP.SHARED"(i32* undef), "QUAL.OMP.NORMALIZED.IV"(i32* undef), "QUAL.OMP.FIRSTPRIVATE"(i32* undef), "QUAL.OMP.NORMALIZED.UB"(i32* undef), "QUAL.OMP.PRIVATE"(i32* undef), "QUAL.OMP.SHARED"(%struct.hoge* @global), "QUAL.OMP.JUMP.TO.END.IF"(i1* undef) ]
  br label %bb17

bb17:                                             ; preds = %bb16
  invoke void undef()
          to label %bb22 unwind label %bb25

bb18:                                             ; preds = %bb16, %bb13
  %tmp19 = cleanuppad within none []
  unreachable

bb22:                                             ; preds = %bb17
  unreachable

bb25:                                             ; preds = %bb17
  %tmp26 = catchswitch within none [label %bb27] unwind label %bb33

bb27:                                             ; preds = %bb25
  %tmp28 = catchpad within %tmp26 [i8* null, i32 64, i8* null]
  br i1 undef, label %bb31, label %bb29

bb29:                                             ; preds = %bb27
  invoke void @spam() #3 [ "funclet"(token %tmp28) ]
          to label %bb30 unwind label %bb33

bb30:                                             ; preds = %bb29
  unreachable

bb31:                                             ; preds = %bb27
  catchret from %tmp28 to label %bb43

bb33:                                             ; preds = %bb29, %bb25
  %tmp34 = cleanuppad within none []
  cleanupret from %tmp34 unwind to caller

bb43:                                             ; preds = %bb39
  br label %bb17
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nofree
declare dso_local void @spam() local_unnamed_addr #0

attributes #0 = { nofree }
attributes #1 = { "unsafe-fp-math"="true" }
attributes #2 = { nounwind }
attributes #3 = { noreturn }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
