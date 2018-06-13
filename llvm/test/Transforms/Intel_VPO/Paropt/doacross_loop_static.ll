; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s -check-prefix=TFORM -check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S < %s | FileCheck %s -check-prefix=ALL
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)'  -S | FileCheck %s -check-prefix=ALL

; #include <stdio.h>
;
; void foo (int (*v_ptr)[5][4])
; {
;   int i, j;
;
;   #pragma omp for ordered (2) private (j) schedule(static)
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

; Function Attrs: nounwind uwtable
define dso_local void @foo([5 x [4 x i32]]* %v_ptr) #0 {
entry:
  %v_ptr.addr = alloca [5 x [4 x i32]]*, align 8
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %tmp1 = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %.omp.stride = alloca i32, align 4
  %.omp.is_last = alloca i32, align 4
  store [5 x [4 x i32]]* %v_ptr, [5 x [4 x i32]]** %v_ptr.addr, align 8, !tbaa !2
  %0 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  %2 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  %3 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %3) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !6
  %4 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %4) #2
  store i32 7, i32* %.omp.ub, align 4, !tbaa !6
  %5 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %5) #2
  store i32 1, i32* %.omp.stride, align 4, !tbaa !6
  %6 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %6) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !6
; TFORM-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; TFORM-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}

  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.ORDERED"(i32 2), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SCHEDULE.STATIC"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
; #pragma omp for ordered(2) schedule(static)
; TFORM: call void @__kmpc_doacross_init({ i32, i32, i32, i32, i8* }* @{{[a-zA-Z._0-9]*}}, i32 %[[TID:[a-zA-Z._0-9]*]], i32 1, { i64, i64, i64 }* %{{[a-zA-Z._0-9]*}})
; TFORM-NEXT:  call void @__kmpc_for_static_init_4({ i32, i32, i32, i32, i8* }* @{{[a-zA-Z._0-9]*}}, i32 %[[TID]], i32 34, i32* %is.last, i32* %lower.bnd, i32* %upper.bnd, i32* %stride, i32 1, i32 1)

  %8 = load i32, i32* %.omp.lb, align 4, !tbaa !6
  store i32 %8, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %9 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %10 = load i32, i32* %.omp.ub, align 4, !tbaa !6
  %cmp = icmp sle i32 %9, %10
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %11 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %div = sdiv i32 %11, 2
  %mul = mul nsw i32 %div, 1
  %add = add nsw i32 1, %mul
  store i32 %add, i32* %i, align 4, !tbaa !6
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %rem = srem i32 %12, 2
  %mul2 = mul nsw i32 %rem, 1
  %add3 = add nsw i32 2, %mul2
  store i32 %add3, i32* %j, align 4, !tbaa !6
  %13 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %sub = sub nsw i32 %13, 1
  %sub4 = sub nsw i32 %sub, 2
  %14 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %sub5 = sub nsw i32 %14, 2
  %15 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(), "QUAL.OMP.DEPEND.SINK"(i32 %sub4), "QUAL.OMP.DEPEND.SINK"(i32 %sub5) ]
  call void @llvm.directive.region.exit(token %15) [ "DIR.OMP.END.ORDERED"() ]
; ALL: call void @__kmpc_doacross_wait({ i32, i32, i32, i32, i8* }* @{{[a-zA-Z._0-9]*}}, i32 %{{[a-zA-Z._0-9]*}}, i64* %{{[a-zA-Z._0-9]*}})
; ALL: call void @__kmpc_doacross_wait({ i32, i32, i32, i32, i8* }* @{{[a-zA-Z._0-9]*}}, i32 %{{[a-zA-Z._0-9]*}}, i64* %{{[a-zA-Z._0-9]*}})
  %16 = load [5 x [4 x i32]]*, [5 x [4 x i32]]** %v_ptr.addr, align 8, !tbaa !2
  %17 = load i32, i32* %i, align 4, !tbaa !6
  %sub6 = sub nsw i32 %17, 1
  %idxprom = sext i32 %sub6 to i64
  %arrayidx = getelementptr inbounds [5 x [4 x i32]], [5 x [4 x i32]]* %16, i64 0, i64 %idxprom
  %18 = load i32, i32* %j, align 4, !tbaa !6
  %sub7 = sub nsw i32 %18, 1
  %idxprom8 = sext i32 %sub7 to i64
  %arrayidx9 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx, i64 0, i64 %idxprom8
  %19 = load i32, i32* %arrayidx9, align 4, !tbaa !8
  %20 = load [5 x [4 x i32]]*, [5 x [4 x i32]]** %v_ptr.addr, align 8, !tbaa !2
  %21 = load i32, i32* %i, align 4, !tbaa !6
  %idxprom10 = sext i32 %21 to i64
  %arrayidx11 = getelementptr inbounds [5 x [4 x i32]], [5 x [4 x i32]]* %20, i64 0, i64 %idxprom10
  %22 = load i32, i32* %j, align 4, !tbaa !6
  %sub12 = sub nsw i32 %22, 2
  %idxprom13 = sext i32 %sub12 to i64
  %arrayidx14 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx11, i64 0, i64 %idxprom13
  %23 = load i32, i32* %arrayidx14, align 4, !tbaa !8
  %add15 = add nsw i32 %19, %23
  %24 = load [5 x [4 x i32]]*, [5 x [4 x i32]]** %v_ptr.addr, align 8, !tbaa !2
  %25 = load i32, i32* %i, align 4, !tbaa !6
  %idxprom16 = sext i32 %25 to i64
  %arrayidx17 = getelementptr inbounds [5 x [4 x i32]], [5 x [4 x i32]]* %24, i64 0, i64 %idxprom16
  %26 = load i32, i32* %j, align 4, !tbaa !6
  %idxprom18 = sext i32 %26 to i64
  %arrayidx19 = getelementptr inbounds [4 x i32], [4 x i32]* %arrayidx17, i64 0, i64 %idxprom18
  store i32 %add15, i32* %arrayidx19, align 4, !tbaa !8
  %27 = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(), "QUAL.OMP.DEPEND.SOURCE"() ]
  call void @llvm.directive.region.exit(token %27) [ "DIR.OMP.END.ORDERED"() ]
; ALL: call void @__kmpc_doacross_post({ i32, i32, i32, i32, i8* }* @{{[a-zA-Z._0-9]*}}, i32 %{{[a-zA-Z._0-9]*}}, i64* %{{[a-zA-Z._0-9]*}})
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %28 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %add20 = add nsw i32 %28, 1
  store i32 %add20, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.LOOP"() ]
;TFORM: call void @__kmpc_for_static_fini({ i32, i32, i32, i32, i8* }* @{{[a-zA-Z._0-9]*}}, i32 %[[TID]])
;TFORM-NEXT: call void @__kmpc_doacross_fini({ i32, i32, i32, i32, i8* }* @{{[a-zA-Z._0-9]*}}, i32 %[[TID]])
  %29 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %29) #2
  %30 = bitcast i32* %.omp.stride to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %30) #2
  %31 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %31) #2
  %32 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %32) #2
  %33 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %33) #2
  %34 = bitcast i32* %j to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %34) #2
  %35 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %35) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang ffaaa2e50b66dde2113bde1a427bdc17d68d2fe9) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 92a24020d54c6b1d0f6865765d777f88c2fc1c39)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPA5_A4_i", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
!8 = !{!9, !7, i64 0}
!9 = !{!"array@_ZTSA5_A4_i", !10, i64 0}
!10 = !{!"array@_ZTSA4_i", !7, i64 0}
