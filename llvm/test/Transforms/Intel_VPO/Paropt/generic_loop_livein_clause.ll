; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-loop-collapse -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck -check-prefix=SCHEME1 %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-loop-collapse,vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck -check-prefix=SCHEME1 %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-paropt-map-loop-bind-teams-to-distribute=false -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck -check-prefixes=SCHEME0,NON-CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -vpo-paropt-map-loop-bind-teams-to-distribute=false -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck -check-prefixes=SCHEME0,NON-CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -bugpoint-enable-legacy-pm -vpo-paropt-map-loop-bind-teams-to-distribute=true -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck -check-prefixes=SCHEME0,CONFM %s
; RUN: opt -vpo-paropt-loop-mapping-scheme=0 -vpo-paropt-map-loop-bind-teams-to-distribute=true -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck -check-prefixes=SCHEME0,CONFM %s

; Test src:
;
; void foo() {
;   int a[100], n, m;
; // case 1
; #pragma omp loop bind(thread) // -> SIMD
;   for (int i = 0; i < 100; i++)
;     a[i] = n + m;
;
; // case 2
; #pragma omp loop bind(parallel) // -> FOR
;   for (int i = 0; i < 100; i++)
;     a[i] = n;
;
; // case 3
; #pragma omp loop bind(teams) // -> DISTRIBUTE PARALLEL FOR (scheme1, scheme0+nonconforming) or DISTRIBUTE (scheme0+conforming)
;   for (int i = 0; i < 100; i++)
;     a[i] = n;
; }

; case 1: GenericLoop --> SIMD; check that
;    a. SHARED --> LIVEIN
;    b. FIRSTPRIVATE --> LIVEIN
;    c. variables are renamed with OPERAND.ADDR (scheme0)
;
; case 2: GenericLoop --> FOR; check that
;    a. SHARED --> LIVEIN
;    b. FIRSTPRIVATE --> unchanged
;    c. variables are renamed with OPERAND.ADDR (scheme0)
;
; case 3: GenericLoop --> DISTRIBUTE PARALLEL FOR (scheme1 or scheme0+nonconforming) or DISTRIBUTE (scheme0+conforming); check that
;    a. SHARED --> unchanged (scheme1 or scheme0+nonconforming) or clause removed (scheme0+conforming)
;    b. FIRSTPRIVATE --> unchanged
;    c. variables are renamed with OPERAND.ADDR (scheme0)

; SCHEME1: "DIR.OMP.SIMD"(), "QUAL.OMP.LIVEIN"(ptr %a), "QUAL.OMP.LIVEIN"(ptr %m), "QUAL.OMP.LIVEIN"(ptr %n), {{.*}}, "QUAL.OMP.LIVEIN"(ptr %.omp.lb)
; SCHEME1: "DIR.OMP.LOOP"(), "QUAL.OMP.LIVEIN"(ptr %a), "QUAL.OMP.LIVEIN"(ptr %n), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb5, i32 0, i32 1)
; SCHEME1: "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100), "QUAL.OMP.SHARED:TYPED"(ptr %n, i32 0, i32 1), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb23, i32 0, i32 1)

; SCHEME0: "DIR.OMP.SIMD"(), "QUAL.OMP.LIVEIN"(ptr %a), "QUAL.OMP.LIVEIN"(ptr %m), "QUAL.OMP.LIVEIN"(ptr %n), {{.*}}, "QUAL.OMP.LIVEIN"(ptr %.omp.lb), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb, ptr %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a, ptr %a.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %m, ptr %m.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %n, ptr %n.addr)
; SCHEME0: "DIR.OMP.LOOP"(), "QUAL.OMP.LIVEIN"(ptr %a), "QUAL.OMP.LIVEIN"(ptr %n), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb5, i32 0, i32 1), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb5, ptr %.omp.lb5.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a, ptr %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(ptr %n, ptr %n.addr{{[0-9]*}})
; NON-CONFM: "DIR.OMP.DISTRIBUTE.PARLOOP"(), "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100), "QUAL.OMP.SHARED:TYPED"(ptr %n, i32 0, i32 1), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb23, i32 0, i32 1), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb23, ptr %.omp.lb23.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a, ptr %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(ptr %n, ptr %n.addr{{[0-9]*}})
; CONFM: "DIR.OMP.DISTRIBUTE"(), {{.*}}, "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb23, i32 0, i32 1), {{.*}}, {{.*}}, {{.*}}, "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb23, ptr %.omp.lb23.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %a, ptr %a.addr{{[0-9]*}}), "QUAL.OMP.OPERAND.ADDR"(ptr %n, ptr %n.addr{{[0-9]*}})

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
  %tmp21 = alloca i32, align 4
  %.omp.iv22 = alloca i32, align 4
  %.omp.lb23 = alloca i32, align 4
  %.omp.ub24 = alloca i32, align 4
  %i28 = alloca i32, align 4
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %array.begin = getelementptr inbounds [100 x i32], ptr %a, i32 0, i32 0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.THREAD"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100),
    "QUAL.OMP.SHARED:TYPED"(ptr %m, i32 0, i32 1),
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
  %6 = load i32, ptr %m, align 4
  %add1 = add nsw i32 %5, %6
  %7 = load i32, ptr %i, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 %idxprom
  store i32 %add1, ptr %arrayidx, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add2 = add nsw i32 %8, 1
  store i32 %add2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.GENERICLOOP"() ]
  store i32 0, ptr %.omp.lb5, align 4
  store i32 99, ptr %.omp.ub6, align 4
  %array.begin20 = getelementptr inbounds [100 x i32], ptr %a, i32 0, i32 0
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.PARALLEL"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100),
    "QUAL.OMP.SHARED:TYPED"(ptr %n, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv4, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb5, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub6, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i10, i32 0, i32 1) ]

  %10 = load i32, ptr %.omp.lb5, align 4
  store i32 %10, ptr %.omp.iv4, align 4
  br label %omp.inner.for.cond7

omp.inner.for.cond7:                              ; preds = %omp.inner.for.inc16, %omp.loop.exit
  %11 = load i32, ptr %.omp.iv4, align 4
  %12 = load i32, ptr %.omp.ub6, align 4
  %cmp8 = icmp sle i32 %11, %12
  br i1 %cmp8, label %omp.inner.for.body9, label %omp.inner.for.end18

omp.inner.for.body9:                              ; preds = %omp.inner.for.cond7
  %13 = load i32, ptr %.omp.iv4, align 4
  %mul11 = mul nsw i32 %13, 1
  %add12 = add nsw i32 0, %mul11
  store i32 %add12, ptr %i10, align 4
  %14 = load i32, ptr %n, align 4
  %15 = load i32, ptr %i10, align 4
  %idxprom13 = sext i32 %15 to i64
  %arrayidx14 = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 %idxprom13
  store i32 %14, ptr %arrayidx14, align 4
  br label %omp.body.continue15

omp.body.continue15:                              ; preds = %omp.inner.for.body9
  br label %omp.inner.for.inc16

omp.inner.for.inc16:                              ; preds = %omp.body.continue15
  %16 = load i32, ptr %.omp.iv4, align 4
  %add17 = add nsw i32 %16, 1
  store i32 %add17, ptr %.omp.iv4, align 4
  br label %omp.inner.for.cond7

omp.inner.for.end18:                              ; preds = %omp.inner.for.cond7
  br label %omp.loop.exit19

omp.loop.exit19:                                  ; preds = %omp.inner.for.end18
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.GENERICLOOP"() ]
  store i32 0, ptr %.omp.lb23, align 4
  store i32 99, ptr %.omp.ub24, align 4
  %array.begin38 = getelementptr inbounds [100 x i32], ptr %a, i32 0, i32 0
  %17 = call token @llvm.directive.region.entry() [ "DIR.OMP.GENERICLOOP"(),
    "QUAL.OMP.BIND.TEAMS"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %a, i32 0, i64 100),
    "QUAL.OMP.SHARED:TYPED"(ptr %n, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv22, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb23, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub24, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i28, i32 0, i32 1) ]

  %18 = load i32, ptr %.omp.lb23, align 4
  store i32 %18, ptr %.omp.iv22, align 4
  br label %omp.inner.for.cond25

omp.inner.for.cond25:                             ; preds = %omp.inner.for.inc34, %omp.loop.exit19
  %19 = load i32, ptr %.omp.iv22, align 4
  %20 = load i32, ptr %.omp.ub24, align 4
  %cmp26 = icmp sle i32 %19, %20
  br i1 %cmp26, label %omp.inner.for.body27, label %omp.inner.for.end36

omp.inner.for.body27:                             ; preds = %omp.inner.for.cond25
  %21 = load i32, ptr %.omp.iv22, align 4
  %mul29 = mul nsw i32 %21, 1
  %add30 = add nsw i32 0, %mul29
  store i32 %add30, ptr %i28, align 4
  %22 = load i32, ptr %n, align 4
  %23 = load i32, ptr %i28, align 4
  %idxprom31 = sext i32 %23 to i64
  %arrayidx32 = getelementptr inbounds [100 x i32], ptr %a, i64 0, i64 %idxprom31
  store i32 %22, ptr %arrayidx32, align 4
  br label %omp.body.continue33

omp.body.continue33:                              ; preds = %omp.inner.for.body27
  br label %omp.inner.for.inc34

omp.inner.for.inc34:                              ; preds = %omp.body.continue33
  %24 = load i32, ptr %.omp.iv22, align 4
  %add35 = add nsw i32 %24, 1
  store i32 %add35, ptr %.omp.iv22, align 4
  br label %omp.inner.for.cond25

omp.inner.for.end36:                              ; preds = %omp.inner.for.cond25
  br label %omp.loop.exit37

omp.loop.exit37:                                  ; preds = %omp.inner.for.end36
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
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
