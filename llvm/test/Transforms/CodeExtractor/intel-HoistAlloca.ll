; RUN: opt < %s -passes="vpo-paropt" -S | FileCheck  %s
; RUN: opt -passes="vpo-paropt" -vpo-paropt-use-empty-code-extractor-analysis-cache=true -S %s | FileCheck %s --check-prefixes=EMPTY_CEAC_TRUE,CHECK
; RUN: opt -passes="vpo-paropt" -vpo-paropt-use-empty-code-extractor-analysis-cache=false -S %s | FileCheck %s --check-prefixes=EMPTY_CEAC_FALSE,CHECK
;
; It checks whether the alloca instruction has been hoisted to the entry
; of outline OMP function.
;
; void foo(int *p, int *q, int m) {
;   int i, j;
;   #pragma omp parallel for private(j)
;   for (j = 0;j < m; j++) {
;     hoist = alloca(4)
;     p[j] = q[j] + 1;
;   }
; }

target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define dso_local void @foo(ptr %p, ptr %q, i32 %m) local_unnamed_addr #0 {
entry:
  %p.addr = alloca ptr, align 8
  %q.addr = alloca ptr, align 8
  %j = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store ptr %p, ptr %p.addr, align 8, !tbaa !2
  store ptr %q, ptr %q.addr, align 8, !tbaa !2
  %0 = bitcast ptr %j to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %0) #2
  %1 = bitcast ptr %.omp.iv to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %1) #2
  %cmp = icmp sgt i32 %m, 0
  br i1 %cmp, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %entry
  %sub2 = add nsw i32 %m, -1
  %2 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %2) #2
  store i32 0, ptr %.omp.lb, align 4, !tbaa !6
  %3 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr %3) #2
  store i32 %sub2, ptr %.omp.ub, align 4, !tbaa !6
  br label %DIR.OMP.PARALLEL.LOOP.1

DIR.OMP.PARALLEL.LOOP.1:                          ; preds = %omp.precond.then
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.SHARED:TYPED"(ptr %q.addr, ptr null, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %p.addr, ptr null, i32 1) ]
  br label %DIR.OMP.PARALLEL.LOOP.116

DIR.OMP.PARALLEL.LOOP.116:                        ; preds = %DIR.OMP.PARALLEL.LOOP.1
  %5 = load i32, ptr %.omp.lb, align 4, !tbaa !6
  store volatile i32 %5, ptr %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %DIR.OMP.PARALLEL.LOOP.116
  %6 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !6
  %7 = load i32, ptr %.omp.ub, align 4, !tbaa !6
  %cmp4 = icmp sgt i32 %6, %7
  br i1 %cmp4, label %omp.loop.exit, label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !6
  %hoist = alloca i32
  store i32 %8, ptr %j, align 4, !tbaa !6
  %9 = load ptr, ptr %q.addr, align 8, !tbaa !2
  %idxprom = sext i32 %8 to i64
  %arrayidx = getelementptr inbounds i32, ptr %9, i64 %idxprom
  %10 = load i32, ptr %arrayidx, align 4, !tbaa !6
  %add6 = add nsw i32 %10, 1
  %11 = load ptr, ptr %p.addr, align 8, !tbaa !2
  %arrayidx8 = getelementptr inbounds i32, ptr %11, i64 %idxprom
  store i32 %add6, ptr %arrayidx8, align 4, !tbaa !6
  %12 = load volatile i32, ptr %.omp.iv, align 4, !tbaa !6
  %add9 = add nsw i32 %12, 1
  store volatile i32 %add9, ptr %.omp.iv, align 4, !tbaa !6
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %entry
  %13 = bitcast ptr %.omp.ub to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %13) #2
  %14 = bitcast ptr %.omp.lb to ptr
  call void @llvm.lifetime.end.p0(i64 4, ptr %14) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %1) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %0) #2
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c9398a48781aea8daf5a7c290e5cd112a269e163) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm e2262133fb438acf8927dc6874ce16c472145a3f)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPi", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !4, i64 0}

; EMPTY_CEAC_TRUE: define dso_local void @foo{{.*}}
; EMPTY_CEAC_TRUE: entry
; EMPTY_CEAC_TRUE: %j = alloca{{.*}}
; CHECK-LABEL: define internal void @foo.DIR.OMP.PARALLEL.LOOP{{.*}}
; EMPTY_CEAC_FALSE: %j = alloca{{.*}}
; CHECK: %j.priv = alloca{{.*}}
; CHECK: %hoist = alloca
; CHECK: DIR.OMP.PARALLEL.LOOP{{.*}}:
