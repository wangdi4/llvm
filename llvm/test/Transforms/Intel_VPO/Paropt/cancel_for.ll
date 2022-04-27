; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL
;
; #include <stdio.h>
;
; int i = 0, j = 0, y = 0;
;
; void foo() {
;   #pragma omp parallel
;   {
;     printf("i = %d\n", i++);
;
;     #pragma omp barrier
;
;     #pragma omp for schedule(static) reduction(+:y)
;     for (j = 0; j < 10; j++) {
;       #pragma omp cancellation point for
;       printf("j = %d\n", j);
;       #pragma omp cancel for if (0)
;       y++;
;     }
;   }
;   printf("y = %d\n", y);
; }

; ModuleID = 'cancel_for.c'
source_filename = "cancel_for.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = global i32 0, align 4
@j = global i32 0, align 4
@y = global i32 0, align 4
@.str = private unnamed_addr constant [8 x i8] c"i = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"j = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"y = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define void @foo() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
; TFORM-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; TFORM-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.SHARED"(i32* @i), "QUAL.OMP.SHARED"(i32* @j), "QUAL.OMP.SHARED"(i32* @y), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last) ]
; #pragma omp parallel
; TFORM: call void {{.+}} @__kmpc_fork_call

  %1 = load i32, i32* @i, align 4, !tbaa !2
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* @i, align 4, !tbaa !2
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 %1)
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.BARRIER"() ]
; #pragma omp barrier (should still be kmpc_barrier, not kmpc_cancel_barrier)
; ALL: call void @__kmpc_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})

  %3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #1
  %4 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %5 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #1
  store i32 9, i32* %.omp.ub, align 4, !tbaa !2
  %6 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #1
  store i32 1, i32* %.omp.stride, align 4, !tbaa !2
  %7 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #1
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !2
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.SCHEDULE.STATIC"(i32 0), "QUAL.OMP.REDUCTION.ADD"(i32* @y), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* @j) ]
; Updated region entry intrinsic after vpo-paropt-prepare
; PREPR: %{{[a-zA-Z._0-9]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"()
; PREPR-SAME: "QUAL.OMP.CANCELLATION.POINTS"(i32* [[CP1ALLOCA:%[a-zA-Z._0-9]+]], i32* [[CP3ALLOCA:%[a-zA-Z._0-9]+]], i32* [[CP2ALLOCA:%[a-zA-Z._0-9]+]])

  %9 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store i32 %9, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %11 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sle i32 %10, %11
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* @j, align 4, !tbaa !2
  %13 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(), "QUAL.OMP.CANCEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %13) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellationpoint
; ALL: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; PREPR-NEXT: store i32 [[CANCEL1]], i32* [[CP1ALLOCA]]
; TFORM: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; TFORM: br i1 [[CHECK1]], label %[[FOREXITLABEL:[a-zA-Z._0-9]+_crit_edge]], label %{{[a-zA-Z._0-9]+}}

  %14 = load i32, i32* @j, align 4, !tbaa !2
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 %14)
  %if.val = trunc i32 %14 to i1
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(), "QUAL.OMP.CANCEL.LOOP"(), "QUAL.OMP.IF"(i1 %if.val) ]
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancel
; ALL: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; PREPR-NEXT: store i32 [[CANCEL2]], i32* [[CP2ALLOCA]]
; TFORM: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; TFORM: br i1 [[CHECK2]], label %[[FOREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

; Cancellation point for if(cancel_expr) {kmpc_cancel()}; else {kmpc_cancellationpoint()}
; ALL: [[CP2:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; PREPR-NEXT: store i32 [[CP2]], i32* [[CP3ALLOCA]]
; TFORM: [[CHECK3:%cancel.check[0-9]*]] = icmp ne i32 [[CP2]], 0
; TFORM: br i1 [[CHECK3]], label %[[FOREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  %16 = load i32, i32* @y, align 4, !tbaa !2
  %inc2 = add nsw i32 %16, 1
  store i32 %inc2, i32* @y, align 4, !tbaa !2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %17 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add3 = add nsw i32 %17, 1
  store i32 %add3, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.LOOP"() ]
  %18 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #1
  %19 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #1
  %20 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #1
  %21 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #1
  %22 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %22) #1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  %23 = load i32, i32* @y, align 4, !tbaa !2
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.2, i32 0, i32 0), i32 %23)
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare i32 @printf(i8*, ...) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #3

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
