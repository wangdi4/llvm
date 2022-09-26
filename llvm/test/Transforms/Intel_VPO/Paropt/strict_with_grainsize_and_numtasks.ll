; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Check the codegen for GRAINSIZE amd NUM_TASKS w/o STRICT
; The test contains hand-modified IR to add "STRICT" to "QUAL.OMP.GRAINSIZE" and "QUAL.OMP.NUM_TASKS"
;
; Test src:
;
; void foo() {
;  #pragma omp taskloop num_tasks(strict: 10)
;  for (int i = 0; i < 10; ++i);
;
;  #pragma omp taskloop num_tasks(10)
;  for (int i = 0; i < 10; ++i);
;
; #pragma omp taskloop grainsize(strict: 5)
;  for (int i = 0; i < 5; ++i);
;  }
;
; #pragma omp taskloop grainsize(5)
;  for (int i = 0; i < 5; ++i);
;  }

; Check for NumTask with STRICT
; CHECK: call void @__kmpc_taskloop_5(%struct.ident_t* {{.*}}, i32  %{{.*}}, i8*  %{{.*}}, i32 {{.*}}, i64* %{{.*}}, i64* %{{.*}}, i64 %{{.*}}, i32 {{.*}}, i32 2, i64 10, i32 1, i8* {{.*}})

; Check for NumTask without STRICT
; CHECK: call void @__kmpc_taskloop_5(%struct.ident_t* {{.*}}, i32  %{{.*}}, i8*  %{{.*}}, i32 {{.*}}, i64* %{{.*}}, i64* %{{.*}}, i64 %{{.*}}, i32 {{.*}}, i32 2, i64 10, i32 0, i8* {{.*}})

; Check for Grainsize with STRICT
; CHECK: call void @__kmpc_taskloop_5(%struct.ident_t* {{.*}}, i32  %{{.*}}, i8*  %{{.*}}, i32 {{.*}}, i64* %{{.*}}, i64* %{{.*}}, i64 %{{.*}}, i32 {{.*}}, i32 1, i64 5, i32 1, i8* {{.*}})

; Check for Grainsize without STRICT
; CHECK: call void @__kmpc_taskloop_5(%struct.ident_t* {{.*}}, i32  %{{.*}}, i8*  %{{.*}}, i32 {{.*}}, i64* %{{.*}}, i64* %{{.*}}, i64 %{{.*}}, i32 {{.*}}, i32 1, i64 5, i32 0, i8* {{.*}})

; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
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
  %tmp17 = alloca i32, align 4
  %.omp.iv18 = alloca i32, align 4
  %.omp.lb19 = alloca i32, align 4
  %.omp.ub20 = alloca i32, align 4
  %i24 = alloca i32, align 4
  %tmp32 = alloca i32, align 4
  %.omp.iv33 = alloca i32, align 4
  %.omp.lb34 = alloca i32, align 4
  %.omp.ub35 = alloca i32, align 4
  %i39 = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.IMPLICIT"() ]
  store i32 0, i32* %.omp.lb, align 4
  store i32 9, i32* %.omp.ub, align 4
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.NUM_TASKS:STRICT"(i32 10),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub),
    "QUAL.OMP.PRIVATE"(i32* %i) ]
  %2 = load i32, i32* %.omp.lb, align 4
  store i32 %2, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, i32* %.omp.iv, align 4
  %4 = load i32, i32* %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %6 = load i32, i32* %.omp.iv, align 4
  %add1 = add nsw i32 %6, 1
  store i32 %add1, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
    "QUAL.OMP.IMPLICIT"() ]
  store i32 0, i32* %.omp.lb4, align 4
  store i32 9, i32* %.omp.ub5, align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
    "QUAL.OMP.NUM_TASKS"(i32 10),
    "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv3),
    "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb4),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub5),
    "QUAL.OMP.PRIVATE"(i32* %i9) ]
  %9 = load i32, i32* %.omp.lb4, align 4
  store i32 %9, i32* %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.cond6:                              ; preds = %omp.inner.for.inc13, %omp.loop.exit
  %10 = load i32, i32* %.omp.iv3, align 4
  %11 = load i32, i32* %.omp.ub5, align 4
  %cmp7 = icmp sle i32 %10, %11
  br i1 %cmp7, label %omp.inner.for.body8, label %omp.inner.for.end15

omp.inner.for.body8:                              ; preds = %omp.inner.for.cond6
  %12 = load i32, i32* %.omp.iv3, align 4
  %mul10 = mul nsw i32 %12, 1
  %add11 = add nsw i32 0, %mul10
  store i32 %add11, i32* %i9, align 4
  br label %omp.body.continue12

omp.body.continue12:                              ; preds = %omp.inner.for.body8
  br label %omp.inner.for.inc13

omp.inner.for.inc13:                              ; preds = %omp.body.continue12
  %13 = load i32, i32* %.omp.iv3, align 4
  %add14 = add nsw i32 %13, 1
  store i32 %add14, i32* %.omp.iv3, align 4
  br label %omp.inner.for.cond6

omp.inner.for.end15:                              ; preds = %omp.inner.for.cond6
  br label %omp.loop.exit16

omp.loop.exit16:                                  ; preds = %omp.inner.for.end15
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASKGROUP"() ]
  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
     "QUAL.OMP.IMPLICIT"() ]
  store i32 0, i32* %.omp.lb19, align 4
  store i32 9, i32* %.omp.ub20, align 4
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
     "QUAL.OMP.GRAINSIZE:STRICT"(i32 5),
     "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv18),
     "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb19),
     "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub20),
     "QUAL.OMP.PRIVATE"(i32* %i24) ]
  %16 = load i32, i32* %.omp.lb19, align 4
  store i32 %16, i32* %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.cond21:                             ; preds = %omp.inner.for.inc28, %omp.loop.exit16
  %17 = load i32, i32* %.omp.iv18, align 4
  %18 = load i32, i32* %.omp.ub20, align 4
  %cmp22 = icmp sle i32 %17, %18
  br i1 %cmp22, label %omp.inner.for.body23, label %omp.inner.for.end30

omp.inner.for.body23:                             ; preds = %omp.inner.for.cond21
  %19 = load i32, i32* %.omp.iv18, align 4
  %mul25 = mul nsw i32 %19, 1
  %add26 = add nsw i32 0, %mul25
  store i32 %add26, i32* %i24, align 4
  br label %omp.body.continue27

omp.body.continue27:                              ; preds = %omp.inner.for.body23
  br label %omp.inner.for.inc28

omp.inner.for.inc28:                              ; preds = %omp.body.continue27
  %20 = load i32, i32* %.omp.iv18, align 4
  %add29 = add nsw i32 %20, 1
  store i32 %add29, i32* %.omp.iv18, align 4
  br label %omp.inner.for.cond21

omp.inner.for.end30:                              ; preds = %omp.inner.for.cond21
  br label %omp.loop.exit31

omp.loop.exit31:                                  ; preds = %omp.inner.for.end30
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.TASKGROUP"() ]
  %21 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"(),
     "QUAL.OMP.IMPLICIT"() ]
  store i32 0, i32* %.omp.lb34, align 4
  store i32 9, i32* %.omp.ub35, align 4
  %22 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKLOOP"(),
     "QUAL.OMP.GRAINSIZE"(i32 5),
     "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv33),
     "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb34),
     "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub35),
     "QUAL.OMP.PRIVATE"(i32* %i39) ]
  %23 = load i32, i32* %.omp.lb34, align 4
  store i32 %23, i32* %.omp.iv33, align 4
  br label %omp.inner.for.cond36

omp.inner.for.cond36:                             ; preds = %omp.inner.for.inc43, %omp.loop.exit31
  %24 = load i32, i32* %.omp.iv33, align 4
  %25 = load i32, i32* %.omp.ub35, align 4
  %cmp37 = icmp sle i32 %24, %25
  br i1 %cmp37, label %omp.inner.for.body38, label %omp.inner.for.end45

omp.inner.for.body38:                             ; preds = %omp.inner.for.cond36
  %26 = load i32, i32* %.omp.iv33, align 4
  %mul40 = mul nsw i32 %26, 1
  %add41 = add nsw i32 0, %mul40
  store i32 %add41, i32* %i39, align 4
  br label %omp.body.continue42

omp.body.continue42:                              ; preds = %omp.inner.for.body38
  br label %omp.inner.for.inc43

omp.inner.for.inc43:                              ; preds = %omp.body.continue42
  %27 = load i32, i32* %.omp.iv33, align 4
  %add44 = add nsw i32 %27, 1
  store i32 %add44, i32* %.omp.iv33, align 4
  br label %omp.inner.for.cond36

omp.inner.for.end45:                              ; preds = %omp.inner.for.cond36
  br label %omp.loop.exit46

omp.loop.exit46:                                  ; preds = %omp.inner.for.end45
  call void @llvm.directive.region.exit(token %22) [ "DIR.OMP.END.TASKLOOP"() ]
  call void @llvm.directive.region.exit(token %21) [ "DIR.OMP.END.TASKGROUP"() ]
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
