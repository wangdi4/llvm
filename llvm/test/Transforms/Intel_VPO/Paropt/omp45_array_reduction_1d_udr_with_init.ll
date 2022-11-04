; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL


;
; static const int N = 100;
;
; typedef struct my_struct{int a; int b;} TYPE;
; bool operator <(const TYPE& t1, const TYPE& t2) { return (t1.a < t2.a) || (t1.b < t2.b); }
;
; void my_init(TYPE& t) { t.a = 1; t.b = 1; }
; void my_add(TYPE& lhs, TYPE const &rhs) { lhs.a += rhs.a; lhs.b += rhs.b; }
;
; static TYPE y[N];
; static TYPE x[N];
;
; #pragma omp declare reduction (my_reduction_add : TYPE : my_add(omp_out, omp_in)) initializer(my_init(omp_priv))
;
; // Reduction on:         Array
; // Reduction type:       UDR (with initializer)
; // Array layout:         1-D
; // Access in body:       EXPR_SUBSCRIPT
; void cq415166_1d_c() {
;
; #pragma omp parallel for reduction(my_reduction_add:y) num_threads(N)
;     for (int i = 0; i < N; i++) {
;         for (int j = 5; j <= 8; j++) {
;             my_add(y[j], x[i]);
;         }
;     }
; }
;
; int main() {
;
;     for (int i = 0; i < N; i++) {
;         x[i].a = i; x[i].b = i * i;
;         y[i].a = 0; y[i].b = 0;
;     }
;
;     cq415166_1d_c();
;
;     return 0;
; }


; ModuleID = 'omp45_array_reduction_1d_udr_with_init.cpp'
source_filename = "omp45_array_reduction_1d_udr_with_init.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i32, i32 }

@_ZL1y_028756d5d789e2cd5db53a1fdc9e18de = internal global [100 x %struct.my_struct] zeroinitializer, align 16
@_ZL1x_028756d5d789e2cd5db53a1fdc9e18de = internal global [100 x %struct.my_struct] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local zeroext i1 @_ZltRK9my_structS1_(%struct.my_struct* dereferenceable(8) %t1, %struct.my_struct* dereferenceable(8) %t2) #0 {
entry:
  %t1.addr = alloca %struct.my_struct*, align 8
  %t2.addr = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %t1, %struct.my_struct** %t1.addr, align 8
  store %struct.my_struct* %t2, %struct.my_struct** %t2.addr, align 8
  %0 = load %struct.my_struct*, %struct.my_struct** %t1.addr, align 8
  %a = getelementptr inbounds %struct.my_struct, %struct.my_struct* %0, i32 0, i32 0
  %1 = load i32, i32* %a, align 4
  %2 = load %struct.my_struct*, %struct.my_struct** %t2.addr, align 8
  %a1 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %2, i32 0, i32 0
  %3 = load i32, i32* %a1, align 4
  %cmp = icmp slt i32 %1, %3
  br i1 %cmp, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %entry
  %4 = load %struct.my_struct*, %struct.my_struct** %t1.addr, align 8
  %b = getelementptr inbounds %struct.my_struct, %struct.my_struct* %4, i32 0, i32 1
  %5 = load i32, i32* %b, align 4
  %6 = load %struct.my_struct*, %struct.my_struct** %t2.addr, align 8
  %b2 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %6, i32 0, i32 1
  %7 = load i32, i32* %b2, align 4
  %cmp3 = icmp slt i32 %5, %7
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %entry
  %8 = phi i1 [ true, %entry ], [ %cmp3, %lor.rhs ]
  ret i1 %8
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z7my_initR9my_struct(%struct.my_struct* dereferenceable(8) %t) #0 {
entry:
  %t.addr = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %t, %struct.my_struct** %t.addr, align 8
  %0 = load %struct.my_struct*, %struct.my_struct** %t.addr, align 8
  %a = getelementptr inbounds %struct.my_struct, %struct.my_struct* %0, i32 0, i32 0
  store i32 1, i32* %a, align 4
  %1 = load %struct.my_struct*, %struct.my_struct** %t.addr, align 8
  %b = getelementptr inbounds %struct.my_struct, %struct.my_struct* %1, i32 0, i32 1
  store i32 1, i32* %b, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z6my_addR9my_structRKS_(%struct.my_struct* dereferenceable(8) %lhs, %struct.my_struct* dereferenceable(8) %rhs) #0 {
entry:
  %lhs.addr = alloca %struct.my_struct*, align 8
  %rhs.addr = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %lhs, %struct.my_struct** %lhs.addr, align 8
  store %struct.my_struct* %rhs, %struct.my_struct** %rhs.addr, align 8
  %0 = load %struct.my_struct*, %struct.my_struct** %rhs.addr, align 8
  %a = getelementptr inbounds %struct.my_struct, %struct.my_struct* %0, i32 0, i32 0
  %1 = load i32, i32* %a, align 4
  %2 = load %struct.my_struct*, %struct.my_struct** %lhs.addr, align 8
  %a1 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %2, i32 0, i32 0
  %3 = load i32, i32* %a1, align 4
  %add = add nsw i32 %3, %1
  store i32 %add, i32* %a1, align 4
  %4 = load %struct.my_struct*, %struct.my_struct** %rhs.addr, align 8
  %b = getelementptr inbounds %struct.my_struct, %struct.my_struct* %4, i32 0, i32 1
  %5 = load i32, i32* %b, align 4
  %6 = load %struct.my_struct*, %struct.my_struct** %lhs.addr, align 8
  %b2 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %6, i32 0, i32 1
  %7 = load i32, i32* %b2, align 4
  %add3 = add nsw i32 %7, %5
  store i32 %add3, i32* %b2, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z13cq415166_1d_cv() #1 {
entry:
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.UDR"([100 x %struct.my_struct]* @_ZL1y_028756d5d789e2cd5db53a1fdc9e18de, i8* null, void ([100 x %struct.my_struct]*)* @_ZTSA100_9my_struct.omp.destr, void (%struct.my_struct*, %struct.my_struct*)* @.omp_combiner., void (%struct.my_struct*, %struct.my_struct*)* @.omp_initializer.), "QUAL.OMP.NUM_THREADS"(i32 100), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"([100 x %struct.my_struct]* @_ZL1x_028756d5d789e2cd5db53a1fdc9e18de) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.UDR"
; ALL: red.init.body{{.*}}:
; ALL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.init.body{{.*}} ]
; ALL: call void @.omp_initializer.(%struct.my_struct* %{{.*}}, %struct.my_struct* %{{.*}})
; ALL: br i1 %red.cpy.done{{.*}}, label %red.init.done{{.*}}, label %red.init.body{{.*}}

; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: red.update.body{{.*}}:
; CRITICAL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; CRITICAL: call void @.omp_combiner.(%struct.my_struct* %{{.*}}, %struct.my_struct* %{{.*}})
; CRITICAL: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; CRITICAL: call void @_ZTSA100_9my_struct.omp.destr([100 x %struct.my_struct]* %{{.*}})

; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: red.update.body{{.*}}:
; FASTRED-DAG: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; FASTRED-DAG: call void @.omp_combiner.(%struct.my_struct* %{{.*}}, %struct.my_struct* %{{.*}})
; FASTRED-DAG: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED-DAG: call void @_ZTSA100_9my_struct.omp.destr([100 x %struct.my_struct]* %{{.*}})

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
  store i32 5, i32* %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %5 = load i32, i32* %j, align 4
  %cmp1 = icmp sle i32 %5, 8
  br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %6 = load i32, i32* %j, align 4
  %idxprom = sext i32 %6 to i64
  %arrayidx = getelementptr inbounds [100 x %struct.my_struct], [100 x %struct.my_struct]* @_ZL1y_028756d5d789e2cd5db53a1fdc9e18de, i64 0, i64 %idxprom
  %7 = load i32, i32* %i, align 4
  %idxprom2 = sext i32 %7 to i64
  %arrayidx3 = getelementptr inbounds [100 x %struct.my_struct], [100 x %struct.my_struct]* @_ZL1x_028756d5d789e2cd5db53a1fdc9e18de, i64 0, i64 %idxprom2
  call void @_Z6my_addR9my_structRKS_(%struct.my_struct* dereferenceable(8) %arrayidx, %struct.my_struct* dereferenceable(8) %arrayidx3) #2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, i32* %j, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32* %j, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %9 = load i32, i32* %.omp.iv, align 4
  %add4 = add nsw i32 %9, 1
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
define internal void @.omp_combiner.(%struct.my_struct* noalias %0, %struct.my_struct* noalias %1) #3 {
entry:
  %.addr = alloca %struct.my_struct*, align 8
  %.addr1 = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %0, %struct.my_struct** %.addr, align 8
  store %struct.my_struct* %1, %struct.my_struct** %.addr1, align 8
  %2 = load %struct.my_struct*, %struct.my_struct** %.addr1, align 8
  %3 = load %struct.my_struct*, %struct.my_struct** %.addr, align 8
  call void @_Z6my_addR9my_structRKS_(%struct.my_struct* dereferenceable(8) %3, %struct.my_struct* dereferenceable(8) %2)
  ret void
}

; Function Attrs: noinline uwtable
define internal void @.omp_initializer.(%struct.my_struct* noalias %0, %struct.my_struct* noalias %1) #3 {
entry:
  %.addr = alloca %struct.my_struct*, align 8
  %.addr1 = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %0, %struct.my_struct** %.addr, align 8
  store %struct.my_struct* %1, %struct.my_struct** %.addr1, align 8
  %2 = load %struct.my_struct*, %struct.my_struct** %.addr1, align 8
  %3 = load %struct.my_struct*, %struct.my_struct** %.addr, align 8
  call void @_Z7my_initR9my_struct(%struct.my_struct* dereferenceable(8) %3)
  ret void
}

; Function Attrs: noinline uwtable
define internal void @_ZTSA100_9my_struct.omp.destr([100 x %struct.my_struct]* %0) #3 section ".text.startup" {
entry:
  %.addr = alloca [100 x %struct.my_struct]*, align 8
  store [100 x %struct.my_struct]* %0, [100 x %struct.my_struct]** %.addr, align 8
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #4 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %2 = load i32, i32* %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [100 x %struct.my_struct], [100 x %struct.my_struct]* @_ZL1x_028756d5d789e2cd5db53a1fdc9e18de, i64 0, i64 %idxprom
  %a = getelementptr inbounds %struct.my_struct, %struct.my_struct* %arrayidx, i32 0, i32 0
  store i32 %1, i32* %a, align 8
  %3 = load i32, i32* %i, align 4
  %4 = load i32, i32* %i, align 4
  %mul = mul nsw i32 %3, %4
  %5 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %5 to i64
  %arrayidx2 = getelementptr inbounds [100 x %struct.my_struct], [100 x %struct.my_struct]* @_ZL1x_028756d5d789e2cd5db53a1fdc9e18de, i64 0, i64 %idxprom1
  %b = getelementptr inbounds %struct.my_struct, %struct.my_struct* %arrayidx2, i32 0, i32 1
  store i32 %mul, i32* %b, align 4
  %6 = load i32, i32* %i, align 4
  %idxprom3 = sext i32 %6 to i64
  %arrayidx4 = getelementptr inbounds [100 x %struct.my_struct], [100 x %struct.my_struct]* @_ZL1y_028756d5d789e2cd5db53a1fdc9e18de, i64 0, i64 %idxprom3
  %a5 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %arrayidx4, i32 0, i32 0
  store i32 0, i32* %a5, align 8
  %7 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %7 to i64
  %arrayidx7 = getelementptr inbounds [100 x %struct.my_struct], [100 x %struct.my_struct]* @_ZL1y_028756d5d789e2cd5db53a1fdc9e18de, i64 0, i64 %idxprom6
  %b8 = getelementptr inbounds %struct.my_struct, %struct.my_struct* %arrayidx7, i32 0, i32 1
  store i32 0, i32* %b8, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, i32* %i, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @_Z13cq415166_1d_cv()
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
