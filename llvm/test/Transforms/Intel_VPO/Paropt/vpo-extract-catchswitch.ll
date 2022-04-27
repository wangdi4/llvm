; RUN: opt -vpo-paropt -S %s | FileCheck %s

; INTEL_CUSTOMIZATION
; CMPLRLLVM-28173:
; end INTEL_CUSTOMIZATION
;
; blam(); // unwind to X;
; #pragma omp parallel for
; for (int tmp28 = 0; tmp28 < ub; tmp28++) {
;   try {
;     wombat();
;     pluto();
;   }
;   catch (...) { // unwind to X;
;     abort();
;   }
; }
; blam(); // unwind to X;
; return;
;
; // common unwind termination handler for this function
; X:
;   abort();
;
; The unwind handler has predecessors that are inside and outside the region.
; We want to break the edge coming from the catch, as it is undefined.
; The catch actually requires some unwind handler, so we cannot just delete
; the edge. A new unwind handler must be created for that catch.

; The C++ code is a little simplified, as it does not directly show why
; blam() calls may have a path to a common termination handler, instead of a
; catch handler.
; There are several reasons: blam() is inlined from a destructor, blam() is
; inlined from a nothrow function, blam() is inside a local scope with a live
; object that needs to be destroyed.

; CHECK: catchswitch{{.*}}unwind label %bb100.termpad
; CHECK: bb100.termpad:
; CHECK: cleanuppad
; CHECK: unreachable

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.28.29334"

%struct.widget = type { i8**, i8*, [20 x i8] }
%struct.wombat = type { %struct.widget }

%struct.wombat.142 = type { i32 (...)**, %struct.widget }

@global = external global %struct.widget

declare dso_local i32 @__CxxFrameHandler3(...)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

define dso_local void @quux() #1 align 2 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) {
bb:
  %tmp = alloca i64, align 8
  %tmp1 = alloca i64, align 8
  br label %bb14

bb14:                                             ; preds = %bb13, %bb11
  invoke void @blam()
          to label %bb22 unwind label %bb100

bb22:                                             ; preds = %bb21
  %tmp23 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 0), "QUAL.OMP.NUM_THREADS"(i64 undef), "QUAL.OMP.NORMALIZED.IV"(i64* undef), "QUAL.OMP.NORMALIZED.UB"(i64* %tmp) ]
  br label %bb27

bb27:                                             ; preds = %bb39, %bb26
  %tmp28 = load volatile i64, i64* undef, align 8
  %tmp29 = icmp sle i64 %tmp28, undef
  br i1 %tmp29, label %bb30, label %bb42

bb30:                                             ; preds = %bb27
  invoke void @wombat()
          to label %bb31 unwind label %bb32

bb31:                                             ; preds = %bb30
  invoke void @pluto()
          to label %bb36 unwind label %bb32

bb32:                                             ; preds = %bb31, %bb30
  %tmp33 = catchswitch within none [label %bb34] unwind label %bb100

bb34:                                             ; preds = %bb32
  %tmp35 = catchpad within %tmp33 [%struct.widget* @global, i32 8, %struct.wombat.142** undef]
  unreachable

bb36:                                             ; preds = %bb31
  br label %bb39


bb39:                                             ; preds = %bb38
  %tmp40 = load volatile i64, i64* undef, align 8
  %tmp41 = add nsw i64 %tmp40, 1
  store volatile i64 %tmp41, i64* undef, align 8
  br label %bb27

bb42:                                             ; preds = %bb27
  br label %bb45

bb45:                                             ; preds = %bb44
  call void @llvm.directive.region.exit(token %tmp23) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %bb61

; OUTSIDE THE REGION
bb61:                                             ; preds = %bb55
  invoke void @blam()
          to label %bb97 unwind label %bb98

bb97:                                             ; preds = %bb96, %bb3
  ret void

; cleanup for blam() call
bb98:                                             ; preds = %bb61
  %tmp99 = cleanuppad within none []
  cleanupret from %tmp99 unwind label %bb100

; Common unwind handler.
; This block is inside the region, because it is reachable from the region
; entry without passing through the region exit.
; bb98 and bb14 are the unwinds from blam() which is outside the region.
; bb32 is the unwind from the catchswitch inside the region.
; We want to break the edge bb32=>bb100.

bb100:                                            ; preds = %bb98, %bb32, %bb14
  %tmp101 = cleanuppad within none []
  unreachable
}

declare dso_local void @wombat() #1 align 2

declare dso_local void @blam() #1 align 2

declare dso_local void @pluto() #1

attributes #0 = { nounwind }
attributes #1 = { "unsafe-fp-math"="true" }

!llvm.ident = !{!0}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
