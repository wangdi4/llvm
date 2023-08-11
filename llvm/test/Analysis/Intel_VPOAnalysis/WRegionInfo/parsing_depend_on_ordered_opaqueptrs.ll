; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-wrncollection -analyze -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,print<vpo-wrncollection>)' -S %s 2>&1 | FileCheck %s
;
; Check that DEPEND(SINK|SOURCE) is parsed internally as DOACROSS(SINK|SOURCE) clause

; Test src:
;
;  #include <stdio.h>
;  void foo(int (*v_ptr)[5][4]) {
;   int i, j;
; #pragma omp for ordered(2) private(j) schedule(static)
;   for (i = 1; i < 5; i++) {
;     for (j = 2; j < 4; j++) {
; #pragma omp ordered depend(sink : i - 1, j - 1) depend(sink : i, j - 2)
;       (*v_ptr)[i][j] = (*v_ptr)[i - 1][j - 1] + (*v_ptr)[i][j - 2];
; #pragma omp ordered depend(source)
;     }
;   }
; } 

; CHECK: BEGIN ORDERED ID=2 {
; CHECK:  DOACROSS.SINK clause (size=2): (i32 %{{.*}} i32 %{{.*}} ) (i32 %{{.*}} i32 %{{.*}} )
; CHECK: } END ORDERED ID=2

; CHECK: BEGIN ORDERED ID=3 {
; CHECK: DOACROSS.SOURCE clause (size=1): (i32 %{{.*}} i32 %{{.*}} )
; CHECK: } END ORDERED ID=3

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @foo(ptr noundef %v_ptr) {
entry:
  %v_ptr.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store ptr %v_ptr, ptr %v_ptr.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 3, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.ORDERED"(i32 2, i32 4, i32 2),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.SCHEDULE.STATIC"(i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 1, %mul
  store i32 %add, ptr %i, align 4
  store i32 2, ptr %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %5 = load i32, ptr %j, align 4
  %cmp1 = icmp slt i32 %5, 4
  br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %6 = load i32, ptr %i, align 4
  %sub = sub nsw i32 %6, 1
  %sub2 = sub nsw i32 %sub, 1
  %div = sdiv i32 %sub2, 1
  %7 = load i32, ptr %j, align 4
  %sub3 = sub nsw i32 %7, 1
  %sub4 = sub nsw i32 %sub3, 2
  %div5 = sdiv i32 %sub4, 1
  %8 = load i32, ptr %i, align 4
  %sub6 = sub nsw i32 %8, 1
  %div7 = sdiv i32 %sub6, 1
  %9 = load i32, ptr %j, align 4
  %sub8 = sub nsw i32 %9, 2
  %sub9 = sub nsw i32 %sub8, 2
  %div10 = sdiv i32 %sub9, 1
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(),
    "QUAL.OMP.DEPEND.SINK"(i32 %div, i32 %div5),
    "QUAL.OMP.DEPEND.SINK"(i32 %div7, i32 %div10) ]

  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.ORDERED"() ]
  %11 = load ptr, ptr %v_ptr.addr, align 8
  %12 = load i32, ptr %i, align 4
  %sub11 = sub nsw i32 %12, 1
  %idxprom = sext i32 %sub11 to i64
  %arrayidx = getelementptr inbounds [5 x [4 x i32]], ptr %11, i64 0, i64 %idxprom
  %13 = load i32, ptr %j, align 4
  %sub12 = sub nsw i32 %13, 1
  %idxprom13 = sext i32 %sub12 to i64
  %arrayidx14 = getelementptr inbounds [4 x i32], ptr %arrayidx, i64 0, i64 %idxprom13
  %14 = load i32, ptr %arrayidx14, align 4
  %15 = load ptr, ptr %v_ptr.addr, align 8
  %16 = load i32, ptr %i, align 4
  %idxprom15 = sext i32 %16 to i64
  %arrayidx16 = getelementptr inbounds [5 x [4 x i32]], ptr %15, i64 0, i64 %idxprom15
  %17 = load i32, ptr %j, align 4
  %sub17 = sub nsw i32 %17, 2
  %idxprom18 = sext i32 %sub17 to i64
  %arrayidx19 = getelementptr inbounds [4 x i32], ptr %arrayidx16, i64 0, i64 %idxprom18
  %18 = load i32, ptr %arrayidx19, align 4
  %add20 = add nsw i32 %14, %18
  %19 = load ptr, ptr %v_ptr.addr, align 8
  %20 = load i32, ptr %i, align 4
  %idxprom21 = sext i32 %20 to i64
  %arrayidx22 = getelementptr inbounds [5 x [4 x i32]], ptr %19, i64 0, i64 %idxprom21
  %21 = load i32, ptr %j, align 4
  %idxprom23 = sext i32 %21 to i64
  %arrayidx24 = getelementptr inbounds [4 x i32], ptr %arrayidx22, i64 0, i64 %idxprom23
  store i32 %add20, ptr %arrayidx24, align 4
  %22 = load i32, ptr %i, align 4
  %sub25 = sub nsw i32 %22, 1
  %div26 = sdiv i32 %sub25, 1
  %23 = load i32, ptr %j, align 4
  %sub27 = sub nsw i32 %23, 2
  %div28 = sdiv i32 %sub27, 1
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(),
    "QUAL.OMP.DEPEND.SOURCE"(i32 %div26, i32 %div28) ]

  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.ORDERED"() ]
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %25 = load i32, ptr %j, align 4
  %inc = add nsw i32 %25, 1
  store i32 %inc, ptr %j, align 4
  br label %for.cond, !llvm.loop !5

for.end:                                          ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %26 = load i32, ptr %.omp.iv, align 4
  %add29 = add nsw i32 %26, 1
  store i32 %add29, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
