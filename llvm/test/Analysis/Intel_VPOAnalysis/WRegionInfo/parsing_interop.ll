; RUN: opt -enable-new-pm=0 -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s
;
; Test src: Input IR is written by hand because FE does not yet handle interop.
; C++ example:
; omp_interop_t  obj1 = 0;
; omp_interop_t  obj2 = 0;
; #pragma omp interop destroy(obj2) init(prefer_type(level_zero,opencl),target,targetsync:obj1)

; ModuleID = 'test2.ll'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(i8* %interop_obj, i8* %interop_obj2) #0 {
entry:
  %a = alloca i32, align 4
  %interop_obj.addr = alloca i8*, align 8
  store i8* %interop_obj, i8** %interop_obj.addr, align 8
  %interop_obj2.addr = alloca i8*, align 8
  store i8* %interop_obj2, i8** %interop_obj2.addr, align 8
  br label %DIR.OMP.INTEROP.1

; Check that Interop directive and the destroy and init clauses are parsed
; Check for the debug string.
;CHECK: BEGIN INTEROP ID=1 {
;CHECK:      DEVICE: UNSPECIFIED
;CHECK-NEXT: NOWAIT: false
;CHECK-NEXT: DESTROY clause (size=1): (i8* %interop_obj2)
;CHECK-NEXT: INIT clause (size=1): (i8* %interop_obj) TARGET TARGETSYNC PREFER_TYPE < 6 (LEVEL0) 3 (OpenCL) >

DIR.OMP.INTEROP.1:                                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(), "QUAL.OMP.DESTROY" (i8* %interop_obj2), "QUAL.OMP.INIT:TARGET.TARGETSYNC.PREFER"(i8* %interop_obj , i32 6, i32 3)]
  br label %DIR.OMP.INTEROP.2

DIR.OMP.INTEROP.2:                                ; preds = %DIR.OMP.INTEROP.1
  %b = alloca i32, align 4
  br label %DIR.OMP.INTEROP.3

DIR.OMP.INTEROP.3:                                ; preds = %DIR.OMP.INTEROP.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.INTEROP"() ]
  br label %DIR.OMP.INTEROP.4

;CHECK: BEGIN INTEROP ID=2 {
;CHECK:      DEVICE: UNSPECIFIED
;CHECK-NEXT: NOWAIT: false
;CHECK-NEXT: USE clause (size=1): (i8* %interop_obj)

DIR.OMP.INTEROP.4:                                ; preds = %DIR.OMP.INTEROP.3
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(), "QUAL.OMP.USE"(i8* %interop_obj) ]
  br label %DIR.OMP.INTEROP.5

DIR.OMP.INTEROP.5:                                ; preds = %DIR.OMP.INTEROP.4
  %c = alloca i32, align 4
  br label %DIR.OMP.INTEROP.6

DIR.OMP.INTEROP.6:                                ; preds = %DIR.OMP.INTEROP.5
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.INTEROP"() ]
  br label %DIR.OMP.INTEROP.7

;CHECK: BEGIN INTEROP ID=3 {
;CHECK:      DEVICE: UNSPECIFIED
;CHECK-NEXT: NOWAIT: false
;CHECK-NEXT: DESTROY clause (size=1): (i8* %interop_obj)

DIR.OMP.INTEROP.7:                                ; preds = %DIR.OMP.INTEROP.6
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.INTEROP"(), "QUAL.OMP.DESTROY"(i8* %interop_obj) ]
  br label %DIR.OMP.INTEROP.8

DIR.OMP.INTEROP.8:                                ; preds = %DIR.OMP.INTEROP.7
  %d = alloca i32, align 4
  br label %DIR.OMP.INTEROP.9

DIR.OMP.INTEROP.9:                                ; preds = %DIR.OMP.INTEROP.8
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.INTEROP"() ]
  br label %DIR.OMP.INTEROP.10

DIR.OMP.INTEROP.10:                               ; preds = %DIR.OMP.INTEROP.9
  %e = alloca i32, align 4
  ret i32 123
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
