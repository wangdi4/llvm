; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL -check-prefix=CRITICAL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL -check-prefix=CRITICAL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL -check-prefix=FASTRED
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL -check-prefix=FASTRED
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=PREPR -check-prefix=ALL

; #include <stdio.h>
;
; int i = 0, j = 0, x = 0;
;
; void foo() {
;   #pragma omp parallel reduction(+:x)
;   {
;     #pragma omp single
;     printf("i = %d\n", i++);
;     #pragma omp cancel parallel
;     #pragma omp barrier
;
;     #pragma omp for
;     for (j = 0; j < 10; j++) {
;       printf("j = %d\n", j);
;     }
;     #pragma omp cancellation point parallel
;     x++;
;   }
;   printf("x = %d\n", x);
; }

; ModuleID = 'cancel_par.c'
source_filename = "cancel_par.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@i = global i32 0, align 4
@j = global i32 0, align 4
@x = global i32 0, align 4
@.str = private unnamed_addr constant [8 x i8] c"i = %d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"j = %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1

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

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.REDUCTION.ADD"(i32* @x), "QUAL.OMP.SHARED"(i32* @i), "QUAL.OMP.SHARED"(i32* @j), "QUAL.OMP.PRIVATE"(i32* %.omp.lb), "QUAL.OMP.PRIVATE"(i32* %.omp.stride), "QUAL.OMP.PRIVATE"(i32* %.omp.is_last) ]
; #pragma omp parallel
; Updated region exit intrinsic after vpo-paropt-prepare
; PREPR: %{{[a-zA-Z._0-9]+}} = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; PREPR-SAME: "QUAL.OMP.CANCELLATION.POINTS"(i32* [[CP1ALLOCA:%[a-zA-Z._0-9]+]], i32* [[CP2ALLOCA:%[a-zA-Z._0-9]+]], i32* [[CP3ALLOCA:%[a-zA-Z._0-9]+]], i32* [[CP4ALLOCA:%[a-zA-Z._0-9]+]])
; TFORM: call void {{.+}} @__kmpc_fork_call

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SINGLE"() ]
  %2 = load i32, i32* @i, align 4, !tbaa !2
  %inc = add nsw i32 %2, 1
  store i32 %inc, i32* @i, align 4, !tbaa !2
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 %2)
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SINGLE"() ]
; #pragama omp single
; ALL: %{{[a-zA-Z._0-9]+}} = call i32 @__kmpc_single({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; ALL  call void @__kmpc_end_single({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; Implicit barrier for single
; ALL: [[CBARRIER1:%[0-9]+]] = call i32 @__kmpc_cancel_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; PREPR-NEXT: store i32 [[CBARRIER1]], i32* [[CP1ALLOCA]]
; TFORM-NOT: store i32 [[CBARRIER1]], i32* {{%[0-9]+}}
; TFORM: [[CHECK1:%cancel.check[0-9]*]] = icmp ne i32 [[CBARRIER1]], 0
; TFORM: br i1 [[CHECK1]], label %[[PAREXITLABEL:[a-zA-Z._0-9]+]], label %{{[a-zA-Z._0-9]+}}

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCEL"(), "QUAL.OMP.CANCEL.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.CANCEL"() ]
; #pragma omp cancel
; ALL: [[CANCEL1:%[0-9]+]] = call i32 @__kmpc_cancel({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 1)
; PREPR-NEXT: store i32 [[CANCEL1]], i32* [[CP2ALLOCA]]
; TFORM-NOT: store i32 [[CANCEL1]], i32* {{%[0-9]+}}
; TFORM: [[CHECK2:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL1]], 0
; TFORM: br i1 [[CHECK2]], label %[[PAREXITLABEL1:[a-zA-Z._0-9]+\.split_crit_edge]], label %{{[a-zA-Z._0-9]+}}

; Exit label for cancel/cancellationpoint with cancel_barrier
; TFORM: [[PAREXITLABEL1]]:{{.*}}
; TFORM: [[CBARRIER4:%[0-9]+]] = call i32 @__kmpc_cancel_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; TFORM-NEXT: br label %[[PAREXITLABEL]]

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.BARRIER"() ]
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.BARRIER"() ]
; #pragma omp barrier
; ALL: [[CBARRIER2:%[0-9]+]] = call i32 @__kmpc_cancel_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; PREPR-NEXT: store i32 [[CBARRIER2]], i32* [[CP3ALLOCA]]
; TFORM-NOT: store i32 [[CBARRIER2]], i32* {{%[0-9]+}}
; TFORM-NEXT: [[CHECK3:%cancel.check[0-9]*]] = icmp ne i32 [[CBARRIER2]], 0
; TFORM-NEXT: br i1 [[CHECK3]], label %[[PAREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  %5 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #1
  %6 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #1
  store i32 0, i32* %.omp.lb, align 4, !tbaa !2
  %7 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #1
  store i32 9, i32* %.omp.ub, align 4, !tbaa !2
  %8 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %8) #1
  store i32 1, i32* %.omp.stride, align 4, !tbaa !2
  %9 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %9) #1
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !2
  %10 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* @j) ]
; #pragma omp for
; TFORM:  call void @__kmpc_for_static_init_4({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}}, i32 34, i32* %is.last, i32* %lower.bnd, i32* %upper.bnd, i32* %stride, i32 1, i32 1)

  %11 = load i32, i32* %.omp.lb, align 4, !tbaa !2
  store i32 %11, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %13 = load i32, i32* %.omp.ub, align 4, !tbaa !2
  %cmp = icmp sle i32 %12, %13
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %14 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %mul = mul nsw i32 %14, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* @j, align 4, !tbaa !2
  %15 = load i32, i32* @j, align 4, !tbaa !2
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 %15)
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %16 = load i32, i32* %.omp.iv, align 4, !tbaa !2
  %add2 = add nsw i32 %16, 1
  store i32 %add2, i32* %.omp.iv, align 4, !tbaa !2
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %10) [ "DIR.OMP.END.LOOP"() ]
; TFORM: call void @__kmpc_for_static_fini({{[^,]+}}, i32 %{{[a-zA-Z._0-9]+}})
; Implicit barrier for omp for
; TFORM: [[CBARRIER3:%[0-9]+]] = call i32 @__kmpc_cancel_barrier({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}})
; TFORM-NEXT: [[CHECK4:%cancel.check[0-9]*]] = icmp ne i32 [[CBARRIER3]], 0
; TFORM-NEXT: br i1 [[CHECK4]], label %[[PAREXITLABEL]], label %{{[a-zA-Z._0-9]+}}

  %17 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %17) #1
  %18 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %18) #1
  %19 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %19) #1
  %20 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %20) #1
  %21 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %21) #1
  %22 = call token @llvm.directive.region.entry() [ "DIR.OMP.CANCELLATION.POINT"(), "QUAL.OMP.CANCEL.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %22) [ "DIR.OMP.END.CANCELLATION.POINT"() ]
; #pragma omp cancellation point
; ALL: [[CANCEL2:%[0-9]+]] = call i32 @__kmpc_cancellationpoint({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 1)
; PREPR-NEXT: store i32 [[CANCEL2]], i32* [[CP4ALLOCA]]
; TFORM-NOT: store i32 [[CANCEL2]], i32* {{%[0-9]+}}
; TFORM-NEXT: [[CHECK5:%cancel.check[0-9]*]] = icmp ne i32 [[CANCEL2]], 0
; TFORM-NEXT: br i1 [[CHECK5]], label %[[PAREXITLABEL1]], label %{{[a-zA-Z._0-9]+}}

  %23 = load i32, i32* @x, align 4, !tbaa !2
  %inc3 = add nsw i32 %23, 1
  store i32 %inc3, i32* @x, align 4, !tbaa !2

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
; Reduction Handling
; CRITICAL: call void @__kmpc_critical({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, [8 x i32]*  @{{[a-zA-Z._0-9]*}})
; CRITICAL: call void @__kmpc_end_critical({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, [8 x i32]*  @{{[a-zA-Z._0-9]*}})

; FASTRED: call i32 @__kmpc_reduce({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, i32 {{[0-9]*}}, i32 {{[0-9]*}}, i8* %{{[a-zA-Z._0-9]*}}, void (i8*, i8*)* @{{[a-zA-Z._0-9]*}}, [8 x i32]* @{{[a-zA-Z._0-9]*}})
; FASTRED: call void @__kmpc_end_reduce({{[^,]+}}, i32 %{{[a-zA-Z._0-9]*}}, [8 x i32]*  @{{[a-zA-Z._0-9]*}})

  %24 = load i32, i32* @x, align 4, !tbaa !2
  %call4 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.2, i32 0, i32 0), i32 %24)
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
