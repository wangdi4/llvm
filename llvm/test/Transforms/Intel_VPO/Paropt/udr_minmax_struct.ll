; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED
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
define dso_local void @minproc(%struct.point* %out, %struct.point* %in) #0 {
entry:
  %out.addr = alloca %struct.point*, align 8
  %in.addr = alloca %struct.point*, align 8
  store %struct.point* %out, %struct.point** %out.addr, align 8
  store %struct.point* %in, %struct.point** %in.addr, align 8
  %0 = load %struct.point*, %struct.point** %in.addr, align 8
  %x = getelementptr inbounds %struct.point, %struct.point* %0, i32 0, i32 0
  %1 = load i32, i32* %x, align 4
  %2 = load %struct.point*, %struct.point** %out.addr, align 8
  %x1 = getelementptr inbounds %struct.point, %struct.point* %2, i32 0, i32 0
  %3 = load i32, i32* %x1, align 4
  %cmp = icmp slt i32 %1, %3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %4 = load %struct.point*, %struct.point** %in.addr, align 8
  %x2 = getelementptr inbounds %struct.point, %struct.point* %4, i32 0, i32 0
  %5 = load i32, i32* %x2, align 4
  %6 = load %struct.point*, %struct.point** %out.addr, align 8
  %x3 = getelementptr inbounds %struct.point, %struct.point* %6, i32 0, i32 0
  store i32 %5, i32* %x3, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %7 = load %struct.point*, %struct.point** %in.addr, align 8
  %y = getelementptr inbounds %struct.point, %struct.point* %7, i32 0, i32 1
  %8 = load i32, i32* %y, align 4
  %9 = load %struct.point*, %struct.point** %out.addr, align 8
  %y4 = getelementptr inbounds %struct.point, %struct.point* %9, i32 0, i32 1
  %10 = load i32, i32* %y4, align 4
  %cmp5 = icmp slt i32 %8, %10
  br i1 %cmp5, label %if.then6, label %if.end9

if.then6:                                         ; preds = %if.end
  %11 = load %struct.point*, %struct.point** %in.addr, align 8
  %y7 = getelementptr inbounds %struct.point, %struct.point* %11, i32 0, i32 1
  %12 = load i32, i32* %y7, align 4
  %13 = load %struct.point*, %struct.point** %out.addr, align 8
  %y8 = getelementptr inbounds %struct.point, %struct.point* %13, i32 0, i32 1
  store i32 %12, i32* %y8, align 4
  br label %if.end9

if.end9:                                          ; preds = %if.then6, %if.end
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @maxproc(%struct.point* %out, %struct.point* %in) #0 {
entry:
  %out.addr = alloca %struct.point*, align 8
  %in.addr = alloca %struct.point*, align 8
  store %struct.point* %out, %struct.point** %out.addr, align 8
  store %struct.point* %in, %struct.point** %in.addr, align 8
  %0 = load %struct.point*, %struct.point** %in.addr, align 8
  %x = getelementptr inbounds %struct.point, %struct.point* %0, i32 0, i32 0
  %1 = load i32, i32* %x, align 4
  %2 = load %struct.point*, %struct.point** %out.addr, align 8
  %x1 = getelementptr inbounds %struct.point, %struct.point* %2, i32 0, i32 0
  %3 = load i32, i32* %x1, align 4
  %cmp = icmp sgt i32 %1, %3
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %4 = load %struct.point*, %struct.point** %in.addr, align 8
  %x2 = getelementptr inbounds %struct.point, %struct.point* %4, i32 0, i32 0
  %5 = load i32, i32* %x2, align 4
  %6 = load %struct.point*, %struct.point** %out.addr, align 8
  %x3 = getelementptr inbounds %struct.point, %struct.point* %6, i32 0, i32 0
  store i32 %5, i32* %x3, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %7 = load %struct.point*, %struct.point** %in.addr, align 8
  %y = getelementptr inbounds %struct.point, %struct.point* %7, i32 0, i32 1
  %8 = load i32, i32* %y, align 4
  %9 = load %struct.point*, %struct.point** %out.addr, align 8
  %y4 = getelementptr inbounds %struct.point, %struct.point* %9, i32 0, i32 1
  %10 = load i32, i32* %y4, align 4
  %cmp5 = icmp sgt i32 %8, %10
  br i1 %cmp5, label %if.then6, label %if.end9

if.then6:                                         ; preds = %if.end
  %11 = load %struct.point*, %struct.point** %in.addr, align 8
  %y7 = getelementptr inbounds %struct.point, %struct.point* %11, i32 0, i32 1
  %12 = load i32, i32* %y7, align 4
  %13 = load %struct.point*, %struct.point** %out.addr, align 8
  %y8 = getelementptr inbounds %struct.point, %struct.point* %13, i32 0, i32 1
  store i32 %12, i32* %y8, align 4
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
  store i32 0, i32* %retval, align 4
  store i32 100, i32* %SIZE, align 4
  %0 = bitcast %struct.point* %minp to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %0, i8* align 4 bitcast (%struct.point* @__const.main.minp to i8*), i64 8, i1 false)
  %1 = bitcast %struct.point* %maxp to i8*
  call void @llvm.memset.p0i8.i64(i8* align 4 %1, i8 0, i64 8, i1 false)
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %2 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %2, 100
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %3 = load i32, i32* %i, align 4
  %4 = load i32, i32* %i, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds [100 x %struct.point], [100 x %struct.point]* %points, i64 0, i64 %idxprom
  %x = getelementptr inbounds %struct.point, %struct.point* %arrayidx, i32 0, i32 0
  store i32 %3, i32* %x, align 8
  %5 = load i32, i32* %i, align 4
  %6 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %6 to i64
  %arrayidx2 = getelementptr inbounds [100 x %struct.point], [100 x %struct.point]* %points, i64 0, i64 %idxprom1
  %y = getelementptr inbounds %struct.point, %struct.point* %arrayidx2, i32 0, i32 1
  store i32 %5, i32* %y, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %7 = load i32, i32* %i, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.UDR"(%struct.point* %minp, i8* null, void (%struct.point*)* @_ZTS5point.omp.destr, void (%struct.point*, %struct.point*)* @.omp_combiner., void (%struct.point*, %struct.point*)* @.omp_initializer.), "QUAL.OMP.REDUCTION.UDR"(%struct.point* %maxp, i8* null, void (%struct.point*)* @_ZTS5point.omp.destr, void (%struct.point*, %struct.point*)* @.omp_combiner..1, void (%struct.point*, %struct.point*)* @.omp_initializer..2), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([100 x %struct.point]* %points) ]

; CRITICAL-NOT: "QUAL.OMP.REDUCTION.UDR"
; CRITICAL: call void @.omp_initializer..2(%struct.point* %maxp.red{{.*}}, %struct.point* %maxp)
; CRITICAL: call void @.omp_initializer.(%struct.point* %minp.red{{.*}}, %struct.point* %minp)
; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: call void @.omp_combiner.(%struct.point* %minp, %struct.point* %minp.red{{.*}})
; CRITICAL: call void @.omp_combiner..1(%struct.point* %maxp, %struct.point* %maxp.red{{.*}})
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; CRITICAL: call void @_ZTS5point.omp.destr(%struct.point* %minp.red{{.*}})
; CRITICAL: call void @_ZTS5point.omp.destr(%struct.point* %maxp.red{{.*}})

; FASTRED-NOT: "QUAL.OMP.REDUCTION.UDR"
; FASTRED-NOT: __kmpc_atomic
; FASTRED: call void @.omp_initializer..2(%struct.point* %maxp.red{{.*}}, %struct.point* %maxp)
; FASTRED: call void @.omp_initializer.(%struct.point* %minp.red{{.*}}, %struct.point* %minp)
; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED-DAG: call void @.omp_combiner.(%struct.point* %minp, %struct.point* %minp.fast_red{{.*}})
; FASTRED-DAG: call void @.omp_combiner..1(%struct.point* %maxp, %struct.point* %maxp.fast_red{{.*}})
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED-DAG: call void @_ZTS5point.omp.destr(%struct.point* %minp.fast_red{{.*}})
; FASTRED-DAG: call void @_ZTS5point.omp.destr(%struct.point* %maxp.fast_red{{.*}})

  %9 = load i32, i32* %.omp.lb, align 4
  store i32 %9, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end
  %10 = load i32, i32* %.omp.iv, align 4
  %11 = load i32, i32* %.omp.ub, align 4
  %cmp3 = icmp sle i32 %10, %11
  br i1 %cmp3, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %12 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %12, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %13 = load i32, i32* %i, align 4
  %idxprom4 = sext i32 %13 to i64
  %arrayidx5 = getelementptr inbounds [100 x %struct.point], [100 x %struct.point]* %points, i64 0, i64 %idxprom4
  call void @minproc(%struct.point* %minp, %struct.point* %arrayidx5) #4
  %14 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %14 to i64
  %arrayidx7 = getelementptr inbounds [100 x %struct.point], [100 x %struct.point]* %points, i64 0, i64 %idxprom6
  call void @maxproc(%struct.point* %maxp, %struct.point* %arrayidx7) #4
  br label %omp.body.continue

omp.body.continue:                                ; preds = %omp.inner.for.body
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %15 = load i32, i32* %.omp.iv, align 4
  %add8 = add nsw i32 %15, 1
  store i32 %add8, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  %16 = load i32, i32* %retval, align 4
  ret i32 %16
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_combiner.(%struct.point* noalias %0, %struct.point* noalias %1) #5 {
entry:
  %.addr = alloca %struct.point*, align 8
  %.addr1 = alloca %struct.point*, align 8
  store %struct.point* %0, %struct.point** %.addr, align 8
  store %struct.point* %1, %struct.point** %.addr1, align 8
  %2 = load %struct.point*, %struct.point** %.addr1, align 8
  %3 = load %struct.point*, %struct.point** %.addr, align 8
  call void @minproc(%struct.point* %3, %struct.point* %2)
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_initializer.(%struct.point* noalias %0, %struct.point* noalias %1) #5 {
entry:
  %.addr = alloca %struct.point*, align 8
  %.addr1 = alloca %struct.point*, align 8
  store %struct.point* %0, %struct.point** %.addr, align 8
  store %struct.point* %1, %struct.point** %.addr1, align 8
  %2 = load %struct.point*, %struct.point** %.addr1, align 8
  %3 = load %struct.point*, %struct.point** %.addr, align 8
  %x = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 0
  store i32 2147483647, i32* %x, align 4
  %y = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 1
  store i32 2147483647, i32* %y, align 4
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @_ZTS5point.omp.destr(%struct.point* %0) #5 section ".text.startup" {
entry:
  %.addr = alloca %struct.point*, align 8
  store %struct.point* %0, %struct.point** %.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_combiner..1(%struct.point* noalias %0, %struct.point* noalias %1) #5 {
entry:
  %.addr = alloca %struct.point*, align 8
  %.addr1 = alloca %struct.point*, align 8
  store %struct.point* %0, %struct.point** %.addr, align 8
  store %struct.point* %1, %struct.point** %.addr1, align 8
  %2 = load %struct.point*, %struct.point** %.addr1, align 8
  %3 = load %struct.point*, %struct.point** %.addr, align 8
  call void @maxproc(%struct.point* %3, %struct.point* %2)
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_initializer..2(%struct.point* noalias %0, %struct.point* noalias %1) #5 {
entry:
  %.addr = alloca %struct.point*, align 8
  %.addr1 = alloca %struct.point*, align 8
  store %struct.point* %0, %struct.point** %.addr, align 8
  store %struct.point* %1, %struct.point** %.addr1, align 8
  %2 = load %struct.point*, %struct.point** %.addr1, align 8
  %3 = load %struct.point*, %struct.point** %.addr, align 8
  %x = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 0
  store i32 0, i32* %x, align 4
  %y = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 1
  store i32 0, i32* %y, align 4
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
