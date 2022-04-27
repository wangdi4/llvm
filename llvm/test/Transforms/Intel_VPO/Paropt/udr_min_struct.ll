; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction=false -S %s | FileCheck %s --check-prefix=CRITICAL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s --check-prefix=FASTRED
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s --check-prefix=FASTRED

;
; typedef struct {int x; int y;} point;
;
; #define N        1000
; #define LARGENUM 9999
;
; point points[N];
; point corner={LARGENUM,LARGENUM};
;
; int main()
; {
;   int i;
;   int minx = LARGENUM;
;   int miny = LARGENUM;
;
;   // Initialize points
;   for (i=0; i<N; i++) {
;     int x = i;
;     int y = N-i;
;     points[i].x = x;  // range: 0 ..  999
;     points[i].y = y;  // range: 1 .. 1000
;     if (minx > x) {
;       minx = x;
;     }
;     if (miny > y) {
;       miny = y;
;     }
;   }
;
;   // Use UDR to find lower-left corner of the
;   // smallest Cartesian rectancle enclosing the points
;
;   #pragma omp declare reduction(min : point :  \
;     omp_out.x = omp_in.x > omp_out.x ? omp_out.x : omp_in.x, \
;     omp_out.y = omp_in.y > omp_out.y ? omp_out.y : omp_in.y) \
;     initializer (omp_priv={LARGENUM,LARGENUM})
;
;   #pragma omp parallel for reduction(min:corner)
;   for (i=0; i<1000; i++) {
;     if (points[i].x < corner.x) corner.x = points[i].x;
;     if (points[i].y < corner.y) corner.y = points[i].y;
;   }
;   return 0;
; }


; ModuleID = 'udr_min_struct.c'
source_filename = "udr_min_struct.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.point = type { i32, i32 }

@corner = dso_local global %struct.point { i32 9999, i32 9999 }, align 4
@points = common dso_local global [1000 x %struct.point] zeroinitializer, align 16

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %minx = alloca i32, align 4
  %miny = alloca i32, align 4
  %x = alloca i32, align 4
  %y = alloca i32, align 4
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 9999, i32* %minx, align 4
  store i32 9999, i32* %miny, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 1000
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  store i32 %1, i32* %x, align 4
  %2 = load i32, i32* %i, align 4
  %sub = sub nsw i32 1000, %2
  store i32 %sub, i32* %y, align 4
  %3 = load i32, i32* %x, align 4
  %4 = load i32, i32* %i, align 4
  %idxprom = sext i32 %4 to i64
  %arrayidx = getelementptr inbounds [1000 x %struct.point], [1000 x %struct.point]* @points, i64 0, i64 %idxprom
  %x1 = getelementptr inbounds %struct.point, %struct.point* %arrayidx, i32 0, i32 0
  store i32 %3, i32* %x1, align 8
  %5 = load i32, i32* %y, align 4
  %6 = load i32, i32* %i, align 4
  %idxprom2 = sext i32 %6 to i64
  %arrayidx3 = getelementptr inbounds [1000 x %struct.point], [1000 x %struct.point]* @points, i64 0, i64 %idxprom2
  %y4 = getelementptr inbounds %struct.point, %struct.point* %arrayidx3, i32 0, i32 1
  store i32 %5, i32* %y4, align 4
  %7 = load i32, i32* %minx, align 4
  %8 = load i32, i32* %x, align 4
  %cmp5 = icmp sgt i32 %7, %8
  br i1 %cmp5, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %9 = load i32, i32* %x, align 4
  store i32 %9, i32* %minx, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %for.body
  %10 = load i32, i32* %miny, align 4
  %11 = load i32, i32* %y, align 4
  %cmp6 = icmp sgt i32 %10, %11
  br i1 %cmp6, label %if.then7, label %if.end8

if.then7:                                         ; preds = %if.end
  %12 = load i32, i32* %y, align 4
  store i32 %12, i32* %miny, align 4
  br label %if.end8

if.end8:                                          ; preds = %if.then7, %if.end
  br label %for.inc

for.inc:                                          ; preds = %if.end8
  %13 = load i32, i32* %i, align 4
  %inc = add nsw i32 %13, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  store i32 0, i32* %.omp.lb, align 4
  store i32 999, i32* %.omp.ub, align 4
  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.UDR"(%struct.point* @corner, i8* null, void (%struct.point*)* @_ZTS5point.omp.destr, void (%struct.point*, %struct.point*)* @.omp_combiner., void (%struct.point*, %struct.point*)* @.omp_initializer.), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([1000 x %struct.point]* @points) ]

; CRITICAL-NOT: "QUAL.OMP.REDUCTION.UDR"
; CRITICAL: call void @.omp_initializer.(%struct.point* %corner.red{{.*}}, %struct.point* @corner)
; CRITICAL: call void @__kmpc_critical({{.*}})
; CRITICAL: call void @.omp_combiner.(%struct.point* @corner, %struct.point* %corner.red{{.*}})
; CRITICAL: call void @__kmpc_end_critical({{.*}})
; CRITICAL: call void @_ZTS5point.omp.destr(%struct.point* %corner.red{{.*}})

; FASTRED-NOT: "QUAL.OMP.REDUCTION.UDR"
; FASTRED-NOT: __kmpc_atomic
; FASTRED: call void @.omp_initializer.(%struct.point* %corner.red{{.*}}, %struct.point* @corner)
; FASTRED: call i32 @__kmpc_reduce({{.*}})
; FASTRED: call void @.omp_combiner.(%struct.point* @corner, %struct.point* %corner.fast_red{{.*}})
; FASTRED-DAG: call void @__kmpc_end_reduce({{.*}})
; FASTRED-DAG: call void @_ZTS5point.omp.destr(%struct.point* %corner.fast_red{{.*}})

  %15 = load i32, i32* %.omp.lb, align 4
  store i32 %15, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end
  %16 = load i32, i32* %.omp.iv, align 4
  %17 = load i32, i32* %.omp.ub, align 4
  %cmp9 = icmp sle i32 %16, %17
  br i1 %cmp9, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %18 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %18, 1
  %add = add nsw i32 0, %mul
  store i32 %add, i32* %i, align 4
  %19 = load i32, i32* %i, align 4
  %idxprom10 = sext i32 %19 to i64
  %arrayidx11 = getelementptr inbounds [1000 x %struct.point], [1000 x %struct.point]* @points, i64 0, i64 %idxprom10
  %x12 = getelementptr inbounds %struct.point, %struct.point* %arrayidx11, i32 0, i32 0
  %20 = load i32, i32* %x12, align 8
  %21 = load i32, i32* getelementptr inbounds (%struct.point, %struct.point* @corner, i32 0, i32 0), align 4
  %cmp13 = icmp slt i32 %20, %21
  br i1 %cmp13, label %if.then14, label %if.end18

if.then14:                                        ; preds = %omp.inner.for.body
  %22 = load i32, i32* %i, align 4
  %idxprom15 = sext i32 %22 to i64
  %arrayidx16 = getelementptr inbounds [1000 x %struct.point], [1000 x %struct.point]* @points, i64 0, i64 %idxprom15
  %x17 = getelementptr inbounds %struct.point, %struct.point* %arrayidx16, i32 0, i32 0
  %23 = load i32, i32* %x17, align 8
  store i32 %23, i32* getelementptr inbounds (%struct.point, %struct.point* @corner, i32 0, i32 0), align 4
  br label %if.end18

if.end18:                                         ; preds = %if.then14, %omp.inner.for.body
  %24 = load i32, i32* %i, align 4
  %idxprom19 = sext i32 %24 to i64
  %arrayidx20 = getelementptr inbounds [1000 x %struct.point], [1000 x %struct.point]* @points, i64 0, i64 %idxprom19
  %y21 = getelementptr inbounds %struct.point, %struct.point* %arrayidx20, i32 0, i32 1
  %25 = load i32, i32* %y21, align 4
  %26 = load i32, i32* getelementptr inbounds (%struct.point, %struct.point* @corner, i32 0, i32 1), align 4
  %cmp22 = icmp slt i32 %25, %26
  br i1 %cmp22, label %if.then23, label %if.end27

if.then23:                                        ; preds = %if.end18
  %27 = load i32, i32* %i, align 4
  %idxprom24 = sext i32 %27 to i64
  %arrayidx25 = getelementptr inbounds [1000 x %struct.point], [1000 x %struct.point]* @points, i64 0, i64 %idxprom24
  %y26 = getelementptr inbounds %struct.point, %struct.point* %arrayidx25, i32 0, i32 1
  %28 = load i32, i32* %y26, align 4
  store i32 %28, i32* getelementptr inbounds (%struct.point, %struct.point* @corner, i32 0, i32 1), align 4
  br label %if.end27

if.end27:                                         ; preds = %if.then23, %if.end18
  br label %omp.body.continue

omp.body.continue:                                ; preds = %if.end27
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %29 = load i32, i32* %.omp.iv, align 4
  %add28 = add nsw i32 %29, 1
  store i32 %add28, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_combiner.(%struct.point* noalias %0, %struct.point* noalias %1) #2 {
entry:
  %.addr = alloca %struct.point*, align 8
  %.addr1 = alloca %struct.point*, align 8
  store %struct.point* %0, %struct.point** %.addr, align 8
  store %struct.point* %1, %struct.point** %.addr1, align 8
  %2 = load %struct.point*, %struct.point** %.addr1, align 8
  %3 = load %struct.point*, %struct.point** %.addr, align 8
  %x = getelementptr inbounds %struct.point, %struct.point* %2, i32 0, i32 0
  %4 = load i32, i32* %x, align 4
  %x2 = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 0
  %5 = load i32, i32* %x2, align 4
  %cmp = icmp sgt i32 %4, %5
  br i1 %cmp, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %x3 = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 0
  %6 = load i32, i32* %x3, align 4
  br label %cond.end

cond.false:                                       ; preds = %entry
  %x4 = getelementptr inbounds %struct.point, %struct.point* %2, i32 0, i32 0
  %7 = load i32, i32* %x4, align 4
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %6, %cond.true ], [ %7, %cond.false ]
  %x5 = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 0
  store i32 %cond, i32* %x5, align 4
  %y = getelementptr inbounds %struct.point, %struct.point* %2, i32 0, i32 1
  %8 = load i32, i32* %y, align 4
  %y6 = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 1
  %9 = load i32, i32* %y6, align 4
  %cmp7 = icmp sgt i32 %8, %9
  br i1 %cmp7, label %cond.true8, label %cond.false10

cond.true8:                                       ; preds = %cond.end
  %y9 = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 1
  %10 = load i32, i32* %y9, align 4
  br label %cond.end12

cond.false10:                                     ; preds = %cond.end
  %y11 = getelementptr inbounds %struct.point, %struct.point* %2, i32 0, i32 1
  %11 = load i32, i32* %y11, align 4
  br label %cond.end12

cond.end12:                                       ; preds = %cond.false10, %cond.true8
  %cond13 = phi i32 [ %10, %cond.true8 ], [ %11, %cond.false10 ]
  %y14 = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 1
  store i32 %cond13, i32* %y14, align 4
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @.omp_initializer.(%struct.point* noalias %0, %struct.point* noalias %1) #2 {
entry:
  %.addr = alloca %struct.point*, align 8
  %.addr1 = alloca %struct.point*, align 8
  store %struct.point* %0, %struct.point** %.addr, align 8
  store %struct.point* %1, %struct.point** %.addr1, align 8
  %2 = load %struct.point*, %struct.point** %.addr1, align 8
  %3 = load %struct.point*, %struct.point** %.addr, align 8
  %x = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 0
  store i32 9999, i32* %x, align 4
  %y = getelementptr inbounds %struct.point, %struct.point* %3, i32 0, i32 1
  store i32 9999, i32* %y, align 4
  ret void
}

; Function Attrs: noinline nounwind uwtable
define internal void @_ZTS5point.omp.destr(%struct.point* %0) #2 section ".text.startup" {
entry:
  %.addr = alloca %struct.point*, align 8
  store %struct.point* %0, %struct.point** %.addr, align 8
  ret void
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
