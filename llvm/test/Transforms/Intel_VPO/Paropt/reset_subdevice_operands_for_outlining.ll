; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug -S %s 2>&1 | FileCheck %s
;
; Test src:
;
; void foo(int* p)
; {
; int n;
; #pragma omp target device(0) subdevice(1,n:1:1) map(p[0:20])
;  {
;   p[2] = 3;
;  }
; }
;
; This test checks that we are setting non-constant Subdevice clause operands to null before outlining.
; CHECK: %{{.*}} = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.DEVICE"(i32 0), "QUAL.OMP.SUBDEVICE"(i32 1, i8* null, i32 1, i32 1)

source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 0, void ()* @.omp_offloading.requires_reg, i8* null }]

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(i32* %p) #0 {
entry:
  %p.addr = alloca i32*, align 8
  %n = alloca i32, align 4
  %.capture_expr.0 = alloca i32, align 4
  %p.map.ptr.tmp = alloca i32*, align 8
  store i32* %p, i32** %p.addr, align 8
  %0 = load i32, i32* %n, align 4
  store i32 %0, i32* %.capture_expr.0, align 4
  %1 = load i32, i32* %.capture_expr.0, align 4
  %2 = load i32*, i32** %p.addr, align 8
  %3 = load i32*, i32** %p.addr, align 8
  %4 = load i32*, i32** %p.addr, align 8
  %arrayidx = getelementptr inbounds i32, i32* %4, i64 0
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.DEVICE"(i32 0), "QUAL.OMP.SUBDEVICE"(i32 1, i32 %1, i32 1, i32 1), "QUAL.OMP.MAP.TOFROM"(i32* %3, i32* %arrayidx, i64 80, i64 35), "QUAL.OMP.PRIVATE"(i32** %p.map.ptr.tmp) ]
  store i32* %3, i32** %p.map.ptr.tmp, align 8
  %6 = load i32*, i32** %p.map.ptr.tmp, align 8
  %ptridx = getelementptr inbounds i32, i32* %6, i64 2
  store i32 3, i32* %ptridx, align 4
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_offloading.requires_reg() #2 section ".text.startup" {
entry:
  call void @__tgt_register_requires(i64 1)
  ret void
}

; Function Attrs: nounwind
declare void @__tgt_register_requires(i64) #1

attributes #0 = { noinline nounwind optnone uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind uwtable "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 2054, i32 61031815, !"_Z3foo", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 9.0.0"}
