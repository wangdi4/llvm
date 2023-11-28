; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED


; #include <limits.h>
;
; struct point {
;   int x;
;   int y;
; };
;
; void minproc ( struct point *out, struct point *in )
; {
;   if ( in->x < out->x ) out->x = in->x;
;   if ( in->y < out->y ) out->y = in->y;
; }
;
; void maxproc ( struct point *out, struct point *in )
; {
;   if ( in->x > out->x ) out->x = in->x;
;   if ( in->y > out->y ) out->y = in->y;
; }
;
; #pragma omp declare reduction(min : struct point : \
;                               minproc(&omp_out, &omp_in)) \
; initializer( omp_priv = { INT_MAX, INT_MAX } )
;
; #pragma omp declare reduction(max : struct point : \
;                               maxproc(&omp_out, &omp_in)) \
; initializer( omp_priv = { 0, 0 } )
;
; int main(void)
; {
;   const int SIZE=100;
;   struct point points[SIZE];
;   struct point minp = { INT_MAX, INT_MAX }, maxp = {0,0};
;   int i;
;
;   for (i=0; i<SIZE; i++) {
;     points[i].x = i;
;     points[i].y = i;
;   }
;
; #pragma omp parallel for reduction(min:minp) reduction(max:maxp)
;   for ( i = 0; i < SIZE; i++ ) {
;     minproc(&minp, &points[i]);
;     maxproc(&maxp, &points[i]);
;   }
; }


; ModuleID = 'udr_minmax_struct.c'
source_filename = "udr_minmax_struct.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.point = type { i32, i32 }

@__const.main.minp = private unnamed_addr constant %struct.point { i32 2147483647, i32 2147483647 }, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @minproc(ptr %out, ptr %in) #0 {
entry:
  %out.addr = alloca ptr, align 8
  %in.addr = alloca ptr, align 8
  store ptr %out, ptr %out.addr, align 8
  store ptr %in, ptr %in.addr, align 8
  %0 = load ptr, ptr %in.addr, align 8
  %x = getelementptr inbounds %struct.point, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %x, align 4
  %2 = load ptr, ptr %out.addr, align 8
  %x1 = getelementptr inbounds %struct.point, ptr %2, i32 0, i32 0
  %3 = load i32, ptr %x1, align 4
  %cmp = icmp slt i32 %1, %3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %4 = load ptr, ptr %in.addr, align 8
  %x2 = getelementptr inbounds %struct.point, ptr %4, i32 0, i32 0
  %5 = load i32, ptr %x2, align 4
  %6 = load ptr, ptr %out.addr, align 8
  %x3 = getelementptr inbounds %struct.point, ptr %6, i32 0, i32 0
  store i32 %5, ptr %x3, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %7 = load ptr, ptr %in.addr, align 8
  %y = getelementptr inbounds %struct.point, ptr %7, i32 0, i32 1
  %8 = load i32, ptr %y, align 4
  %9 = load ptr, ptr %out.addr, align 8
  %y4 = getelementptr inbounds %struct.point, ptr %9, i32 0, i32 1
  %10 = load i32, ptr %y4, align 4
  %cmp5 = icmp slt i32 %8, %10
  br i1 %cmp5, label %if.then6, label %if.end9

if.then6:                                         ; preds = %if.end
  %11 = load ptr, ptr %in.addr, align 8
  %y7 = getelementptr inbounds %struct.point, ptr %11, i32 0, i32 1
  %12 = load i32, ptr %y7, align 4
  %13 = load ptr, ptr %out.addr, align 8
  %y8 = getelementptr inbounds %struct.point, ptr %13, i32 0, i32 1
  store i32 %12, ptr %y8, align 4
  br label %if.end9

if.end9:                                          ; preds = %if.then6, %if.end
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @maxproc(ptr %out, ptr %in) #0 {
entry:
  %out.addr = alloca ptr, align 8
  %in.addr = alloca ptr, align 8
  store ptr %out, ptr %out.addr, align 8
  store ptr %in, ptr %in.addr, align 8
  %0 = load ptr, ptr %in.addr, align 8
  %x = getelementptr inbounds %struct.point, ptr %0, i32 0, i32 0
  %1 = load i32, ptr %x, align 4
  %2 = load ptr, ptr %out.addr, align 8
  %x1 = getelementptr inbounds %struct.point, ptr %2, i32 0, i32 0
  %3 = load i32, ptr %x1, align 4
  %cmp = icmp sgt i32 %1, %3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %4 = load ptr, ptr %in.addr, align 8
  %x2 = getelementptr inbounds %struct.point, ptr %4, i32 0, i32 0
  %5 = load i32, ptr %x2, align 4
  %6 = load ptr, ptr %out.addr, align 8
  %x3 = getelementptr inbounds %struct.point, ptr %6, i32 0, i32 0
  store i32 %5, ptr %x3, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %7 = load ptr, ptr %in.addr, align 8
  %y = getelementptr inbounds %struct.point, ptr %7, i32 0, i32 1
  %8 = load i32, ptr %y, align 4
  %9 = load ptr, ptr %out.addr, align 8
  %y4 = getelementptr inbounds %struct.point, ptr %9, i32 0, i32 1
  %10 = load i32, ptr %y4, align 4
  %cmp5 = icmp sgt i32 %8, %10
  br i1 %cmp5, label %if.then6, label %if.end9

if.then6:                                         ; preds = %if.end
  %11 = load ptr, ptr %in.addr, align 8
  %y7 = getelementptr inbounds %struct.point, ptr %11, i32 0, i32 1
  %12 = load i32, ptr %y7, align 4
  %13 = load ptr, ptr %out.addr, align 8
  %y8 = getelementptr inbounds %struct.point, ptr %13, i32 0, i32 1
  store i32 %12, ptr %y8, align 4
  br label %if.end9

if.end9:                                          ; preds = %if.then6, %if.end
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #1 {
entry:
  %retval = alloca i32, align 4
  %SIZE = alloca i32, align 4
  %points = alloca [100 x %struct.point], align 16
  %minp = alloca %struct.point, align 4
  %maxp = alloca %struct.point, align 4
  %i = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 100, ptr %SIZE, align 4
  %0 = bitcast ptr %minp to ptr
  call void @llvm.memcpy.p0i8.p0i8.i64(ptr align 4 %0, ptr align 4 bitcast (ptr @__const.main.minp to ptr), i64 8, i1 false)
  %1 = bitcast ptr %maxp to ptr
  call void @llvm.memset.p0i8.i64(ptr align 4 %1, i8 0, i64 8, i1 false)
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %2, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i32, ptr %i, align 4
  %4 = load i32, ptr %i, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds [100 x %struct.point], ptr %points, i64 0, i64 %idxprom
  %x = getelementptr inbounds %struct.point, ptr %arrayidx, i32 0, i32 0
  store i32 %3, ptr %x, align 8
  %5 = load i32, ptr %i, align 4
  %6 = load i32, ptr %i, align 4
  %idxprom1 = sext i32 %6 to i64
  %arrayidx2 = getelementptr inbounds [100 x %struct.point], ptr %points, i64 0, i64 %idxprom1
  %y = getelementptr inbounds %struct.point, ptr %arrayidx2, i32 0, i32 1
  store i32 %5, ptr %y, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32, ptr %i, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, ptr %.omp.lb, align 4
  store i32 99, ptr %.omp.ub, align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %minp, %struct.point zeroinitializer, i32 1, ptr null, ptr @_ZTS5point.omp.destr, ptr @.omp_combiner., ptr @.omp_initializer.),
    "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %maxp, %struct.point zeroinitializer, i32 1, ptr null, ptr @_ZTS5point.omp.destr, ptr @.omp_combiner..1, ptr @.omp_initializer..2),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %.omp.lb, i32 0, i32 1),
    "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr %.omp.iv, i32 0),
    "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr %.omp.ub, i32 0),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %i, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %points, [100 x %struct.point] zeroinitializer, i32 1) ]

; CRITICAL-NOT: "QUAL.OMP.REDUCTION.UDR"
; CRITICAL: call void @.omp_initializer..2(ptr %maxp.red{{.*}}, ptr %maxp)
; CRITICAL: call void @.omp_initializer.(ptr %minp.red{{.*}}, ptr %minp)
; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: call void @.omp_combiner.(ptr %minp, ptr %minp.red{{.*}})
; CRITICAL: call void @.omp_combiner..1(ptr %maxp, ptr %maxp.red{{.*}})
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; CRITICAL: call void @_ZTS5point.omp.destr(ptr %minp.red{{.*}})
; CRITICAL: call void @_ZTS5point.omp.destr(ptr %maxp.red{{.*}})

; FASTRED-NOT: "QUAL.OMP.REDUCTION.UDR"
; FASTRED-NOT: __kmpc_atomic
; FASTRED: call void @.omp_initializer..2(ptr %maxp.fast_red{{.*}}, ptr %maxp)
; FASTRED: call void @.omp_initializer.(ptr %minp.fast_red{{.*}}, ptr %minp)
; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: call void @.omp_combiner.(ptr %minp, ptr %minp.fast_red{{.*}})
; FASTRED-DAG: call void @.omp_combiner..1(ptr %maxp, ptr %maxp.fast_red{{.*}})
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED-DAG: call void @_ZTS5point.omp.destr(ptr %minp.fast_red{{.*}})
; FASTRED-DAG: call void @_ZTS5point.omp.destr(ptr %maxp.fast_red{{.*}})

  %9 = load i32, ptr %.omp.lb, align 4
  store i32 %9, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end
  %10 = load i32, ptr %.omp.iv, align 4
  %11 = load i32, ptr %.omp.ub, align 4
  %cmp3 = icmp sle i32 %10, %11
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, ptr %.omp.iv, align 4
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, ptr %i, align 4
  %13 = load i32, ptr %i, align 4
  %idxprom4 = sext i32 %13 to i64
  %arrayidx5 = getelementptr inbounds [100 x %struct.point], ptr %points, i64 0, i64 %idxprom4
  call void @minproc(ptr %minp, ptr %arrayidx5) #4
  %14 = load i32, ptr %i, align 4
  %idxprom6 = sext i32 %14 to i64
  %arrayidx7 = getelementptr inbounds [100 x %struct.point], ptr %points, i64 0, i64 %idxprom6
  call void @maxproc(ptr %maxp, ptr %arrayidx7) #4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, ptr %.omp.iv, align 4
  %add8 = add nsw i32 %15, 1
  store i32 %add8, ptr %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.PARALLEL.LOOP"() ]

  %16 = load i32, ptr %retval, align 4
  ret i32 %16
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_combiner.(ptr noalias %0, ptr noalias %1) #5 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @minproc(ptr %3, ptr %2)
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_initializer.(ptr noalias %0, ptr noalias %1) #5 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  %x = getelementptr inbounds %struct.point, ptr %3, i32 0, i32 0
  store i32 2147483647, ptr %x, align 4
  %y = getelementptr inbounds %struct.point, ptr %3, i32 0, i32 1
  store i32 2147483647, ptr %y, align 4
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @_ZTS5point.omp.destr(ptr %0) #5 section ".text.startup" {
entry:
  %.addr = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_combiner..1(ptr noalias %0, ptr noalias %1) #5 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  call void @maxproc(ptr %3, ptr %2)
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_initializer..2(ptr noalias %0, ptr noalias %1) #5 {
entry:
  %.addr = alloca ptr, align 8
  %.addr1 = alloca ptr, align 8
  store ptr %0, ptr %.addr, align 8
  store ptr %1, ptr %.addr1, align 8
  %2 = load ptr, ptr %.addr1, align 8
  %3 = load ptr, ptr %.addr, align 8
  %x = getelementptr inbounds %struct.point, ptr %3, i32 0, i32 0
  store i32 0, ptr %x, align 4
  %y = getelementptr inbounds %struct.point, ptr %3, i32 0, i32 1
  store i32 0, ptr %y, align 4
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind willreturn }
attributes #3 = { argmemonly nounwind willreturn writeonly }
attributes #4 = { nounwind }
attributes #5 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
