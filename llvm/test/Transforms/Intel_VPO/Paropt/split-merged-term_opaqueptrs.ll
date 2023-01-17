; RUN: opt -passes=vpo-paropt -S %s | FileCheck %s

; 2 adjacent OMP regions with catch handlers:
; #pragma omp parallel
; {
;   try {
;     xxx();
;   } catch (...) { // unwind to "terminate"
;   }
; }
; #pragma omp loop
; LOOP {
;   try {
;     xxx();
;   } catch (...) { // unwind to "terminate"
;   }
; } end LOOP
; The catchswitch instructions in each region have an unwind edge
; (for uncaught exceptions).
; The unwind edges and terminate handlers for the regions have been
; merged together, which creates illegal shared control flow between the
; 2 regions.
; Paropt needs to split the handlers so they are distinct.

; CHECK-LABEL: @caller
; CHECK: catchswitch{{.*}} unwind label %terminate

; CHECK: define{{.*}}@caller.DIR.OMP.PARALLEL
; CHECK: catchswitch{{.*}} unwind label %terminate.termpad

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.28.29334"

%rtti.TypeDescriptor7.0.20.28.48.60.80.92.96.108.116.128.164.172.200.208.228.240.248.312.316.320.324.328.346.360 = type { ptr, ptr, [8 x i8] }
%class.a.3.23.31.51.63.83.95.99.111.119.131.167.175.203.211.231.243.251.315.319.323.327.331.347.361 = type { i8 }

@"?bf@@3HA" = external dso_local global i32, align 4
@"?c@@3HA" = external dso_local global i32, align 4
@"?bg@@3HA" = external dso_local global i32, align 4
@"?d@@3HA" = external dso_local global i32, align 4
@"?bh@@3_NA" = external dso_local global i8, align 1
@"?e@@3MA" = external dso_local global float, align 4
@"??_R0?AVa@@@8" = external global %rtti.TypeDescriptor7.0.20.28.48.60.80.92.96.108.116.128.164.172.200.208.228.240.248.312.316.320.324.328.346.360

define dso_local void @caller() local_unnamed_addr #0 personality ptr @__CxxFrameHandler3 {
DIR.OMP.PARALLEL.324:
  %end.dir.temp = alloca i1, align 1
  br label %DIR.OMP.PARALLEL.2

DIR.OMP.PARALLEL.2:                               ; preds = %DIR.OMP.PARALLEL.324
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr @"?bf@@3HA", i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @"?bg@@3HA", i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @"?c@@3HA", i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @"?d@@3HA", i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @"?bh@@3_NA", i8 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr @"?e@@3MA", float 0.000000e+00, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.4, label %DIR.OMP.PARALLEL.3

DIR.OMP.PARALLEL.3:                               ; preds = %DIR.OMP.PARALLEL.2
  invoke void @callee()
          to label %DIR.OMP.END.PARALLEL.4 unwind label %catch.dispatch

catch.dispatch:                                   ; preds = %DIR.OMP.PARALLEL.3
  %1 = catchswitch within none [label %catch] unwind label %terminate

catch:                                            ; preds = %catch.dispatch
  %2 = catchpad within %1 [ptr @"??_R0?AVa@@@8", i32 0, ptr null]
  unreachable

DIR.OMP.END.PARALLEL.4:                           ; preds = %DIR.OMP.PARALLEL.3, %DIR.OMP.PARALLEL.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  %end.dir.temp20 = alloca i1, align 1
  br label %DIR.OMP.LOOP.6

DIR.OMP.LOOP.6:                                   ; preds = %DIR.OMP.END.PARALLEL.4
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr @"?d@@3HA", i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr poison, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr poison, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr poison, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr null, %class.a.3.23.31.51.63.83.95.99.111.119.131.167.175.203.211.231.243.251.315.319.323.327.331.347.361 zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr poison, %class.a.3.23.31.51.63.83.95.99.111.119.131.167.175.203.211.231.243.251.315.319.323.327.331.347.361 zeroinitializer, i32 1),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp20) ]

  %temp.load21 = load volatile i1, ptr %end.dir.temp20, align 1
  br i1 %temp.load21, label %DIR.OMP.END.LOOP.830, label %DIR.OMP.LOOP.7

DIR.OMP.LOOP.7:                                   ; preds = %DIR.OMP.LOOP.6
  invoke void @callee()
          to label %omp.inner.for.inc unwind label %catch.dispatch6

catch.dispatch6:                                  ; preds = %DIR.OMP.LOOP.7
  %4 = catchswitch within none [label %catch7] unwind label %terminate

catch7:                                           ; preds = %catch.dispatch6
  %5 = catchpad within %4 [ptr @"??_R0?AVa@@@8", i32 0, ptr null]
  unreachable

omp.inner.for.inc:                                ; preds = %DIR.OMP.LOOP.7
  unreachable

DIR.OMP.END.LOOP.830:                             ; preds = %DIR.OMP.LOOP.6
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]

  ret void

terminate:                                        ; preds = %catch.dispatch6, %catch.dispatch
  %6 = cleanuppad within none []
  unreachable
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @callee() local_unnamed_addr #0

; Function Attrs: nofree
declare dso_local i32 @__CxxFrameHandler3(...) #2

attributes #0 = { "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { nofree }

