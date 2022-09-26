; REQUIRES: asserts
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-wrncollection -analyze -debug -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -debug -S %s 2>&1 | FileCheck %s

; Test src:
;
; void foo(int *ptr, int *a, float *b, double *c, int i, int n) {
; #pragma ompx prefetch data(2 : a [i:4], b [i:n]) data(c [i:8]) if (i % 32 == 0)
;   ptr[i] = a[i];
; }

; CHECK: BEGIN PREFETCH
; CHECK: IF_EXPR:
; CHECK: DATA clause (size=3):
; CEHCK-SAME: (ptr %arrayidx, TYPE: i32, NUM_ELEMENTS: i64 %sec.number_of_elements, HINT: 2)
; CEHCK-SAME: (ptr %arrayidx2, TYPE: float, NUM_ELEMENTS: i64 %sec.number_of_elements7, HINT: 2)
; CEHCK-SAME: (ptr %arrayidx8, TYPE: double, NUM_ELEMENTS: i64 %sec.number_of_elements13, HINT: 0)

; Parsing test for the prefetch construct

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
  store ptr %ptr, ptr %ptr.addr, align 8
  store ptr %a, ptr %a.addr, align 8
  store ptr %b, ptr %b.addr, align 8
  store ptr %c, ptr %c.addr, align 8
  store i32 %i, ptr %i.addr, align 4
  store i32 %n, ptr %n.addr, align 4
  %0 = load i32, ptr %i.addr, align 4
  %1 = sext i32 %0 to i64
  %2 = load ptr, ptr %a.addr, align 8
  %arrayidx = getelementptr inbounds i32, ptr %2, i64 %1
  %sec.lower.cast = ptrtoint ptr %arrayidx to i64
  %3 = load i32, ptr %i.addr, align 4
  %4 = sext i32 %3 to i64
  %lb_add_len = add nsw i64 %4, 3
  %5 = load ptr, ptr %a.addr, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %5, i64 %lb_add_len
  %sec.upper.cast = ptrtoint ptr %arrayidx1 to i64
  %6 = sub i64 %sec.upper.cast, %sec.lower.cast
  %7 = sdiv exact i64 %6, 8
  %sec.number_of_elements = add i64 %7, 1
  %8 = load i32, ptr %i.addr, align 4
  %9 = sext i32 %8 to i64
  %10 = load ptr, ptr %b.addr, align 8
  %arrayidx2 = getelementptr inbounds float, ptr %10, i64 %9
  %sec.lower.cast3 = ptrtoint ptr %arrayidx2 to i64
  %11 = load i32, ptr %i.addr, align 4
  %12 = sext i32 %11 to i64
  %13 = load i32, ptr %n.addr, align 4
  %14 = sext i32 %13 to i64
  %lb_add_len4 = add nsw i64 %12, %14
  %idx_sub_1 = sub nsw i64 %lb_add_len4, 1
  %15 = load ptr, ptr %b.addr, align 8
  %arrayidx5 = getelementptr inbounds float, ptr %15, i64 %idx_sub_1
  %sec.upper.cast6 = ptrtoint ptr %arrayidx5 to i64
  %16 = sub i64 %sec.upper.cast6, %sec.lower.cast3
  %17 = sdiv exact i64 %16, 8
  %sec.number_of_elements7 = add i64 %17, 1
  %18 = load i32, ptr %i.addr, align 4
  %19 = sext i32 %18 to i64
  %20 = load ptr, ptr %c.addr, align 8
  %arrayidx8 = getelementptr inbounds double, ptr %20, i64 %19
  %sec.lower.cast9 = ptrtoint ptr %arrayidx8 to i64
  %21 = load i32, ptr %i.addr, align 4
  %22 = sext i32 %21 to i64
  %lb_add_len10 = add nsw i64 %22, 7
  %23 = load ptr, ptr %c.addr, align 8
  %arrayidx11 = getelementptr inbounds double, ptr %23, i64 %lb_add_len10
  %sec.upper.cast12 = ptrtoint ptr %arrayidx11 to i64
  %24 = sub i64 %sec.upper.cast12, %sec.lower.cast9
  %25 = sdiv exact i64 %24, 8
  %sec.number_of_elements13 = add i64 %25, 1
  %26 = load i32, ptr %i.addr, align 4
  %rem = srem i32 %26, 32
  %cmp = icmp eq i32 %rem, 0
  %27 = call token @llvm.directive.region.entry() [ "DIR.OMP.PREFETCH"(),
    "QUAL.OMP.DATA"(ptr %arrayidx, i32 0, i64 %sec.number_of_elements, i32 2),
    "QUAL.OMP.DATA"(ptr %arrayidx2, float 0.000000e+00, i64 %sec.number_of_elements7, i32 2),
    "QUAL.OMP.DATA"(ptr %arrayidx8, double 0.000000e+00, i64 %sec.number_of_elements13, i32 0),
    "QUAL.OMP.IF"(i1 %cmp) ]
  call void @llvm.directive.region.exit(token %27) [ "DIR.OMP.END.PREFETCH"() ]
  %28 = load ptr, ptr %a.addr, align 8
  %29 = load i32, ptr %i.addr, align 4
  %idxprom = sext i32 %29 to i64
  %arrayidx14 = getelementptr inbounds i32, ptr %28, i64 %idxprom
  %30 = load i32, ptr %arrayidx14, align 4
  %31 = load ptr, ptr %ptr.addr, align 8
  %32 = load i32, ptr %i.addr, align 4
  %idxprom15 = sext i32 %32 to i64
  %arrayidx16 = getelementptr inbounds i32, ptr %31, i64 %idxprom15
  store i32 %30, ptr %arrayidx16, align 4
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
