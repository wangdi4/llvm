; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s

; Parsing test for OMP5.1 dispatch construct.
;   // C++ source:
;   void foo();
;   int *ptr;
;   #pragma omp dispatch is_device_ptr(ptr) device(0) nocontext(true) novariants(false) nowait
;      foo();
;
; Note: The test IR is hand-generated and does not have the implicit task
;       due to "nowait", which is not needed for this parsing test
;
; The expected WRN dump is:
;
; BEGIN DISPATCH ID=1 {
;
;   DEVICE: i32 0
;   NOCONTEXT: i1 true
;   NOVARIANTS: i1 false
;   NOWAIT: true
;   SUBDEVICE clause: UNSPECIFIED
;   IS_DEVICE_PTR clause (size=1): PTR_TO_PTR(i32** %ptr)
;
;   EntryBB: DIR.OMP.DISPATCH.1
;   ExitBB: DIR.OMP.END.DISPATCH.3
;
; } END DISPATCH ID=1


; CHECK: BEGIN DISPATCH
; CHECK: DEVICE: i32 0
; CHECK: NOCONTEXT: i1 true
; CHECK: NOVARIANTS: i1 false
; CHECK: NOWAIT: true
; CHECK: SUBDEVICE clause: UNSPECIFIED
; CHECK: IS_DEVICE_PTR clause (size=1): PTR_TO_PTR(i32** %ptr)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #0 {
entry:
  %ptr = alloca i32*, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(), "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(i32** %ptr), "QUAL.OMP.DEVICE"(i32 0), "QUAL.OMP.NOCONTEXT"(i1 true), "QUAL.OMP.NOVARIANTS"(i1 false), "QUAL.OMP.NOWAIT"()]
  call void @_Z3foov() #2 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z3foov() #2

attributes #0 = { noinline norecurse nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
