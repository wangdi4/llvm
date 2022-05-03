; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s

; void foo() {
;   int a[100], n, m;
;   // case 1
;   #pragma omp loop bind(thread) // -> SIMD
;   for (int i=0; i<100; i++)
;     a[i] = n + m;
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
; case 1: GenericLoop --> SIMD; check that
;    a. SHARED --> LIVEIN
;    b. FIRSTPRIVATE --> LIVEIN
;    c. variables are renamed with OPERAND.ADDR
;    d. SHARED(m,n) --> LIVEIN(m,n) // ie, support multiple vars in the same clause
;
; case 2: GenericLoop --> FOR; check that
;    a. SHARED --> LIVEIN
;    b. FIRSTPRIVATE --> unchanged
;    c. variables are renamed with OPERAND.ADDR
;
; case 3: GenericLoop --> DISTRIBUTE PARALLEL FOR; check that
;    a. SHARED --> unchanged
;    b. FIRSTPRIVATE --> unchanged
;    c. variables are renamed with OPERAND.ADDR
;
; ModuleID = 'tt.c'
source_filename = "tt.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %a = alloca [100 x i32], align 16
  %n = alloca i32, align 4
  %m = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %tmp3 = alloca i32, align 4
  %.omp.iv4 = alloca i32, align 4
  %.omp.lb5 = alloca i32, align 4
  %.omp.ub6 = alloca i32, align 4
  %i10 = alloca i32, align 4
  %tmp20 = alloca i32, align 4
  %.omp.iv21 = alloca i32, align 4
  %.omp.lb22 = alloca i32, align 4
  %.omp.ub23 = alloca i32, align 4
  %i27 = alloca i32, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.BIND.THREAD"(), "QUAL.OMP.SHARED"([100 x i32]* %a), "QUAL.OMP.SHARED"(i32* %m, i32* %n), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]

; CHECK: "DIR.OMP.SIMD"(), "QUAL.OMP.LIVEIN"([100 x i32]* %a), "QUAL.OMP.LIVEIN"(i32* %m, i32* %n), {{.*}}, "QUAL.OMP.LIVEIN"(i32* %.omp.lb), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb, i32** %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"([100 x i32]* %a, [100 x i32]** %a.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %m, i32** %m.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %n, i32** %n.addr)

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
  %6 = load i32, i32* %m, align 4
  %add1 = add nsw i32 %5, %6
  %7 = load i32, i32* %i, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom
  store i32 %add1, i32* %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add2 = add nsw i32 %8, 1
  store i32 %add2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]
  store i32 0, i32* %.omp.lb5, align 4
  store i32 99, i32* %.omp.ub6, align 4

  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.BIND.PARALLEL"(), "QUAL.OMP.SHARED"([100 x i32]* %a), "QUAL.OMP.SHARED"(i32* %n), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv4), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb5), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub6), "QUAL.OMP.PRIVATE"(i32* %i10) ]

; CHECK: "DIR.OMP.LOOP"(), "QUAL.OMP.LIVEIN"([100 x i32]* %a), "QUAL.OMP.LIVEIN"(i32* %n), {{.*}}, "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb5), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb5, i32** %.omp.lb5.addr), "QUAL.OMP.OPERAND.ADDR"([100 x i32]* %a, [100 x i32]** %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(i32* %n, i32** %n.addr{{[0-9]*}})

  %10 = load i32, i32* %.omp.lb5, align 4
  store i32 %10, i32* %.omp.iv4, align 4
  br label %omp.inner.for.cond7

omp.inner.for.cond7:                              ; preds = %omp.inner.for.inc16, %omp.loop.exit
  %11 = load i32, i32* %.omp.iv4, align 4
  %12 = load i32, i32* %.omp.ub6, align 4
  %cmp8 = icmp sle i32 %11, %12
  br i1 %cmp8, label %omp.inner.for.body9, label %omp.inner.for.end18

omp.inner.for.body9:                              ; preds = %omp.inner.for.cond7
  %13 = load i32, i32* %.omp.iv4, align 4
  %mul11 = mul nsw i32 %13, 1
  %add12 = add nsw i32 0, %mul11
  store i32 %add12, i32* %i10, align 4
  %14 = load i32, i32* %n, align 4
  %15 = load i32, i32* %i10, align 4
  %idxprom13 = sext i32 %15 to i64
  %arrayidx14 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom13
  store i32 %14, i32* %arrayidx14, align 4
  br label %omp.body.continue15

omp.body.continue15:                              ; preds = %omp.inner.for.body9
  br label %omp.inner.for.inc16

omp.inner.for.inc16:                              ; preds = %omp.body.continue15
  %16 = load i32, i32* %.omp.iv4, align 4
  %add17 = add nsw i32 %16, 1
  store i32 %add17, i32* %.omp.iv4, align 4
  br label %omp.inner.for.cond7

omp.inner.for.end18:                              ; preds = %omp.inner.for.cond7
  br label %omp.loop.exit19

omp.loop.exit19:                                  ; preds = %omp.inner.for.end18
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.GENERICLOOP"() ]
  store i32 0, i32* %.omp.lb22, align 4
  store i32 99, i32* %.omp.ub23, align 4

  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(), "QUAL.OMP.BIND.TEAMS"(), "QUAL.OMP.SHARED"([100 x i32]* %a), "QUAL.OMP.SHARED"(i32* %n), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv21), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb22), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub23), "QUAL.OMP.PRIVATE"(i32* %i27) ]

; CHECK: "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED"([100 x i32]* %a), "QUAL.OMP.SHARED"(i32* %n), {{.*}}, "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb22), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(i32* %.omp.lb22, i32** %.omp.lb22.addr), "QUAL.OMP.OPERAND.ADDR"([100 x i32]* %a, [100 x i32]** %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(i32* %n, i32** %n.addr{{[0-9]*}})

  %18 = load i32, i32* %.omp.lb22, align 4
  store i32 %18, i32* %.omp.iv21, align 4
  br label %omp.inner.for.cond24

omp.inner.for.cond24:                             ; preds = %omp.inner.for.inc33, %omp.loop.exit19
  %19 = load i32, i32* %.omp.iv21, align 4
  %20 = load i32, i32* %.omp.ub23, align 4
  %cmp25 = icmp sle i32 %19, %20
  br i1 %cmp25, label %omp.inner.for.body26, label %omp.inner.for.end35

omp.inner.for.body26:                             ; preds = %omp.inner.for.cond24
  %21 = load i32, i32* %.omp.iv21, align 4
  %mul28 = mul nsw i32 %21, 1
  %add29 = add nsw i32 0, %mul28
  store i32 %add29, i32* %i27, align 4
  %22 = load i32, i32* %n, align 4
  %23 = load i32, i32* %i27, align 4
  %idxprom30 = sext i32 %23 to i64
  %arrayidx31 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom30
  store i32 %22, i32* %arrayidx31, align 4
  br label %omp.body.continue32

omp.body.continue32:                              ; preds = %omp.inner.for.body26
  br label %omp.inner.for.inc33

omp.inner.for.inc33:                              ; preds = %omp.body.continue32
  %24 = load i32, i32* %.omp.iv21, align 4
  %add34 = add nsw i32 %24, 1
  store i32 %add34, i32* %.omp.iv21, align 4
  br label %omp.inner.for.cond24

omp.inner.for.end35:                              ; preds = %omp.inner.for.cond24
  br label %omp.loop.exit36

omp.loop.exit36:                                  ; preds = %omp.inner.for.end35
  call void @llvm.directive.region.exit(token %17) [ "DIR.OMP.END.GENERICLOOP"() ]
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
