; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL

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
; // Reduction on:         Array passed to function
; // Reduction type:       UDR (no initializer)
; // Array layout:         1-D
; // Access in body:       EXPR_SUBSCRIPT
; void cq415166_1d_b_passed(TYPE ypas[N]) {
; #pragma omp parallel for reduction(my_reduction_add:ypas[:N])
;     for(int i = 0; i < N; i++) {
;         my_add(ypas[25 + i/4], x[i]);
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
;     cq415166_1d_b_passed(y);
;
;     return 0;
; }


; ModuleID = 'omp45_array_reduction_1d_udr_passed.cpp'
source_filename = "omp45_array_reduction_1d_udr_passed.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@_ZL1x_e8f1e776114868a0bc900bd0cefb8125 = internal global [100 x i16] zeroinitializer, align 16
@_ZL1y_e8f1e776114868a0bc900bd0cefb8125 = internal global [100 x i16] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z6my_addRsRKs(i16* dereferenceable(2) %lhs, i16* dereferenceable(2) %rhs) #0 {
entry:
  %lhs.addr = alloca i16*, align 8
  %rhs.addr = alloca i16*, align 8
  store i16* %lhs, i16** %lhs.addr, align 8
  store i16* %rhs, i16** %rhs.addr, align 8
  %0 = load i16*, i16** %rhs.addr, align 8
  %1 = load i16, i16* %0, align 2
  %conv = sext i16 %1 to i32
  %2 = load i16*, i16** %lhs.addr, align 8
  %3 = load i16, i16* %2, align 2
  %conv1 = sext i16 %3 to i32
  %add = add nsw i32 %conv1, %conv
  %conv2 = trunc i32 %add to i16
  store i16 %conv2, i16* %2, align 2
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z20cq415166_1d_b_passedPs(i16* %ypas) #1 {
entry:
  %ypas.addr = alloca i16*, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  store i16* %ypas, i16** %ypas.addr, align 8
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.UDR:ARRSECT"(i16** %ypas.addr, i64 1, i64 0, i64 100, i64 1, i8* null, i8* null, void (i16*, i16*)* @.omp_combiner., i8* null), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([100 x i16]* @_ZL1x_e8f1e776114868a0bc900bd0cefb8125) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.UDR"
; ALL: red.init.body{{.*}}:
; ALL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.init.body{{.*}} ]
; ALL: store {{.*}} 0, {{.*}}
; ALL: br i1 %red.cpy.done{{.*}}, label %red.init.done{{.*}}, label %red.init.body{{.*}}

; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: red.update.body{{.*}}:
; CRITICAL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; CRITICAL: call void @.omp_combiner.(i16* %{{.*}}, i16* %{{.*}})
; CRITICAL: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; CRITICAL: call void @__kmpc_end_critical({{.*}})

; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: red.update.body{{.*}}:
; FASTRED-DAG: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; FASTRED-DAG: call void @.omp_combiner.(i16* %{{.*}}, i16* %{{.*}})
; FASTRED-DAG: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})

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
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %5 = load i16*, i16** %ypas.addr, align 8
  %6 = load i32, i32* %i, align 4
  %div = sdiv i32 %6, 4
  %add1 = add nsw i32 25, %div
  %idxprom = sext i32 %add1 to i64
  %arrayidx = getelementptr inbounds i16, i16* %5, i64 %idxprom
  %7 = load i32, i32* %i, align 4
  %idxprom2 = sext i32 %7 to i64
  %arrayidx3 = getelementptr inbounds [100 x i16], [100 x i16]* @_ZL1x_e8f1e776114868a0bc900bd0cefb8125, i64 0, i64 %idxprom2
  call void @_Z6my_addRsRKs(i16* dereferenceable(2) %arrayidx, i16* dereferenceable(2) %arrayidx3) #2
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %8 = load i32, i32* %.omp.iv, align 4
  %add4 = add nsw i32 %8, 1
  store i32 %add4, i32* %.omp.iv, align 4
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
define internal void @.omp_combiner.(i16* noalias %0, i16* noalias %1) #3 {
entry:
  %.addr = alloca i16*, align 8
  %.addr1 = alloca i16*, align 8
  store i16* %0, i16** %.addr, align 8
  store i16* %1, i16** %.addr1, align 8
  %2 = load i16*, i16** %.addr1, align 8
  %3 = load i16*, i16** %.addr, align 8
  call void @_Z6my_addRsRKs(i16* dereferenceable(2) %3, i16* dereferenceable(2) %2)
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main(i32 %arc, i8** %argv) #4 {
entry:
  %retval = alloca i32, align 4
  %arc.addr = alloca i32, align 4
  %argv.addr = alloca i8**, align 8
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 %arc, i32* %arc.addr, align 4
  store i8** %argv, i8*** %argv.addr, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [100 x i16], [100 x i16]* @_ZL1x_e8f1e776114868a0bc900bd0cefb8125, i64 0, i64 %idxprom
  store i16 1, i16* %arrayidx, align 2
  %2 = load i32, i32* %i, align 4
  %conv = trunc i32 %2 to i16
  %3 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %3 to i64
  %arrayidx2 = getelementptr inbounds [100 x i16], [100 x i16]* @_ZL1y_e8f1e776114868a0bc900bd0cefb8125, i64 0, i64 %idxprom1
  store i16 %conv, i16* %arrayidx2, align 2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, i32* %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @_Z20cq415166_1d_b_passedPs(i16* getelementptr inbounds ([100 x i16], [100 x i16]* @_ZL1y_e8f1e776114868a0bc900bd0cefb8125, i64 0, i64 0))
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
