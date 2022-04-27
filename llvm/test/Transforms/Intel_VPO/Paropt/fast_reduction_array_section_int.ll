; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction-ctrl=0 -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction-ctrl=0 -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL


;
; #define N 100
;
; int main(void)
; {
;   int i, j;
;   int a[N], b[N][N];
;   for (i=0; i<N; i++) {
;     a[i] = 0;
;     for (j=0; j<N; j++) {
;       b[i][j] = i + j;
;     }
;   }
;
; #pragma omp parallel for reduction(+:a[0:N]) private(j)
;   for (i=0; i<N; i++) {
;     for (j=0; j<N; j++) {
;       a[j] += b[i][j];
;     }
;   }
;
;   return 0;
; }


; ModuleID = 'fast_reduction_array_section_int.c'
source_filename = "fast_reduction_array_section_int.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %a = alloca [100 x i32], align 16
  %b = alloca [100 x [100 x i32]], align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc8, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 100
  br i1 %cmp, label %for.body, label %for.end10

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom
  store i32 0, i32* %arrayidx, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %2 = load i32, i32* %j, align 4
  %cmp2 = icmp slt i32 %2, 100
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %3 = load i32, i32* %i, align 4
  %4 = load i32, i32* %j, align 4
  %add = add nsw i32 %3, %4
  %5 = load i32, i32* %i, align 4
  %idxprom4 = sext i32 %5 to i64
  %arrayidx5 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %b, i64 0, i64 %idxprom4
  %6 = load i32, i32* %j, align 4
  %idxprom6 = sext i32 %6 to i64
  %arrayidx7 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx5, i64 0, i64 %idxprom6
  store i32 %add, i32* %arrayidx7, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %7 = load i32, i32* %j, align 4
  %inc = add nsw i32 %7, 1
  store i32 %inc, i32* %j, align 4
  br label %for.cond1

for.end:                                          ; preds = %for.cond1
  br label %for.inc8

for.inc8:                                         ; preds = %for.end
  %8 = load i32, i32* %i, align 4
  %inc9 = add nsw i32 %8, 1
  store i32 %inc9, i32* %i, align 4
  br label %for.cond

for.end10:                                        ; preds = %for.cond
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([100 x i32]* %a, i64 1, i64 0, i64 100, i64 1), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([100 x [100 x i32]]* %b), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
; ALL-NOT: __kmpc_atomic
; ALL: %struct.fast_red_t = type <{ [100 x i32] }>
; ALL: define internal void @main_tree_reduce_{{.*}}(i8* %dst, i8* %src) {
; ALL: declare i32 @__kmpc_reduce(%struct.ident_t*, i32, i32, i32, i8*, void (i8*, i8*)*, [8 x i32]*)
; ALL: declare void @__kmpc_end_reduce(%struct.ident_t*, i32, [8 x i32]*)
; ALL: %fast_red_struct{{.*}} = alloca %struct.fast_red_t, align 16

; USE-REC: %[[REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
; USE-REC-NEXT: %[[REC_GEP:[^,]+]] = getelementptr inbounds [100 x i32], [100 x i32]* %[[REC]], i32 0, i32 0
; USE-REC-NEXT: %[[REC_GEP_MINUS_OFFSET:[^,]+]] = getelementptr i32, i32* %[[REC_GEP]], i64 0
; USE-REC-NEXT: %[[REC_GEP_MINUS_OFFSET_CAST:[^,]+]] = bitcast i32* %[[REC_GEP_MINUS_OFFSET]] to [100 x i32]*
; USE-REC: %[[END:[^,]+]] = getelementptr i32, i32* %[[REC_GEP]], i64 100
; USE-REC-NEXT: %[[ISEMPTY:[^,]+]] = icmp eq i32* %[[REC_GEP]], %[[END]]
; USE-REC-NEXT: br i1 %[[ISEMPTY]], label %[[DONE:[^,]+]], label %[[BODY:[^,]+]]
; USE-REC: [[DONE]]:
; USE-REC: [[BODY]]:


; USE-LOCAL: %[[LOCAL:[^,]+]] = alloca [100 x i32], align 16
; USE-LOCAL: %[[LOCAL_GEP:[^,]+]] = getelementptr inbounds [100 x i32], [100 x i32]* %[[LOCAL]], i32 0, i32 0
; USE-LOCAL-NEXT: %[[LOCAL_GEP_MINUS_OFFSET:[^,]+]] = getelementptr i32, i32* %[[LOCAL_GEP]], i64 0
; USE-LOCAL-NEXT: %[[LOCAL_GEP_MINUS_OFFSET_CAST:[^,]+]] = bitcast i32* %[[LOCAL_GEP_MINUS_OFFSET]] to [100 x i32]*
; USE-LOCAL: %[[REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
; USE-LOCAL-NEXT: %[[REC_GEP:[^,]+]] = getelementptr inbounds [100 x i32], [100 x i32]* %[[REC]], i32 0, i32 0

; USE-LOCAL: %[[END:[^,]+]] = getelementptr i32, i32* %[[LOCAL_GEP]], i64 100
; USE-LOCAL-NEXT: %[[ISEMPTY:[^,]+]] = icmp eq i32* %[[LOCAL_GEP]], %[[END]]
; USE-LOCAL-NEXT: br i1 %[[ISEMPTY]], label %[[DONE:[^,]+]], label %[[BODY:[^,]+]]
; USE-LOCAL: [[DONE]]:
; USE-LOCAL: [[BODY]]:

; ALL: call void @__kmpc_for_static_fini(%struct.ident_t* @{{.*}}, i32 %{{.*}})
; ALL: %[[BITCAST:[^,]+]] = bitcast %struct.fast_red_t* %fast_red_struct to i8*
; ALL: %[[RET:[^,]+]] = call i32 @__kmpc_reduce(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 1, i32 400, i8* %[[BITCAST]], void (i8*, i8*)* @main_tree_reduce_{{.*}}, [8 x i32]* @{{.*}})
; ALL-NEXT: %to.tree.reduce = icmp eq i32 %[[RET]], 1
; ALL-NEXT: br i1 %to.tree.reduce, label %tree.reduce, label %tree.reduce.exit
; ALL: tree.reduce:
; ALL: tree.reduce.exit:
; ALL: %[[GLOBAL_CAST:[^,]+]] = bitcast [100 x i32]* %[[GLOBAL:[^,]+]] to i32*
; ALL-NEXT: %[[GLOBAL_CAST_PLUS_OFFSET:[^,]+]] = getelementptr i32, i32* %[[GLOBAL_CAST]], i64 0
; ALL: %[[GLOBAL_END:[^,]+]] = getelementptr i32, i32* %[[GLOBAL_CAST_PLUS_OFFSET]], i64 100
; ALL-NEXT: %[[RED_UPDATE_ISEMPTY:[^,]+]] = icmp eq i32* %[[GLOBAL_CAST_PLUS_OFFSET]], %[[GLOBAL_END]]
; ALL-NEXT: br i1 %[[RED_UPDATE_ISEMPTY]], label %[[RED_UPDATE_DONE:[^,]+]], label %[[RED_UPDATE_BODY:[^,]+]]
; ALL: [[RED_UPDATE_DONE]]:
; ALL: [[RED_UPDATE_BODY]]:
; ALL-NEXT: %[[CPY_DST_PTR:[^,]+]] = phi i32* [ %[[GLOBAL_CAST_PLUS_OFFSET]], %{{.*}} ], [ %[[CPY_DST_INC:[^,]+]], %[[RED_UPDATE_BODY]] ]
; ALL-NEXT: %[[CPY_SRC_PTR:[^,]+]] = phi i32* [ %[[REC_GEP]], %{{.*}} ], [ %[[CPY_SRC_INC:[^,]+]], %[[RED_UPDATE_BODY]] ]
; ALL-NEXT: %[[CPY_SRC_VAL:[^,]+]] = load i32, i32* %[[CPY_SRC_PTR]], align 4
; ALL-NEXT: %[[CPY_DST_VAL:[^,]+]] = load i32, i32* %[[CPY_DST_PTR]], align 4
; ALL-NEXT: %[[ADD:[^,]+]] = add i32 %[[CPY_DST_VAL]], %[[CPY_SRC_VAL]]
; ALL-NEXT: store i32 %[[ADD]], i32* %[[CPY_DST_PTR]], align 4
; ALL-NEXT: %[[CPY_DST_INC]] = getelementptr i32, i32* %[[CPY_DST_PTR]], i32 1
; ALL-NEXT: %[[CPY_SRC_INC]] = getelementptr i32, i32* %[[CPY_SRC_PTR]], i32 1
; ALL-NEXT: %[[CPY_DONE:[^,]+]] = icmp eq i32* %[[CPY_DST_INC]], %[[GLOBAL_END]]
; ALL-NEXT: br i1 %[[CPY_DONE]], label %[[RED_UPDATE_DONE]], label %[[RED_UPDATE_BODY]]
; ALL: call void @__kmpc_end_reduce(%struct.ident_t* @.kmpc_loc{{.*}}, i32 %my.tid{{.*}}, [8 x i32]* @{{.*}})
; ALL-NEXT: br label %tree.reduce.exit

  %10 = load i32, i32* %.omp.lb, align 4
  store i32 %10, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end10
  %11 = load i32, i32* %.omp.iv, align 4
  %12 = load i32, i32* %.omp.ub, align 4
  %cmp11 = icmp sle i32 %11, %12
  br i1 %cmp11, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %13 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %13, 1
  %add12 = add nsw i32 0, %mul
  store i32 %add12, i32* %i, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond13

for.cond13:                                       ; preds = %for.inc23, %omp.inner.for.body
  %14 = load i32, i32* %j, align 4
  %cmp14 = icmp slt i32 %14, 100
  br i1 %cmp14, label %for.body15, label %for.end25

for.body15:                                       ; preds = %for.cond13
  %15 = load i32, i32* %i, align 4
  %idxprom16 = sext i32 %15 to i64
  %arrayidx17 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %b, i64 0, i64 %idxprom16
  %16 = load i32, i32* %j, align 4
  %idxprom18 = sext i32 %16 to i64
  %arrayidx19 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx17, i64 0, i64 %idxprom18
  %17 = load i32, i32* %arrayidx19, align 4
  %18 = load i32, i32* %j, align 4
  %idxprom20 = sext i32 %18 to i64
  %arrayidx21 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom20
  %19 = load i32, i32* %arrayidx21, align 4
  %add22 = add nsw i32 %19, %17
  store i32 %add22, i32* %arrayidx21, align 4
  br label %for.inc23

for.inc23:                                        ; preds = %for.body15
  %20 = load i32, i32* %j, align 4
  %inc24 = add nsw i32 %20, 1
  store i32 %inc24, i32* %j, align 4
  br label %for.cond13

for.end25:                                        ; preds = %for.cond13
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end25
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %21 = load i32, i32* %.omp.iv, align 4
  %add26 = add nsw i32 %21, 1
  store i32 %add26, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
