; RUN: opt -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -simplifycfg -S %s | FileCheck %s
; RUN: opt -passes='function(loop(loop-rotate),vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,sroa,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt,function(simplifycfg)' -S %s | FileCheck %s

; Verify correctness of CFG and run-time calls.

; Original code:
; #define LOOP() for (int i = 0; i < 1000; ++i)
; void for_ordered(int *x)
; {
; #pragma omp for ordered
;   LOOP();
; }

; CHECK-LABEL: @_Z11for_orderedPi

; CHECK: call i32 @__kmpc_global_thread_num(

; ZTT block:
; CHECK: %[[ZTT1:.+]] = icmp sle i32 %[[LB:[^,]+]], %[[UB:[^,]+]]
; CHECK: br i1 %[[ZTT1]], label %[[PHB:[^,]+]], label %[[EXIT:[^,]+]]

; PHB:
; CHECK: [[PHB]]:
; 'schedule' 66 is kmp_ord_static
; CHECK: call void @__kmpc_dispatch_init_4({{.*}}, i32 %[[TID:[^,]+]], i32 66, i32 %[[DISPLB:[^,]+]], i32 %[[DISPUB:[^,]+]], i32 1, i32 1)
; CHECK: br label %[[DISPNEXT:[^,]+]]

; DISPNEXT:
; CHECK: [[DISPNEXT]]:
; CHECK: %[[NEXT:.+]] = call i32 @__kmpc_dispatch_next_4({{.*}}, i32 %[[TID]], i32* %[[PISLAST:[^,]+]], i32* %[[PLB:[^,]+]], i32* %[[PUB:[^,]+]], i32* %[[PST:[^,]+]])
; CHECK: %[[NEXTP:.+]] = icmp ne i32 %[[NEXT]], 0
; CHECK: br i1 %[[NEXTP]], label %[[DISPB:[^,]+]], label %[[BARRIER:[^,]+]]

; DISPB:
; CHECK: [[DISPB]]:
; CHECK: %[[LBNEW:.+]] = load i32, i32* %[[PLB]]
; CHECK: %[[UBNEW:.+]] = load i32, i32* %[[PUB]]
; CHECK: %[[ZTT2:.+]] = icmp sle i32 %[[LBNEW]], %[[UBNEW]]
; CHECK: br i1 %[[ZTT2]], label %[[LOOPBODY:[^,]+]], label %[[BARRIER:[^,]+]]

; LOOPBODY:
; CHECK: [[LOOPBODY]]:
; CHECK: call void @__kmpc_dispatch_fini_4({{.*}}, i32 %[[TID]])
; CHECK: br i1 {{.*}}, label %[[LOOPBODY]], label %[[DISPNEXT:[^,]+]]

; BARRIER:
; CHECK: [[BARRIER]]:
; CHECK: call void @__kmpc_barrier
; CHECK: br label %[[EXIT]]

; EXIT:
; CHECK: [[EXIT]]:

; ModuleID = 'dist_cases.cpp'
source_filename = "dist_cases.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @_Z11for_orderedPi(i32* %x) #0 {
entry:
  %x.addr = alloca i32*, align 8
  %.omp.iv = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i32* %x, i32** %x.addr, align 8, !tbaa !2
  %0 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 0, i32* %.omp.lb, align 4, !tbaa !6
  %2 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %2) #2
  store i32 999, i32* %.omp.ub, align 4, !tbaa !6
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(), "QUAL.OMP.ORDERED"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i) ]
  %4 = load i32, i32* %.omp.lb, align 4, !tbaa !6
  store i32 %4, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %5 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %6 = load i32, i32* %.omp.ub, align 4, !tbaa !6
  %cmp = icmp sle i32 %5, %6
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %7 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %7) #2
  %8 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !6
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %9 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %9) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !6
  %add1 = add nsw i32 %10, 1
  store i32 %add1, i32* %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.LOOP"() ]
  %11 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %11) #2
  %12 = bitcast i32* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %12) #2
  %13 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %13) #2
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

attributes #0 = { nounwind uwtable "may-have-openmp-directive"="true" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}
