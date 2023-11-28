; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -S <%s | FileCheck -check-prefix=SCHEME1 %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse)' -S <%s | FileCheck -check-prefix=SCHEME1 %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-paropt-map-loop-bind-teams-to-distribute=false -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck -check-prefixes=SCHEME0,NON-CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -vpo-paropt-map-loop-bind-teams-to-distribute=false -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck -check-prefixes=SCHEME0,NON-CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-paropt-map-loop-bind-teams-to-distribute=true -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck -check-prefixes=SCHEME0,CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -vpo-paropt-map-loop-bind-teams-to-distribute=true -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck -check-prefixes=SCHEME0,CONFM %s

; Test src:
;
; void foo() {
;   int a[100], n;
; // case 1
; #pragma omp loop bind(thread) // -> SIMD
;   for (int i = 0; i < 100; i++)
;     a[i] = n;
;
; // case 2
; #pragma omp loop bind(parallel) // -> FOR
;   for (int i = 0; i < 100; i++)
;     a[i] = n;
;
; // case 3
; #pragma omp loop bind(teams) // -> DISTRIBUTE PARALLEL FOR or DISTRIBUTE
;   for (int i = 0; i < 100; i++)
;     a[i] = n;
; }

; Similar test to generic_loop_livein_clause.ll but with the input clauses being TYPED.
;
; case 1: GenericLoop --> SIMD; check that
;    a. SHARED:TYPED --> LIVEIN (untyped)
;    b. FIRSTPRIVATE:TYPED --> LIVEIN (untyped)
;    c. variables are renamed with OPERAND.ADDR (scheme0)
;
; case 2: GenericLoop --> FOR; check that
;    a. SHARED:TYPED --> LIVEIN (untyped)
;    b. FIRSTPRIVATE:TYPED --> unchanged
;    c. variables are renamed with OPERAND.ADDR (scheme0)
;
; case 3: GenericLoop --> DISTRIBUTE PARALLEL FOR (scheme1 or scheme0+non-conforming) or DISTRIBUTE (scheme0+conforming); check that
;    a. SHARED:TYPED --> unchanged (scheme1 or scheme0+non-conforming) or clauses removed (scheme0+conforming)
;    b. FIRSTPRIVATE:TYPED --> unchanged
;    c. variables are renamed with OPERAND.ADDR (scheme0)

; SCHEME1: "DIR.OMP.SIMD"(), "QUAL.OMP.LIVEIN"(ptr %a), "QUAL.OMP.LIVEIN"(ptr %n), {{.*}}, "QUAL.OMP.LIVEIN"(ptr %.omp.lb)
; SCHEME1: "DIR.OMP.LOOP"(), "QUAL.OMP.LIVEIN"(ptr %a), "QUAL.OMP.LIVEIN"(ptr %n), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1)
; SCHEME1: "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100), "QUAL.OMP.SHARED:TYPED"(ptr %n, i32 0, i32 1), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb22, i32 0, i32 1)

; SCHEME0: "DIR.OMP.SIMD"(), "QUAL.OMP.LIVEIN"(ptr %a), "QUAL.OMP.LIVEIN"(ptr %n), {{.*}}, "QUAL.OMP.LIVEIN"(ptr %.omp.lb), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb, ptr %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a, ptr %a.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %n, ptr %n.addr)
; SCHEME0: "DIR.OMP.LOOP"(), "QUAL.OMP.LIVEIN"(ptr %a), "QUAL.OMP.LIVEIN"(ptr %n), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb4, ptr %.omp.lb4.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a, ptr %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(ptr %n, ptr %n.addr{{[0-9]*}})
; NON-CONFM: "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100), "QUAL.OMP.SHARED:TYPED"(ptr %n, i32 0, i32 1), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb22, i32 0, i32 1), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb22, ptr %.omp.lb22.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a, ptr %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(ptr %n, ptr %n.addr{{[0-9]*}})
; CONFM: "DIR.OMP.DISTRIBUTE"(), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv21, i32 0), "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb22, i32 0, i32 1), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb22, ptr %.omp.lb22.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a, ptr %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(ptr %n, ptr %n.addr{{[0-9]*}})

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
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %array.begin = getelementptr inbounds [100 x i32], ptr %a, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.THREAD"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100),
    "QUAL.OMP.SHARED:TYPED"(ptr %n, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

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
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load i32, ptr %n, align 4
  %6 = load i32, ptr %i, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 %idxprom
  store i32 %5, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %7 = load i32, ptr %.omp.iv, align 4
  %add1 = add nsw i32 %7, 1
  store i32 %add1, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]
  store i32 0, ptr %.omp.lb4, align 4
  store i32 99, ptr %.omp.ub5, align 4
  %array.begin19 = getelementptr inbounds [100 x i32], ptr %a, i32 0, i32 0
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100),
    "QUAL.OMP.SHARED:TYPED"(ptr %n, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv3, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb4, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub5, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i9, i32 0, i32 1) ]

  %9 = load i32, ptr %.omp.lb4, align 4
  store i32 %9, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc15, %omp.loop.exit
  %10 = load i32, ptr %.omp.iv3, align 4
  %11 = load i32, ptr %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %10, %11
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end17

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %12 = load i32, ptr %.omp.iv3, align 4
  %mul10 = mul nsw i32 %12, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, ptr %i9, align 4
  %13 = load i32, ptr %n, align 4
  %14 = load i32, ptr %i9, align 4
  %idxprom12 = sext i32 %14 to i64
  %arrayidx13 = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 %idxprom12
  store i32 %13, ptr %arrayidx13, align 4
  br label %omp.body.continue14

omp.body.continue14:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc15

omp.inner.for.inc15:                              ; preds = %omp.body.continue14
  %15 = load i32, ptr %.omp.iv3, align 4
  %add16 = add nsw i32 %15, 1
  store i32 %add16, ptr %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end17:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit18

omp.loop.exit18:                                  ; preds = %omp.inner.for.end17
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.GENERICLOOP"() ]
  store i32 0, ptr %.omp.lb22, align 4
  store i32 99, ptr %.omp.ub23, align 4
  %array.begin37 = getelementptr inbounds [100 x i32], ptr %a, i32 0, i32 0
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100),
    "QUAL.OMP.SHARED:TYPED"(ptr %n, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv21, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb22, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub23, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i27, i32 0, i32 1) ]

  %17 = load i32, ptr %.omp.lb22, align 4
  store i32 %17, ptr %.omp.iv21, align 4
  br label %omp.inner.for.cond24

omp.inner.for.cond24:                             ; preds = %omp.inner.for.inc33, %omp.loop.exit18
  %18 = load i32, ptr %.omp.iv21, align 4
  %19 = load i32, ptr %.omp.ub23, align 4
  %cmp25 = icmp sle i32 %18, %19
  br i1 %cmp25, label %omp.inner.for.body26, label %omp.inner.for.end35

omp.inner.for.body26:                             ; preds = %omp.inner.for.cond24
  %20 = load i32, ptr %.omp.iv21, align 4
  %mul28 = mul nsw i32 %20, 1
  %add29 = add nsw i32 0, %mul28
  store i32 %add29, ptr %i27, align 4
  %21 = load i32, ptr %n, align 4
  %22 = load i32, ptr %i27, align 4
  %idxprom30 = sext i32 %22 to i64
  %arrayidx31 = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 %idxprom30
  store i32 %21, ptr %arrayidx31, align 4
  br label %omp.body.continue32

omp.body.continue32:                              ; preds = %omp.inner.for.body26
  br label %omp.inner.for.inc33

omp.inner.for.inc33:                              ; preds = %omp.body.continue32
  %23 = load i32, ptr %.omp.iv21, align 4
  %add34 = add nsw i32 %23, 1
  store i32 %add34, ptr %.omp.iv21, align 4
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
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
