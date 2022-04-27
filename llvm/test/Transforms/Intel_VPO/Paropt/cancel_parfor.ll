; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; #include <stdio.h>
;
; int i = 0, j = 0, y = 0;
;
; void foo() {
;   #pragma omp parallel for schedule(dynamic) reduction(+:y)
;   for (j = 0; j < 10; j++) {
;     #pragma omp cancellation point for
;     printf("j = %d\n", j);
;     #pragma omp cancel for
;     y++;
;   }
;   printf("y = %d\n", y);
; }

; ModuleID = 'cancel_parfor.c'
source_filename = "cancel_parfor.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = global i32 0, align 4
@j = global i32 0, align 4
@y = global i32 0, align 4
@.str = private unnamed_addr constant [8 x i8] c"j = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"y = %d\0A\00", align 1

; Function Attrs: nounwind uwtable
define void @foo() #0 {
entry:
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 9, i32* %.omp.ub, align 4, !tbaa !2
  %3 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !2
  %4 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !2

; CHECK-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; CHECK-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.SCHEDULE.DYNAMIC"(i32 1), "QUAL.OMP.REDUCTION.ADD"(i32* @y), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* @j) ]
  %6 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store i32 %6, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %8 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sle i32 %7, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %9, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* @j, align 4, !tbaa !2
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(), "QUAL.OMP.CANCEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellationpoint
; CHECK: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; CHECK: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; CHECK: br i1 [[CHECK1]], label %[[FOREXITLABEL:[a-zA-Z._0-9]+split]], label %{{[a-zA-Z._0-9]+}}

  %11 = load i32, i32* @j, align 4, !tbaa !2
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 %11)
  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(), "QUAL.OMP.CANCEL.LOOP"() ]
  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancel
; CHECK: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 2)
; CHECK: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; CHECK: br i1 [[CHECK2]], label %[[FOREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  %13 = load i32, i32* @y, align 4, !tbaa !2
  %inc = add nsw i32 %13, 1
  store i32 %inc, i32* @y, align 4, !tbaa !2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %14 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add1 = add nsw i32 %14, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %15 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #2
  %16 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %16) #2
  %17 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #2
  %18 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #2
  %19 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #2
  %20 = load i32, i32* @y, align 4, !tbaa !2
  %call2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 %20)
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare i32 @printf(i8*, ...) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 6.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"int", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
