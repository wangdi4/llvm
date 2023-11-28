; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL

; Test src:
;
; typedef short TYPE;
;
; static TYPE x[100];
; static TYPE y[100];
; static const int N = 100;
;
; void my_add(TYPE& lhs, TYPE const &rhs) {
;     lhs += rhs;
; }
;
; #pragma omp declare reduction (my_reduction_add : TYPE : my_add(omp_out, omp_in))
;
; // Reduction on:         Reference to Array
; // Reduction type:       UDR (no initializer)
; // Array layout:         1-D
; // Access in body:       EXPR_SUBSCRIPT
; void cq415166_1d_b_ref(TYPE (&yref)[N]) {
; #pragma omp parallel for reduction(my_reduction_add:yref)
;     for(int i = 0; i < N; i++) {
;         my_add(yref[25 + i/4], x[i]);
;     }
; }
;
; int main(int arc, char** argv) {
;     int i;
;
;     for (i = 0; i < N; i++) {
;         x[i] = 1;
;         y[i] = i;
;     }
;
;     cq415166_1d_b_ref(y);
;     return 0;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL1x = internal global [100 x i16] zeroinitializer, align 16
@_ZL1y = internal global [100 x i16] zeroinitializer, align 16

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z6my_addRsRKs(ptr noundef nonnull align 2 dereferenceable(2) %lhs, ptr noundef nonnull align 2 dereferenceable(2) %rhs) #0 {
entry:
  %lhs.addr = alloca ptr, align 8
  %rhs.addr = alloca ptr, align 8
  store ptr %lhs, ptr %lhs.addr, align 8
  store ptr %rhs, ptr %rhs.addr, align 8
  %0 = load ptr, ptr %rhs.addr, align 8
  %1 = load i16, ptr %0, align 2
  %conv = sext i16 %1 to i32
  %2 = load ptr, ptr %lhs.addr, align 8
  %3 = load i16, ptr %2, align 2
  %conv1 = sext i16 %3 to i32
  %add = add nsw i32 %conv1, %conv
  %conv2 = trunc i32 %add to i16
  store i16 %conv2, ptr %2, align 2
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z17cq415166_1d_b_refRA100_s(ptr noundef nonnull align 2 dereferenceable(200) %yref) #1 {
entry:
  %yref.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr %yref, ptr %yref.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = load ptr, ptr %yref.addr, align 8
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:BYREF.TYPED"(ptr %yref.addr, [100 x i16] zeroinitializer, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr null),
    "QUAL.OMP.SHARED:TYPED"(ptr @_ZL1x, i16 0, i64 100),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.UDR"
; ALL: red.init.body{{.*}}:
; ALL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.init.body{{.*}} ]
; ALL: store {{.*}} 0, {{.*}}
; ALL: br i1 %red.cpy.done{{.*}}, label %red.init.done{{.*}}, label %red.init.body{{.*}}
; CRITICAL: call void @__kmpc_critical({{.*}})
; FASTRED: call i32 @__kmpc_reduce({{.*}})
; ALL: red.update.body{{.*}}:
; ALL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; ALL: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; ALL: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; FASTRED: call void @__kmpc_end_reduce({{.*}})

  %2 = load i32, ptr %.omp.lb, align 4
  store i32 %2, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %3 = load i32, ptr %.omp.iv, align 4
  %4 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %3, %4
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %5 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %5, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %6 = load ptr, ptr %yref.addr, align 8
  %7 = load i32, ptr %i, align 4
  %div = sdiv i32 %7, 4
  %add1 = add nsw i32 25, %div
  %idxprom = sext i32 %add1 to i64
  %arrayidx = getelementptr inbounds [100 x i16], ptr %6, i64 0, i64 %idxprom
  %8 = load i32, ptr %i, align 4
  %idxprom2 = sext i32 %8 to i64
  %arrayidx3 = getelementptr inbounds [100 x i16], ptr @_ZL1x, i64 0, i64 %idxprom2
  call void @_Z6my_addRsRKs(ptr noundef nonnull align 2 dereferenceable(2) %arrayidx, ptr noundef nonnull align 2 dereferenceable(2) %arrayidx3) #2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %9, 1
  store i32 %add4, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
define internal void @.omp_combiner.(ptr noalias noundef %0, ptr noalias noundef %1) #3 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @_Z6my_addRsRKs(ptr noundef nonnull align 2 dereferenceable(2) %3, ptr noundef nonnull align 2 dereferenceable(2) %2)
  ret void
}

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main(i32 noundef %arc, ptr noundef %argv) #4 {
entry:
  %retval = alloca i32, align 4
  %arc.addr = alloca i32, align 4
  %argv.addr = alloca ptr, align 8
  %i = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 %arc, ptr %arc.addr, align 4
  store ptr %argv, ptr %argv.addr, align 8
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %0, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [100 x i16], ptr @_ZL1x, i64 0, i64 %idxprom
  store i16 1, ptr %arrayidx, align 2
  %2 = load i32, ptr %i, align 4
  %conv = trunc i32 %2 to i16
  %3 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %3 to i64
  %arrayidx2 = getelementptr inbounds [100 x i16], ptr @_ZL1y, i64 0, i64 %idxprom1
  store i16 %conv, ptr %arrayidx2, align 2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, ptr %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond, !llvm.loop !5

for.end:                                          ; preds = %for.cond
  call void @_Z17cq415166_1d_b_refRA100_s(ptr noundef nonnull align 2 dereferenceable(200) @_ZL1y)
  ret i32 0
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress noinline norecurse nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.mustprogress"}
