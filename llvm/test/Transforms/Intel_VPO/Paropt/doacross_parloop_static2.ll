; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=ALL

; #include <stdio.h>
;
; void foo (int (*v_ptr)[5][4])
; {
;   int i, j;
;
;   #pragma omp parallel for ordered (2) private (j) schedule(static, 2)
;   for (i = 1; i < 5; i++) {
;     for (j = 2; j < 4; j++) {
;
;       #pragma omp ordered depend(sink: i-1, j-1) depend(sink: i, j-2)
;       (*v_ptr) [i][j] = (*v_ptr) [i-1][j - 1] + (*v_ptr) [i][j-2];
;
;       #pragma omp ordered depend(source)
;     }
;   }
; }

; ModuleID = 'doacross_test.c'
source_filename = "doacross_test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo([5 x [4 x i32]]* %v_ptr) #0 {
entry:
  %v_ptr.addr = alloca [5 x [4 x i32]]*, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  store [5 x [4 x i32]]* %v_ptr, [5 x [4 x i32]]** %v_ptr.addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 3, i32* %.omp.ub, align 4
  store i32 1, i32* %.omp.stride, align 4
  store i32 0, i32* %.omp.is_last, align 4
; TFORM-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; TFORM-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.ORDERED"(i32 2, i32 4, i32 2), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SCHEDULE.STATIC"(i32 2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([5 x [4 x i32]]** %v_ptr.addr) ]
; #pragma omp parallel for ordered(2) schedule(static,2)
; TFORM: call void @__kmpc_doacross_init({{[^,]+}}, i32 %[[TID:[a-zA-Z._0-9]+]], i32 2, i8* %{{[a-zA-Z._0-9]+}})
; TFORM-NEXT: call void @__kmpc_for_static_init_4({{[^,]+}}, i32 %[[TID]], i32 33, i32* %is.last, i32* %lower.bnd, i32* %upper.bnd, i32* %stride, i32 1, i32 2)
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
  %add = add nsw i32 1, %mul
  store i32 %add, i32* %i, align 4
  store i32 2, i32* %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %5 = load i32, i32* %j, align 4
  %cmp1 = icmp slt i32 %5, 4
  br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %6 = load i32, i32* %i, align 4
  %sub = sub nsw i32 %6, 1
  %sub2 = sub nsw i32 %sub, 1
  %div = sdiv i32 %sub2, 1
  %7 = load i32, i32* %j, align 4
  %sub3 = sub nsw i32 %7, 1
  %sub4 = sub nsw i32 %sub3, 2
  %div5 = sdiv i32 %sub4, 1
  %8 = load i32, i32* %i, align 4
  %sub6 = sub nsw i32 %8, 1
  %div7 = sdiv i32 %sub6, 1
  %9 = load i32, i32* %j, align 4
  %sub8 = sub nsw i32 %9, 2
  %sub9 = sub nsw i32 %sub8, 2
  %div10 = sdiv i32 %sub9, 1
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(), "QUAL.OMP.DEPEND.SINK"(i32 %div, i32 %div5), "QUAL.OMP.DEPEND.SINK"(i32 %div7, i32 %div10) ]
; ALL-DAG: call void @__kmpc_doacross_wait({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i8* %{{[a-zA-Z._0-9]+}})
; ALL-DAG: call void @__kmpc_doacross_wait({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i8* %{{[a-zA-Z._0-9]+}})
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.ORDERED"() ]
  %11 = load [5 x [4 x i32]]*, [5 x [4 x i32]]** %v_ptr.addr, align 8
  %12 = load i32, i32* %i, align 4
  %sub11 = sub nsw i32 %12, 1
  %idxprom = sext i32 %sub11 to i64
  %arrayidx = getelementptr inbounds [5 x [4 x i32]], [5 x [4 x i32]]* %11, i64 0, i64 %idxprom
  %13 = load i32, i32* %j, align 4
  %sub12 = sub nsw i32 %13, 1
  %idxprom13 = sext i32 %sub12 to i64
  %arrayidx14 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx, i64 0, i64 %idxprom13
  %14 = load i32, i32* %arrayidx14, align 4
  %15 = load [5 x [4 x i32]]*, [5 x [4 x i32]]** %v_ptr.addr, align 8
  %16 = load i32, i32* %i, align 4
  %idxprom15 = sext i32 %16 to i64
  %arrayidx16 = getelementptr inbounds [5 x [4 x i32]], [5 x [4 x i32]]* %15, i64 0, i64 %idxprom15
  %17 = load i32, i32* %j, align 4
  %sub17 = sub nsw i32 %17, 2
  %idxprom18 = sext i32 %sub17 to i64
  %arrayidx19 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx16, i64 0, i64 %idxprom18
  %18 = load i32, i32* %arrayidx19, align 4
  %add20 = add nsw i32 %14, %18
  %19 = load [5 x [4 x i32]]*, [5 x [4 x i32]]** %v_ptr.addr, align 8
  %20 = load i32, i32* %i, align 4
  %idxprom21 = sext i32 %20 to i64
  %arrayidx22 = getelementptr inbounds [5 x [4 x i32]], [5 x [4 x i32]]* %19, i64 0, i64 %idxprom21
  %21 = load i32, i32* %j, align 4
  %idxprom23 = sext i32 %21 to i64
  %arrayidx24 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx22, i64 0, i64 %idxprom23
  store i32 %add20, i32* %arrayidx24, align 4
  %22 = load i32, i32* %i, align 4
  %sub25 = sub nsw i32 %22, 1
  %div26 = sdiv i32 %sub25, 1
  %23 = load i32, i32* %j, align 4
  %sub27 = sub nsw i32 %23, 2
  %div28 = sdiv i32 %sub27, 1
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(), "QUAL.OMP.DEPEND.SOURCE"(i32 %div26, i32 %div28) ]
; ALL-DAG: call void @__kmpc_doacross_post({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i8* %{{[a-zA-Z._0-9]+}})
  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.ORDERED"() ]
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %25 = load i32, i32* %j, align 4
  %inc = add nsw i32 %25, 1
  store i32 %inc, i32* %j, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %26 = load i32, i32* %.omp.iv, align 4
  %add29 = add nsw i32 %26, 1
  store i32 %add29, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
; TFORM-DAG: call void @__kmpc_for_static_fini({{[^,]+}}, i32 %[[TID]])
; TFORM-DAG: call void @__kmpc_doacross_fini({{[^,]+}}, i32 %[[TID]])
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
