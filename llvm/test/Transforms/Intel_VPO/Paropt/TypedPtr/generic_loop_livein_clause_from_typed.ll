; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s

; voii foo() {
;   int a[100], n;
;   // case 1
;   #pragma omp loop bind(thread) // -> SIMD
;   for (int i=0; i<100; i++)
;     a[i] = n;
;
;   // case 2
;   #pragma omp loop bind(parallel) // -> FOR
;   for (int i=0; i<100; i++)
;     a[i] = n;
;
;   // case 3
;   #pragma omp loop bind(teams) // -> DISTRIBUTE PARALLEL FOR
;   for (int i=0; i<100; i++)
;     a[i] = n;
; }
;
; Similar test to generic_loop_livein_clause.ll but with the input clauses being TYPED.
;
; case 1: GenericLoop --> SIMD; check that
;    a. SHARED:TYPED --> LIVEIN (untyped)
;    b. FIRSTPRIVATE:TYPED --> LIVEIN (untyped)
;    c. variables are renamed with OPERAND.ADDR
;
; case 2: GenericLoop --> FOR; check that
;    a. SHARED:TYPED --> LIVEIN (untyped)
;    b. FIRSTPRIVATE:TYPED --> unchanged
;    c. variables are renamed with OPERAND.ADDR
;
; case 3: GenericLoop --> DISTRIBUTE PARALLEL FOR; check that
;    a. SHARED:TYPED --> unchanged
;    b. FIRSTPRIVATE:TYPED --> unchanged
;    c. variables are renamed with OPERAND.ADDR
;
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %a = alloca [100 x i32], align 16
  %n = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp2 = alloca i32, align 4
  %.omp.iv3 = alloca i32, align 4
  %.omp.lb4 = alloca i32, align 4
  %.omp.ub5 = alloca i32, align 4
  %i9 = alloca i32, align 4
  %tmp20 = alloca i32, align 4
  %.omp.iv21 = alloca i32, align 4
  %.omp.lb22 = alloca i32, align 4
  %.omp.ub23 = alloca i32, align 4
  %i27 = alloca i32, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4

  %array.begin = getelementptr inbounds [100 x i32], [100 x i32]* %a, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.BIND.THREAD"(), "QUAL.OMP.SHARED:TYPED"([100 x i32]* %a, i32 0, i64 100), "QUAL.OMP.SHARED:TYPED"(i32* %n, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb, i32 0, i32 1), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub, i32 0), "QUAL.OMP.PRIVATE:TYPED"(i32* %i, i32 0, i32 1) ]

; CHECK: "DIR.OMP.SIMD"(), "QUAL.OMP.LIVEIN"([100 x i32]* %a), "QUAL.OMP.LIVEIN"(i32* %n), {{.*}}, "QUAL.OMP.LIVEIN"(i32* %.omp.lb), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb, i32** %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"([100 x i32]* %a, [100 x i32]** %a.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %n, i32** %n.addr)

  %1 = load i32, i32* %.omp.lb, align 4
  store i32 %1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, i32* %.omp.iv, align 4
  %3 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %5 = load i32, i32* %n, align 4
  %6 = load i32, i32* %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom
  store i32 %5, i32* %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]
  store i32 0, i32* %.omp.lb4, align 4
  store i32 99, i32* %.omp.ub5, align 4
  %array.begin19 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i32 0, i32 0

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.BIND.PARALLEL"(), "QUAL.OMP.SHARED:TYPED"([100 x i32]* %a, i32 0, i64 100), "QUAL.OMP.SHARED:TYPED"(i32* %n, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv3, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb4, i32 0, i32 1), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub5, i32 0), "QUAL.OMP.PRIVATE:TYPED"(i32* %i9, i32 0, i32 1) ]

; CHECK: "DIR.OMP.LOOP"(), "QUAL.OMP.LIVEIN"([100 x i32]* %a), "QUAL.OMP.LIVEIN"(i32* %n), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb4, i32 0, i32 1), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb4, i32** %.omp.lb4.addr), "QUAL.OMP.OPERAND.ADDR"([100 x i32]* %a, [100 x i32]** %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(i32* %n, i32** %n.addr{{[0-9]*}})

  %9 = load i32, i32* %.omp.lb4, align 4
  store i32 %9, i32* %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc15, %omp.loop.exit
  %10 = load i32, i32* %.omp.iv3, align 4
  %11 = load i32, i32* %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %10, %11
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end17

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %12 = load i32, i32* %.omp.iv3, align 4
  %mul10 = mul nsw i32 %12, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, i32* %i9, align 4
  %13 = load i32, i32* %n, align 4
  %14 = load i32, i32* %i9, align 4
  %idxprom12 = sext i32 %14 to i64
  %arrayidx13 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom12
  store i32 %13, i32* %arrayidx13, align 4
  br label %omp.body.continue14

omp.body.continue14:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc15

omp.inner.for.inc15:                              ; preds = %omp.body.continue14
  %15 = load i32, i32* %.omp.iv3, align 4
  %add16 = add nsw i32 %15, 1
  store i32 %add16, i32* %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end17:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit18

omp.loop.exit18:                                  ; preds = %omp.inner.for.end17
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.GENERICLOOP"() ]
  store i32 0, i32* %.omp.lb22, align 4
  store i32 99, i32* %.omp.ub23, align 4
  %array.begin37 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i32 0, i32 0

  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.BIND.TEAMS"(), "QUAL.OMP.SHARED:TYPED"([100 x i32]* %a, i32 0, i64 100), "QUAL.OMP.SHARED:TYPED"(i32* %n, i32 0, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i32* %.omp.iv21, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb22, i32 0, i32 1), "QUAL.OMP.NORMALIZED.UB:TYPED"(i32* %.omp.ub23, i32 0), "QUAL.OMP.PRIVATE:TYPED"(i32* %i27, i32 0, i32 1) ]

; CHECK: "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED:TYPED"([100 x i32]* %a, i32 0, i64 100), "QUAL.OMP.SHARED:TYPED"(i32* %n, i32 0, i32 1), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(i32* %.omp.lb22, i32 0, i32 1), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb22, i32** %.omp.lb22.addr), "QUAL.OMP.OPERAND.ADDR"([100 x i32]* %a, [100 x i32]** %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(i32* %n, i32** %n.addr{{[0-9]*}})

  %17 = load i32, i32* %.omp.lb22, align 4
  store i32 %17, i32* %.omp.iv21, align 4
  br label %omp.inner.for.cond24

omp.inner.for.cond24:                             ; preds = %omp.inner.for.inc33, %omp.loop.exit18
  %18 = load i32, i32* %.omp.iv21, align 4
  %19 = load i32, i32* %.omp.ub23, align 4
  %cmp25 = icmp sle i32 %18, %19
  br i1 %cmp25, label %omp.inner.for.body26, label %omp.inner.for.end35

omp.inner.for.body26:                             ; preds = %omp.inner.for.cond24
  %20 = load i32, i32* %.omp.iv21, align 4
  %mul28 = mul nsw i32 %20, 1
  %add29 = add nsw i32 0, %mul28
  store i32 %add29, i32* %i27, align 4
  %21 = load i32, i32* %n, align 4
  %22 = load i32, i32* %i27, align 4
  %idxprom30 = sext i32 %22 to i64
  %arrayidx31 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom30
  store i32 %21, i32* %arrayidx31, align 4
  br label %omp.body.continue32

omp.body.continue32:                              ; preds = %omp.inner.for.body26
  br label %omp.inner.for.inc33

omp.inner.for.inc33:                              ; preds = %omp.body.continue32
  %23 = load i32, i32* %.omp.iv21, align 4
  %add34 = add nsw i32 %23, 1
  store i32 %add34, i32* %.omp.iv21, align 4
  br label %omp.inner.for.cond24

omp.inner.for.end35:                              ; preds = %omp.inner.for.cond24
  br label %omp.loop.exit36

omp.loop.exit36:                                  ; preds = %omp.inner.for.end35
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.GENERICLOOP"() ]
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
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
