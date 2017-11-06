; RUN: opt < %s -domtree -loops  -lcssa-verification  -loop-rotate -vpo-cfg-restructuring -vpo-paropt-prepare -simplifycfg  -sroa  -loops -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s


; This file tests the implemention of omp taskloop reduction clause.
; void foo() {
; int a;
; #pragma omp taskloop reduction(+:a)
; for (int i = 0; i < 10; ++i);
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo() #0 {
entry:
  %a = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %.omp.stride = alloca i64, align 8
  %.omp.is_last = alloca i32, align 4
  %i = alloca i32, align 4
  %0 = bitcast i32* %a to i8*
  call void @llvm.lifetime.start(i64 4, i8* %0) #2
  %1 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start(i64 4, i8* %1) #2
  %2 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start(i64 8, i8* %2) #2
  store i64 0, i64* %.omp.lb, align 8, !tbaa !1
  %3 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start(i64 8, i8* %3) #2
  store i64 9, i64* %.omp.ub, align 8, !tbaa !1
  %4 = bitcast i64* %.omp.stride to i8*
  call void @llvm.lifetime.start(i64 8, i8* %4) #2
  store i64 1, i64* %.omp.stride, align 8, !tbaa !1
  %5 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.start(i64 4, i8* %5) #2
  store i32 0, i32* %.omp.is_last, align 4, !tbaa !5
  call void @llvm.intel.directive(metadata !"DIR.OMP.TASKLOOP")
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.REDUCTION.ADD", i32* %a)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i64* %.omp.lb)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.FIRSTPRIVATE", i64* %.omp.ub)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i32* %i)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %6 = load i64, i64* %.omp.lb, align 8, !tbaa !1
  %conv = trunc i64 %6 to i32
  store i32 %conv, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %7 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %conv1 = sext i32 %7 to i64
  %8 = load i64, i64* %.omp.ub, align 8, !tbaa !1
  %cmp = icmp ule i64 %conv1, %8
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %9 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start(i64 4, i8* %9) #2
  %10 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %mul = mul nsw i32 %10, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4, !tbaa !5
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  %11 = bitcast i32* %i to i8*
  call void @llvm.lifetime.end(i64 4, i8* %11) #2
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, i32* %.omp.iv, align 4, !tbaa !5
  %add3 = add nsw i32 %12, 1
  store i32 %add3, i32* %.omp.iv, align 4, !tbaa !5
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.TASKLOOP")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  %13 = bitcast i32* %.omp.is_last to i8*
  call void @llvm.lifetime.end(i64 4, i8* %13) #2
  %14 = bitcast i64* %.omp.stride to i8*
  call void @llvm.lifetime.end(i64 8, i8* %14) #2
  %15 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end(i64 8, i8* %15) #2
  %16 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end(i64 8, i8* %16) #2
  %17 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end(i64 4, i8* %17) #2
  %18 = bitcast i32* %a to i8*
  call void @llvm.lifetime.end(i64 4, i8* %18) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21308)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !3, i64 0}

; CHECK:  %{{.*}} = call i8* @__kmpc_task_reduction_init({{.*}})
; CHECK:  %{{.*}} = call i8* @__kmpc_task_reduction_get_th_data({{.*}})
