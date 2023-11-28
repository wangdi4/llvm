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
; // Reduction on:         Pointee of Pointer to Array.
; // Reduction type:       UDR (no initializer)
; // Array section layout: 1-D
; // Access in body:       EXPR_SUBSCRIPT
; void cq415166_1d_e_ptr() {
;
;     TYPE (*yptr)[N] = &y;
;
; #pragma omp parallel for reduction(my_reduction_add:yptr[0][25:N/4])
;     for(int i = 0; i < N; i++) {
;         my_add(yptr[0][25 + i/4], x[i]);
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
;     cq415166_1d_e_ptr();
;
;     return 0;
; }

; The test IR was hand-modified to use a constant section size/offset for
; reduction. CFE currently generates IR instructions to compute them.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL1y_04ad2d3e2cff23fc26c501e82cae0492 = internal global [100 x i16] zeroinitializer, align 16
@_ZL1x_04ad2d3e2cff23fc26c501e82cae0492 = internal global [100 x i16] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z6my_addRsRKs(ptr dereferenceable(2) %lhs, ptr dereferenceable(2) %rhs) #0 {
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

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z17cq415166_1d_e_ptrv() #1 {
entry:
  %yptr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store ptr @_ZL1y_04ad2d3e2cff23fc26c501e82cae0492, ptr %yptr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:ARRSECT.PTR_TO_PTR.TYPED"(ptr %yptr, i16 0, i64 25, i64 25, ptr null, ptr null, ptr @.omp_combiner., ptr null),
    "QUAL.OMP.SHARED:TYPED"(ptr @_ZL1x_04ad2d3e2cff23fc26c501e82cae0492, i16 0, i64 100),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1) ]

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4

; ALL-NOT: "QUAL.OMP.REDUCTION.UDR"
; ALL: red.init.body{{.*}}:
; ALL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.init.body{{.*}} ]
; ALL: store {{.*}} 0, {{.*}}
; ALL: br i1 %red.cpy.done{{.*}}, label %red.init.done{{.*}}, label %red.init.body{{.*}}

; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: red.update.body{{.*}}:
; CRITICAL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; CRITICAL: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; CRITICAL: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; CRITICAL: call void @__kmpc_end_critical({{.*}})

; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: red.update.body{{.*}}:
; FASTRED-DAG: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; FASTRED-DAG: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; FASTRED-DAG: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})

  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %entry
  %2 = load i32, ptr %.omp.iv, align 4
  %3 = load i32, ptr %.omp.ub, align 4
  %cmp = icmp sle i32 %2, %3
  br i1 %cmp, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %4 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %4, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %5 = load ptr, ptr %yptr, align 8
  %arrayidx = getelementptr inbounds [100 x i16], ptr %5, i64 0
  %6 = load i32, ptr %i, align 4
  %div = sdiv i32 %6, 4
  %add1 = add nsw i32 25, %div
  %idxprom = sext i32 %add1 to i64
  %arrayidx2 = getelementptr inbounds [100 x i16], ptr %arrayidx, i64 0, i64 %idxprom
  %7 = load i32, ptr %i, align 4
  %idxprom3 = sext i32 %7 to i64
  %arrayidx4 = getelementptr inbounds [100 x i16], ptr @_ZL1x_04ad2d3e2cff23fc26c501e82cae0492, i64 0, i64 %idxprom3
  call void @_Z6my_addRsRKs(ptr dereferenceable(2) %arrayidx2, ptr dereferenceable(2) %arrayidx4) #2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %8, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: noinline uwtable
define internal void @.omp_combiner.(ptr noalias %0, ptr noalias %1) #3 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @_Z6my_addRsRKs(ptr dereferenceable(2) %3, ptr dereferenceable(2) %2)
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main(i32 %arc, ptr %argv) #4 {
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
  %arrayidx = getelementptr inbounds [100 x i16], ptr @_ZL1x_04ad2d3e2cff23fc26c501e82cae0492, i64 0, i64 %idxprom
  store i16 1, ptr %arrayidx, align 2
  %2 = load i32, ptr %i, align 4
  %conv = trunc i32 %2 to i16
  %3 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %3 to i64
  %arrayidx2 = getelementptr inbounds [100 x i16], ptr @_ZL1y_04ad2d3e2cff23fc26c501e82cae0492, i64 0, i64 %idxprom1
  store i16 %conv, ptr %arrayidx2, align 2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, ptr %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @_Z17cq415166_1d_e_ptrv()
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
