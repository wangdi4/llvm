; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL --check-prefix=ALL
; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED --check-prefix=ALL

; Test src:
;
; static const int N = 100;
;
; typedef struct my_struct{int a; int b;} TYPE;
; bool operator <(const TYPE& t1, const TYPE& t2) { return (t1.a < t2.a) || (t1.b < t2.b); }
;
; void my_add(TYPE& lhs, TYPE const &rhs) { lhs.a += rhs.a; lhs.b += rhs.b; }
;
; static TYPE y[N];
; static TYPE x[N];
;
; #pragma omp declare reduction (my_reduction_add : TYPE : my_add(omp_out, omp_in))
;
; // Reduction on:         Array passed to function
; // Reduction type:       UDR (with initializer)
; // Array section layout: 1-D
; // Access in body:       EXPR_SUBSCRIPT
; void cq415166_1d_f_pas(TYPE ypas[N]) {
;
; #pragma omp parallel for reduction(my_reduction_add:ypas[5:4]) num_threads(N)
;     for (int i = 0; i < N; i++) {
;         for (int j = 5; j <= 8; j++) {
;             my_add(ypas[j], x[i]);
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
;     cq415166_1d_f_pas(y);
;     return 0;
; }

; The test IR was hand-modified to use a constant section size/offset for
; reduction. CFE currently generates IR instructions to compute them.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.my_struct = type { i32, i32 }

@_ZL1x_cfb28f0ac03fe018acd8fdbeebb430b5 = internal global [100 x %struct.my_struct] zeroinitializer, align 16
@_ZL1y_cfb28f0ac03fe018acd8fdbeebb430b5 = internal global [100 x %struct.my_struct] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local zeroext i1 @_ZltRK9my_structS1_(ptr dereferenceable(8) %t1, ptr dereferenceable(8) %t2) #0 {
entry:
  %t1.addr = alloca ptr, align 8
  %t2.addr = alloca ptr, align 8
  store ptr %t1, ptr %t1.addr, align 8
  store ptr %t2, ptr %t2.addr, align 8
  %0 = load ptr, ptr %t1.addr, align 8
  %a = getelementptr inbounds %struct.my_struct, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %a, align 4
  %2 = load ptr, ptr %t2.addr, align 8
  %a1 = getelementptr inbounds %struct.my_struct, ptr %2, i32 0, i32 0
  %3 = load i32, ptr %a1, align 4
  %cmp = icmp slt i32 %1, %3
  br i1 %cmp, label %lor.end, label %lor.rhs

lor.rhs:                                          ; preds = %entry
  %4 = load ptr, ptr %t1.addr, align 8
  %b = getelementptr inbounds %struct.my_struct, ptr %4, i32 0, i32 1
  %5 = load i32, ptr %b, align 4
  %6 = load ptr, ptr %t2.addr, align 8
  %b2 = getelementptr inbounds %struct.my_struct, ptr %6, i32 0, i32 1
  %7 = load i32, ptr %b2, align 4
  %cmp3 = icmp slt i32 %5, %7
  br label %lor.end

lor.end:                                          ; preds = %lor.rhs, %entry
  %8 = phi i1 [ true, %entry ], [ %cmp3, %lor.rhs ]
  ret i1 %8
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z6my_addR9my_structRKS_(ptr dereferenceable(8) %lhs, ptr dereferenceable(8) %rhs) #0 {
entry:
  %lhs.addr = alloca ptr, align 8
  %rhs.addr = alloca ptr, align 8
  store ptr %lhs, ptr %lhs.addr, align 8
  store ptr %rhs, ptr %rhs.addr, align 8
  %0 = load ptr, ptr %rhs.addr, align 8
  %a = getelementptr inbounds %struct.my_struct, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %a, align 4
  %2 = load ptr, ptr %lhs.addr, align 8
  %a1 = getelementptr inbounds %struct.my_struct, ptr %2, i32 0, i32 0
  %3 = load i32, ptr %a1, align 4
  %add = add nsw i32 %3, %1
  store i32 %add, ptr %a1, align 4
  %4 = load ptr, ptr %rhs.addr, align 8
  %b = getelementptr inbounds %struct.my_struct, ptr %4, i32 0, i32 1
  %5 = load i32, ptr %b, align 4
  %6 = load ptr, ptr %lhs.addr, align 8
  %b2 = getelementptr inbounds %struct.my_struct, ptr %6, i32 0, i32 1
  %7 = load i32, ptr %b2, align 4
  %add3 = add nsw i32 %7, %5
  store i32 %add3, ptr %b2, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @_Z17cq415166_1d_f_pasP9my_struct(ptr %ypas) #1 {
entry:
  %ypas.addr = alloca ptr, align 8
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store ptr %ypas, ptr %ypas.addr, align 8
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:ARRSECT.PTR_TO_PTR.TYPED"(ptr %ypas.addr, %struct.my_struct zeroinitializer, i64 4, i64 5, ptr @_ZTSA4_9my_struct.omp.def_constr, ptr @_ZTSA4_9my_struct.omp.destr, ptr @.omp_combiner., ptr null),
    "QUAL.OMP.NUM_THREADS"(i32 100),
    "QUAL.OMP.SHARED:TYPED"(ptr @_ZL1x_cfb28f0ac03fe018acd8fdbeebb430b5, %struct.my_struct zeroinitializer, i64 100),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %j, i32 0, i32 1) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.UDR"
; FASTRED-NOT: __kmpc_atomic

; ALL: red.init.body{{.*}}:
; ALL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.init.body{{.*}} ]
; ALL: {{.*}} = call {{.*}} @_ZTSA4_9my_struct.omp.def_constr({{.*}})
; ALL: br i1 %red.cpy.done{{.*}}, label %red.init.done{{.*}}, label %red.init.body{{.*}}
; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: red.update.body{{.*}}:
; CRITICAL-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; CRITICAL: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; CRITICAL: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; CRITICAL: red.destr.body:                                   ; preds = %red.destr.body, %red.update.{{.*}}
; CRITICAL-NEXT:   %red.cpy.dest.ptr{{.*}} = phi ptr [ %array.begin, %red.update.done{{.*}} ], [ %red.cpy.dest.inc{{.*}}, %red.destr.body ]
; CRITICAL-NEXT:   call void @_ZTSA4_9my_struct.omp.destr(ptr %{{.*}})
; CRITICAL-NEXT:   %red.cpy.dest.inc{{.*}} = getelementptr %struct.my_struct, ptr %red.cpy.dest.ptr{{.*}}, i32 1
; CRITICAL-NEXT:   %red.cpy.done{{.*}} = icmp eq ptr %red.cpy.dest.inc{{.*}}, %{{.*}}
; CRITICAL-NEXT:   br i1 %red.cpy.done{{.*}}, label %red.destr.done, label %red.destr.body


; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: red.update.body{{.*}}:
; FASTRED-DAG-NEXT: %{{.*}} = phi {{.*}} [ {{.*}} ], [ {{.*}}, %red.update.body{{.*}} ]
; FASTRED-DAG: call void @.omp_combiner.(ptr %{{.*}}, ptr %{{.*}})
; FASTRED-DAG: br i1 %red.cpy.done{{.*}}, label %red.update.done{{.*}}, label %red.update.body{{.*}}
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED: red.destr.body:                                   ; preds = %red.destr.body, %tree.reduce.exit{{.*}}
; FASTRED-NEXT:  %red.cpy.dest.ptr{{.*}} = phi ptr [ %array.begin, %tree.reduce.exit{{.*}} ], [ %red.cpy.dest.inc{{.*}}, %red.destr.body ]
; FASTRED-NEXT:  call void @_ZTSA4_9my_struct.omp.destr(ptr %{{.*}})
; FASTRED-NEXT:  %red.cpy.dest.inc{{.*}} = getelementptr %struct.my_struct, ptr %red.cpy.dest.ptr{{.*}}, i32 1
; FASTRED-NEXT:  %red.cpy.done{{.*}} = icmp eq ptr %red.cpy.dest.inc{{.*}}, %{{.*}}
; FASTRED-NEXT:  br i1 %red.cpy.done{{.*}}, label %red.destr.done, label %red.destr.body

  %1 = load i32, ptr %.omp.lb, align 4
  store i32 %1, ptr %.omp.iv, align 4
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
  store i32 5, ptr %j, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %omp.inner.for.body
  %5 = load i32, ptr %j, align 4
  %cmp1 = icmp sle i32 %5, 8
  br i1 %cmp1, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %6 = load ptr, ptr %ypas.addr, align 8
  %7 = load i32, ptr %j, align 4
  %idxprom = sext i32 %7 to i64
  %arrayidx = getelementptr inbounds %struct.my_struct, ptr %6, i64 %idxprom
  %8 = load i32, ptr %i, align 4
  %idxprom2 = sext i32 %8 to i64
  %arrayidx3 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x_cfb28f0ac03fe018acd8fdbeebb430b5, i64 0, i64 %idxprom2
  call void @_Z6my_addR9my_structRKS_(ptr dereferenceable(8) %arrayidx, ptr dereferenceable(8) %arrayidx3) #2
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %9 = load i32, ptr %j, align 4
  %inc = add nsw i32 %9, 1
  store i32 %inc, ptr %j, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %10 = load i32, ptr %.omp.iv, align 4
  %add4 = add nsw i32 %10, 1
  store i32 %add4, ptr %.omp.iv, align 4
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
  call void @_Z6my_addR9my_structRKS_(ptr dereferenceable(8) %3, ptr dereferenceable(8) %2)
  ret void
}

; Function Attrs: noinline uwtable
define internal ptr @_ZTSA4_9my_struct.omp.def_constr(ptr %0) #3 section ".text.startup" {
entry:
  %retval = alloca ptr, align 8
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  %1 = load ptr, ptr %retval, align 8
  ret ptr %1
}

; Function Attrs: noinline uwtable
define internal void @_ZTSA4_9my_struct.omp.destr(ptr %0) #3 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  ret void
}

; Function Attrs: noinline norecurse nounwind optnone uwtable
define dso_local i32 @main() #4 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %0, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %i, align 4
  %2 = load i32, ptr %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x_cfb28f0ac03fe018acd8fdbeebb430b5, i64 0, i64 %idxprom
  %a = getelementptr inbounds %struct.my_struct, ptr %arrayidx, i32 0, i32 0
  store i32 %1, ptr %a, align 8
  %3 = load i32, ptr %i, align 4
  %4 = load i32, ptr %i, align 4
  %mul = mul nsw i32 %3, %4
  %5 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %5 to i64
  %arrayidx2 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1x_cfb28f0ac03fe018acd8fdbeebb430b5, i64 0, i64 %idxprom1
  %b = getelementptr inbounds %struct.my_struct, ptr %arrayidx2, i32 0, i32 1
  store i32 %mul, ptr %b, align 4
  %6 = load i32, ptr %i, align 4
  %idxprom3 = sext i32 %6 to i64
  %arrayidx4 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1y_cfb28f0ac03fe018acd8fdbeebb430b5, i64 0, i64 %idxprom3
  %a5 = getelementptr inbounds %struct.my_struct, ptr %arrayidx4, i32 0, i32 0
  store i32 0, ptr %a5, align 8
  %7 = load i32, ptr %i, align 4
  %idxprom6 = sext i32 %7 to i64
  %arrayidx7 = getelementptr inbounds [100 x %struct.my_struct], ptr @_ZL1y_cfb28f0ac03fe018acd8fdbeebb430b5, i64 0, i64 %idxprom6
  %b8 = getelementptr inbounds %struct.my_struct, ptr %arrayidx7, i32 0, i32 1
  store i32 0, ptr %b8, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %8 = load i32, ptr %i, align 4
  %inc = add nsw i32 %8, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @_Z17cq415166_1d_f_pasP9my_struct(ptr @_ZL1y_cfb28f0ac03fe018acd8fdbeebb430b5)
  ret i32 0
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noinline norecurse nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
