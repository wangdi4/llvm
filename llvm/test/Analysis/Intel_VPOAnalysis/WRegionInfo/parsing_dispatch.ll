; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s

; Parsing test for OMP5.1 dispatch construct.
; // C++ source:
; void foo();
; int *ptr;
; void bar() {
; #pragma omp dispatch is_device_ptr(ptr) device(0) nocontext(true)              \
;     novariants(false) nowait
;   foo();
; }
;
; The expected WRN dump is:
;
;   BEGIN DISPATCH ID=2 {
;
;     DEVICE: i32 0
;     NOCONTEXT: i1 true
;     NOVARIANTS: i1 false
;     NOWAIT: true
;     SUBDEVICE clause: UNSPECIFIED
;     IS_DEVICE_PTR clause (size=1): PTR_TO_PTR(ptr @ptr)
;
;     EntryBB: DIR.OMP.TASK.3
;     ExitBB: DIR.OMP.END.DISPATCH.5
;
;   } END DISPATCH ID=2

; CHECK: BEGIN DISPATCH
; CHECK: DEVICE: i32 0
; CHECK: NOCONTEXT: i1 true
; CHECK: NOVARIANTS: i1 false
; CHECK: NOWAIT: true
; CHECK: SUBDEVICE clause: UNSPECIFIED
; CHECK: IS_DEVICE_PTR clause (size=1): PTR_TO_PTR(ptr @ptr)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ptr = dso_local global ptr null, align 8

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3barv() #0 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IMPLICIT"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr @ptr, ptr null, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(ptr @ptr),
    "QUAL.OMP.DEVICE"(i32 0),
    "QUAL.OMP.NOCONTEXT"(i1 true),
    "QUAL.OMP.NOVARIANTS"(i1 false),
    "QUAL.OMP.NOWAIT"() ]
  call void @_Z3foov() #1 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISPATCH"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z3foov() #2

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
