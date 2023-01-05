; RUN: opt -passes="loop(loop-rotate)" %s -S | FileCheck %s

; CMPLRLLVM-32873
; This case has a loop with an exit through an invoke+unwind to bb14.
; The exit block bb14 has a non-loop pred and the exit must be split.
; The code that we added to handle cleanuppad exits, is assuming that all of
; the cleanuppad predecessors are in the dominator tree. In this case,
; the other pred bbunreachable is not in the domtree, as it is unreachable.
; We don't need this loop rotated, just prevent the crash.

; CHECK-LABEL: bb2
; CHECK: unwind label %bb14

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

; LOOP
bb1:                                              ; preds = %bb
; this just needs to be enough for loop rotation to recognize it
  %tmp = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.NUM_THREADS"(i32 undef), "QUAL.OMP.SCHEDULE.STATIC"(i32 0) ]
  br label %bb2

bb2:                                              ; preds = %bb11, %bb1
  invoke void @baz()
          to label %bb5 unwind label %bb14

bb5:
  br i1 undef, label %bb6, label %bb1
; LOOP END

bb6:
  ret void

bb12:                                             ; preds = %bb11, %bb6, %bb4, %bb3, %bb2, %bbunreachable
  %tmp13 = catchswitch within none [label %bbcatch] unwind to caller

bbunreachable:
  %foo = catchswitch within none [label %bbcatch2] unwind label %bb14

bbcatch:                                             ; preds = %bb12
  %tmp15 = catchpad within %tmp13 [%struct.ham* @global, i32 8, %struct.ham** undef]
  unreachable

bbcatch2:                                             ; preds = %bbunreachable
  %tmpxx = catchpad within %foo [%struct.ham* @global, i32 8, %struct.ham** undef]
  unreachable

bb14: ; preds = %bbunreachable, %bb2
  %cl = cleanuppad within none []
  cleanupret from %cl unwind to caller
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

