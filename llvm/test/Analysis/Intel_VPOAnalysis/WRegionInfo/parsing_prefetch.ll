; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s

; // Parsing test for the prefetch construct
; Test src:
;
; int cond;
; void foo(int *ptr, float *fff) {
; #pragma omp prefetch data(ptr : 1 : 10, fff : 2 : 20) if (cond > 3)
;   ptr[3] = fff[5];
; }

; CHECK: BEGIN PREFETCH
; CHECK: IF_EXPR:
; CHECK: DATA clause (size=2): (ptr %{{.*}} : 1 : 10) (ptr %{{.*}} : 2 : 20)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@cond = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(ptr noundef %ptr, ptr noundef %fff) #0 {
entry:
  %ptr.addr = alloca ptr, align 8
  %fff.addr = alloca ptr, align 8
  %.tmp.prefetch = alloca ptr, align 8
  %.tmp.prefetch1 = alloca ptr, align 8
  store ptr %ptr, ptr %ptr.addr, align 8
  store ptr %fff, ptr %fff.addr, align 8
  %0 = load ptr, ptr %ptr.addr, align 8
  store ptr %0, ptr %.tmp.prefetch, align 8
  %1 = load ptr, ptr %fff.addr, align 8
  store ptr %1, ptr %.tmp.prefetch1, align 8
  %2 = load i32, ptr @cond, align 4
  %cmp = icmp sgt i32 %2, 3
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.PREFETCH"(),
    "QUAL.OMP.DATA"(ptr %.tmp.prefetch, i32 1, i64 10),
    "QUAL.OMP.DATA"(ptr %.tmp.prefetch1, i32 2, i64 20),
    "QUAL.OMP.IF"(i1 %cmp) ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.PREFETCH"() ]
  %4 = load ptr, ptr %fff.addr, align 8
  %arrayidx = getelementptr inbounds float, ptr %4, i64 5
  %5 = load float, ptr %arrayidx, align 4
  %conv = fptosi float %5 to i32
  %6 = load ptr, ptr %ptr.addr, align 8
  %arrayidx2 = getelementptr inbounds i32, ptr %6, i64 3
  store i32 %conv, ptr %arrayidx2, align 4
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
