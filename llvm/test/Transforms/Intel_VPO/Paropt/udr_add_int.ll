; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED

;
; int a[1000];
; int main()
; {
;   int i;
;   int sum=0;
;   int sum_udr=0;
;
;   // Init array
;   for (i=0; i<1000; i++) {
;     a[i] = i%10;
;   }
;
;   #pragma omp declare reduction(myadd : int : omp_out += omp_in) initializer (omp_priv = 0)
;
;   #pragma omp parallel for reduction(myadd:sum_udr)
;   for (i=0; i<1000; i++) {
;     sum_udr = sum_udr+a[i];
;   }
;   return 0;
; }


; ModuleID = 'udr_add_int.c'
source_filename = "udr_add_int.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = common dso_local global [1000 x i32] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %sum = alloca i32, align 4
  %sum_udr = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %sum, align 4
  store i32 0, ptr %sum_udr, align 4
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %0, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, ptr %i, align 4
  %rem = srem i32 %1, 10
  %2 = load i32, ptr %i, align 4
  %idxprom = sext i32 %2 to i64
  %arrayidx = getelementptr inbounds [1000 x i32], ptr @a, i64 0, i64 %idxprom
  store i32 %rem, ptr %arrayidx, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %3 = load i32, ptr %i, align 4
  %inc = add nsw i32 %3, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, ptr %.omp.lb, align 4
  store i32 999, ptr %.omp.ub, align 4
  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %sum_udr, i32 0, i32 1, ptr null, ptr @_ZTSi.omp.destr, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.SHARED:TYPED"(ptr @a, i32 0, i64 1000),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0) ]

; CRITICAL-NOT: "QUAL.OMP.REDUCTION.UDR"
; CRITICAL: call void @.omp_initializer.(ptr %sum_udr.red{{.*}}, ptr %sum_udr)
; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: call void @.omp_combiner.(ptr %sum_udr, ptr %sum_udr.red{{.*}})
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; CRITICAL: call void @_ZTSi.omp.destr(ptr %sum_udr.red{{.*}})

; FASTRED-NOT: "QUAL.OMP.REDUCTION.UDR"
; FASTRED-NOT: __kmpc_atomic
; FASTRED: call void @.omp_initializer.(ptr %sum_udr.fast_red{{.*}}, ptr %sum_udr)
; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: call void @.omp_combiner.(ptr %sum_udr, ptr %sum_udr.fast_red{{.*}})
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED-DAG: call void @_ZTSi.omp.destr(ptr %sum_udr.fast_red{{.*}})

  %5 = load i32, ptr %.omp.lb, align 4
  store i32 %5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end
  %6 = load i32, ptr %.omp.iv, align 4
  %7 = load i32, ptr %.omp.ub, align 4
  %cmp1 = icmp sle i32 %6, %7
  br i1 %cmp1, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %8 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %8, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %9 = load i32, ptr %sum_udr, align 4
  %10 = load i32, ptr %i, align 4
  %idxprom2 = sext i32 %10 to i64
  %arrayidx3 = getelementptr inbounds [1000 x i32], ptr @a, i64 0, i64 %idxprom2
  %11 = load i32, ptr %arrayidx3, align 4
  %add4 = add nsw i32 %9, %11
  store i32 %add4, ptr %sum_udr, align 4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %12 = load i32, ptr %.omp.iv, align 4
  %add5 = add nsw i32 %12, 1
  store i32 %add5, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_combiner.(ptr noalias %0, ptr noalias %1) #2 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  %4 = load i32, ptr %2, align 4
  %5 = load i32, ptr %3, align 4
  %add = add nsw i32 %5, %4
  store i32 %add, ptr %3, align 4
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_initializer.(ptr noalias %0, ptr noalias %1) #2 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  store i32 0, ptr %3, align 4
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @_ZTSi.omp.destr(ptr %0) #2 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
