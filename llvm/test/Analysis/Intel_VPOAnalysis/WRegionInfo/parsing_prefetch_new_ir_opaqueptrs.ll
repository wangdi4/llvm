; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s

; // Parsing test for the prefetch construct
; Test src:
;
; void foo(int *ptr, int *a, float *b, double *c, int i, int n) {
; #pragma ompx prefetch data(2 : a [i:4], b [i:n])                             \
;     data(c [i:8]) if (i % 32 == 0)
;   ptr[i] = a[i];
; }

; The IR is a hand-modified version because FE hasn't support the new syntax of DATA clause.

; CHECK: BEGIN PREFETCH
; CHECK: IF_EXPR:
; CHECK: DATA clause (size=3):
; CEHCK-SAME: (ptr %.tmp.prefetch, TYPE: i32, NUM_ELEMENTS: i64 4, HINT: 2)
; CEHCK-SAME: (ptr %.tmp.prefetch1, TYPE: float, NUM_ELEMENTS: i32 %n, HINT: 2)
; CEHCK-SAME: (ptr %.tmp.prefetch4, TYPE: double, NUM_ELEMENTS: i64 8, HINT: 0)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo(ptr noundef %ptr, ptr noundef %a, ptr noundef %b, ptr noundef %c, i32 noundef %i, i32 noundef %n) #0 {
entry:
  %ptr.addr = alloca ptr, align 8
  %a.addr = alloca ptr, align 8
  %b.addr = alloca ptr, align 8
  %c.addr = alloca ptr, align 8
  %i.addr = alloca i32, align 4
  %n.addr = alloca i32, align 4
  %.tmp.prefetch = alloca ptr, align 8
  %.tmp.prefetch1 = alloca ptr, align 8
  %.tmp.prefetch4 = alloca ptr, align 8
  store ptr %ptr, ptr %ptr.addr, align 8
  store ptr %a, ptr %a.addr, align 8
  store ptr %b, ptr %b.addr, align 8
  store ptr %c, ptr %c.addr, align 8
  store i32 %i, ptr %i.addr, align 4
  store i32 %n, ptr %n.addr, align 4
  %0 = load ptr, ptr %a.addr, align 8
  %1 = load i32, ptr %i.addr, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds i32, ptr %0, i64 %idxprom
  store ptr %arrayidx, ptr %.tmp.prefetch, align 8
  %2 = load ptr, ptr %b.addr, align 8
  %3 = load i32, ptr %i.addr, align 4
  %idxprom2 = sext i32 %3 to i64
  %arrayidx3 = getelementptr inbounds float, ptr %2, i64 %idxprom2
  store ptr %arrayidx3, ptr %.tmp.prefetch1, align 8
  %4 = load ptr, ptr %c.addr, align 8
  %5 = load i32, ptr %i.addr, align 4
  %idxprom5 = sext i32 %5 to i64
  %arrayidx6 = getelementptr inbounds double, ptr %4, i64 %idxprom5
  store ptr %arrayidx6, ptr %.tmp.prefetch4, align 8
  %6 = load i32, ptr %i.addr, align 4
  %rem = srem i32 %6, 32
  %cmp = icmp eq i32 %rem, 0
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.PREFETCH"(),
    "QUAL.OMP.DATA"(ptr %.tmp.prefetch, i32 0, i64 4, i32 2),
    "QUAL.OMP.DATA"(ptr %.tmp.prefetch1, float 0.000000e+00, i32 %n, i32 2),
    "QUAL.OMP.DATA"(ptr %.tmp.prefetch4, double 0.000000e+00, i64 8, i32 0),
    "QUAL.OMP.IF"(i1 %cmp) ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.PREFETCH"() ]
  %8 = load ptr, ptr %a.addr, align 8
  %9 = load i32, ptr %i.addr, align 4
  %idxprom7 = sext i32 %9 to i64
  %arrayidx8 = getelementptr inbounds i32, ptr %8, i64 %idxprom7
  %10 = load i32, ptr %arrayidx8, align 4
  %11 = load ptr, ptr %ptr.addr, align 8
  %12 = load i32, ptr %i.addr, align 4
  %idxprom9 = sext i32 %12 to i64
  %arrayidx10 = getelementptr inbounds i32, ptr %11, i64 %idxprom9
  store i32 %10, ptr %arrayidx10, align 4
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
