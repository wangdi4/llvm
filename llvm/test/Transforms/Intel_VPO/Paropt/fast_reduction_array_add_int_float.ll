; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction-ctrl=0 -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction-ctrl=0 -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL


; #include <stdio.h>
;
; #define N 100
;
; int main(void)
; {
;   int i, j;
;   int a[N], b[N][N];
;   float c[N], d[N][N];
;   for (i=0; i<N; i++) {
;     a[i] = 0;
;     c[i] = 0.0e0;
;     for (j=0; j<N; j++) {
;       b[i][j] = i + j;
;       d[i][j] = i + j;
;     }
;   }
;
; #pragma omp parallel for reduction(+:a[0:N]) reduction(+:c[0:N]) private(j)
;   for (i=0; i<N; i++) {
;     for (j=0; j<N; j++) {
;       a[j] += b[i][j];
;       c[j] += d[i][j];
;     }
;   }
;
;   for (i=0; i<N; i++) {
;     printf("a[%d]=%d, c[%d]=%f\n", i, a[i], i, c[i]);
;   }
;
;   return 0;
; }


; ModuleID = 'fast_reduction_array_add_int_float.c'
source_filename = "fast_reduction_array_add_int_float.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [20 x i8] c"a[%d]=%d, c[%d]=%f\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %a = alloca [100 x i32], align 16
  %b = alloca [100 x [100 x i32]], align 16
  %c = alloca [100 x float], align 16
  %d = alloca [100 x [100 x float]], align 16
  %tmp = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc15, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 100
  br i1 %cmp, label %for.body, label %for.end17

for.body:                                         ; preds = %for.cond
  %1 = load i32, i32* %i, align 4
  %idxprom = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom
  store i32 0, i32* %arrayidx, align 4
  %2 = load i32, i32* %i, align 4
  %idxprom1 = sext i32 %2 to i64
  %arrayidx2 = getelementptr inbounds [100 x float], [100 x float]* %c, i64 0, i64 %idxprom1
  store float 0.000000e+00, float* %arrayidx2, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond3

for.cond3:                                        ; preds = %for.inc, %for.body
  %3 = load i32, i32* %j, align 4
  %cmp4 = icmp slt i32 %3, 100
  br i1 %cmp4, label %for.body5, label %for.end

for.body5:                                        ; preds = %for.cond3
  %4 = load i32, i32* %i, align 4
  %5 = load i32, i32* %j, align 4
  %add = add nsw i32 %4, %5
  %6 = load i32, i32* %i, align 4
  %idxprom6 = sext i32 %6 to i64
  %arrayidx7 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %b, i64 0, i64 %idxprom6
  %7 = load i32, i32* %j, align 4
  %idxprom8 = sext i32 %7 to i64
  %arrayidx9 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx7, i64 0, i64 %idxprom8
  store i32 %add, i32* %arrayidx9, align 4
  %8 = load i32, i32* %i, align 4
  %9 = load i32, i32* %j, align 4
  %add10 = add nsw i32 %8, %9
  %conv = sitofp i32 %add10 to float
  %10 = load i32, i32* %i, align 4
  %idxprom11 = sext i32 %10 to i64
  %arrayidx12 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* %d, i64 0, i64 %idxprom11
  %11 = load i32, i32* %j, align 4
  %idxprom13 = sext i32 %11 to i64
  %arrayidx14 = getelementptr inbounds [100 x float], [100 x float]* %arrayidx12, i64 0, i64 %idxprom13
  store float %conv, float* %arrayidx14, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body5
  %12 = load i32, i32* %j, align 4
  %inc = add nsw i32 %12, 1
  store i32 %inc, i32* %j, align 4
  br label %for.cond3

for.end:                                          ; preds = %for.cond3
  br label %for.inc15

for.inc15:                                        ; preds = %for.end
  %13 = load i32, i32* %i, align 4
  %inc16 = add nsw i32 %13, 1
  store i32 %inc16, i32* %i, align 4
  br label %for.cond

for.end17:                                        ; preds = %for.cond
  store i32 0, i32* %.omp.lb, align 4
  store i32 99, i32* %.omp.ub, align 4
  %14 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([100 x i32]* %a, i64 1, i64 0, i64 100, i64 1), "QUAL.OMP.REDUCTION.ADD:ARRSECT"([100 x float]* %c, i64 1, i64 0, i64 100, i64 1), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"([100 x [100 x i32]]* %b), "QUAL.OMP.SHARED"([100 x [100 x float]]* %d), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]


; ALL-NOT: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
; ALL-NOT: __kmpc_atomic
; ALL: %struct.fast_red_t = type <{ [100 x i32], [100 x float] }>
; ALL: define internal void @main_tree_reduce_{{.*}}(i8* %dst, i8* %src) {
; ALL: declare i32 @__kmpc_reduce(%struct.ident_t*, i32, i32, i32, i8*, void (i8*, i8*)*, [8 x i32]*)
; ALL: declare void @__kmpc_end_reduce(%struct.ident_t*, i32, [8 x i32]*)
; ALL: %fast_red_struct{{.*}} = alloca %struct.fast_red_t, align 16

; USE-REC: %[[REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
; USE-REC-NEXT: %[[REC_GEP:[^,]+]] = getelementptr inbounds [100 x i32], [100 x i32]* %[[REC]], i32 0, i32 0
; USE-REC-NEXT: %[[REC_GEP_MINUS_OFFSET:[^,]+]] = getelementptr i32, i32* %[[REC_GEP]], i64 0
; USE-REC-NEXT: %[[REC_GEP_MINUS_OFFSET_CAST:[^,]+]] = bitcast i32* %[[REC_GEP_MINUS_OFFSET]] to [100 x i32]*
; USE-REC-NEXT: %[[FP_REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 1
; USE-REC-NEXT: %[[FP_REC_GEP:[^,]+]] = getelementptr inbounds [100 x float], [100 x float]* %[[FP_REC]], i32 0, i32 0
; USE-REC-NEXT: %[[FP_REC_GEP_MINUS_OFFSET:[^,]+]] = getelementptr float, float* %[[FP_REC_GEP]], i64 0
; USE-REC-NEXT: %[[FP_REC_GEP_MINUS_OFFSET_CAST:[^,]+]] = bitcast float* %[[FP_REC_GEP_MINUS_OFFSET]] to [100 x float]*
; USE-REC: %[[FP_END:[^,]+]] = getelementptr float, float* %[[FP_REC_GEP]], i64 100
; USE-REC-NEXT: %[[FP_ISEMPTY:[^,]+]] = icmp eq float* %[[FP_REC_GEP]], %[[FP_END]]
; USE-REC-NEXT: br i1 %[[FP_ISEMPTY]], label %[[FP_DONE:[^,]+]], label %[[FP_BODY:[^,]+]]
; USE-REC: [[FP_DONE]]:
; USE-REC: [[FP_BODY]]:
; USE-REC: store float 0.000000e+00, float* %{{.*}}, align 4

; USE-REC: %[[END:[^,]+]] = getelementptr i32, i32* %[[REC_GEP]], i64 100
; USE-REC-NEXT: %[[ISEMPTY:[^,]+]] = icmp eq i32* %[[REC_GEP]], %[[END]]
; USE-REC-NEXT: br i1 %[[ISEMPTY]], label %[[DONE:[^,]+]], label %[[BODY:[^,]+]]
; USE-REC: [[DONE]]:
; USE-REC: [[BODY]]:
; USE-REC: store i32 0, i32* %{{.*}}, align 4


; USE-LOCAL: %[[LOCAL:[^,]+]] = alloca [100 x i32], align 16
; USE-LOCAL-NEXT: %[[FP_LOCAL:[^,]+]] = alloca [100 x float], align 16
; USE-LOCAL: %[[LOCAL_GEP:[^,]+]] = getelementptr inbounds [100 x i32], [100 x i32]* %[[LOCAL]], i32 0, i32 0
; USE-LOCAL-NEXT: %[[LOCAL_GEP_MINUS_OFFSET:[^,]+]] = getelementptr i32, i32* %[[LOCAL_GEP]], i64 0
; USE-LOCAL-NEXT: %[[LOCAL_GEP_MINUS_OFFSET_CAST:[^,]+]] = bitcast i32* %[[LOCAL_GEP_MINUS_OFFSET]] to [100 x i32]*
; USE-LOCAL-NEXT: %[[FP_LOCAL_GEP:[^,]+]] = getelementptr inbounds [100 x float], [100 x float]* %[[FP_LOCAL]], i32 0, i32 0
; USE-LOCAL-NEXT: %[[FP_LOCAL_GEP_MINUS_OFFSET:[^,]+]] = getelementptr float, float* %[[FP_LOCAL_GEP]], i64 0
; USE-LOCAL-NEXT: %[[FP_LOCAL_GEP_MINUS_OFFSET_CAST:[^,]+]] = bitcast float* %[[FP_LOCAL_GEP_MINUS_OFFSET]] to [100 x float]*

; USE-LOCAL: %[[FP_REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 1
; USE-LOCAL-NEXT: %[[FP_REC_GEP:[^,]+]] = getelementptr inbounds [100 x float], [100 x float]* %[[FP_REC]], i32 0, i32 0

; USE-LOCAL: %[[FP_END:[^,]+]] = getelementptr float, float* %[[FP_LOCAL_GEP]], i64 100
; USE-LOCAL-NEXT: %[[FP_ISEMPTY:[^,]+]] = icmp eq float* %[[FP_LOCAL_GEP]], %[[FP_END]]
; USE-LOCAL-NEXT: br i1 %[[FP_ISEMPTY]], label %[[FP_DONE:[^,]+]], label %[[FP_BODY:[^,]+]]
; USE-LOCAL: [[FP_DONE]]:
; USE-LOCAL: [[FP_BODY]]:

; USE-LOCAL: %[[REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
; USE-LOCAL-NEXT: %[[REC_GEP:[^,]+]] = getelementptr inbounds [100 x i32], [100 x i32]* %[[REC]], i32 0, i32 0

; USE-LOCAL: %[[END:[^,]+]] = getelementptr i32, i32* %[[LOCAL_GEP]], i64 100
; USE-LOCAL-NEXT: %[[ISEMPTY:[^,]+]] = icmp eq i32* %[[LOCAL_GEP]], %[[END]]
; USE-LOCAL-NEXT: br i1 %[[ISEMPTY]], label %[[DONE:[^,]+]], label %[[BODY:[^,]+]]
; USE-LOCAL: [[DONE]]:
; USE-LOCAL: [[BODY]]:

; ALL: call void @__kmpc_for_static_fini(%struct.ident_t* @{{.*}}, i32 %{{.*}})
; ALL: %[[BITCAST:[^,]+]] = bitcast %struct.fast_red_t* %fast_red_struct to i8*
; ALL: %[[RET:[^,]+]] = call i32 @__kmpc_reduce(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 2, i32 800, i8* %[[BITCAST]], void (i8*, i8*)* @main_tree_reduce_{{.*}}, [8 x i32]* @{{.*}})
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


; ALL: %[[FP_GLOBAL_CAST:[^,]+]] = bitcast [100 x float]* %[[FP_GLOBAL:[^,]+]] to float*
; ALL-NEXT: %[[FP_GLOBAL_CAST_PLUS_OFFSET:[^,]+]] = getelementptr float, float* %[[FP_GLOBAL_CAST]], i64 0
; ALL: %[[FP_GLOBAL_END:[^,]+]] = getelementptr float, float* %[[FP_GLOBAL_CAST_PLUS_OFFSET]], i64 100
; ALL-NEXT: %[[FP_RED_UPDATE_ISEMPTY:[^,]+]] = icmp eq float* %[[FP_GLOBAL_CAST_PLUS_OFFSET]], %[[FP_GLOBAL_END]]
; ALL-NEXT: br i1 %[[FP_RED_UPDATE_ISEMPTY]], label %[[FP_RED_UPDATE_DONE:[^,]+]], label %[[FP_RED_UPDATE_BODY:[^,]+]]
; ALL: [[FP_RED_UPDATE_DONE]]:
; ALL: [[FP_RED_UPDATE_BODY]]:
; ALL-NEXT: %[[FP_CPY_DST_PTR:[^,]+]] = phi float* [ %[[FP_GLOBAL_CAST_PLUS_OFFSET]], %{{.*}} ], [ %[[FP_CPY_DST_INC:[^,]+]], %[[FP_RED_UPDATE_BODY]] ]
; ALL-NEXT: %[[FP_CPY_SRC_PTR:[^,]+]] = phi float* [ %[[FP_REC_GEP]], %{{.*}} ], [ %[[FP_CPY_SRC_INC:[^,]+]], %[[FP_RED_UPDATE_BODY]] ]
; ALL-NEXT: %[[FP_CPY_SRC_VAL:[^,]+]] = load float, float* %[[FP_CPY_SRC_PTR]], align 4
; ALL-NEXT: %[[FP_CPY_DST_VAL:[^,]+]] = load float, float* %[[FP_CPY_DST_PTR]], align 4
; ALL-NEXT: %[[FP_ADD:[^,]+]] = fadd float %[[FP_CPY_DST_VAL]], %[[FP_CPY_SRC_VAL]]
; ALL-NEXT: store float %[[FP_ADD]], float* %[[FP_CPY_DST_PTR]], align 4
; ALL-NEXT: %[[FP_CPY_DST_INC]] = getelementptr float, float* %[[FP_CPY_DST_PTR]], i32 1
; ALL-NEXT: %[[FP_CPY_SRC_INC]] = getelementptr float, float* %[[FP_CPY_SRC_PTR]], i32 1
; ALL-NEXT: %[[FP_CPY_DONE:[^,]+]] = icmp eq float* %[[FP_CPY_DST_INC]], %[[FP_GLOBAL_END]]
; ALL-NEXT: br i1 %[[FP_CPY_DONE]], label %[[FP_RED_UPDATE_DONE]], label %[[FP_RED_UPDATE_BODY]]

; ALL: call void @__kmpc_end_reduce(%struct.ident_t* @.kmpc_loc{{.*}}, i32 %my.tid{{.*}}, [8 x i32]* @{{.*}})
; ALL-NEXT: br label %tree.reduce.exit

  %15 = load i32, i32* %.omp.lb, align 4
  store i32 %15, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %for.end17
  %16 = load i32, i32* %.omp.iv, align 4
  %17 = load i32, i32* %.omp.ub, align 4
  %cmp18 = icmp sle i32 %16, %17
  br i1 %cmp18, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %18 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %18, 1
  %add20 = add nsw i32 0, %mul
  store i32 %add20, i32* %i, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond21

for.cond21:                                       ; preds = %for.inc39, %omp.inner.for.body
  %19 = load i32, i32* %j, align 4
  %cmp22 = icmp slt i32 %19, 100
  br i1 %cmp22, label %for.body24, label %for.end41

for.body24:                                       ; preds = %for.cond21
  %20 = load i32, i32* %i, align 4
  %idxprom25 = sext i32 %20 to i64
  %arrayidx26 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* %b, i64 0, i64 %idxprom25
  %21 = load i32, i32* %j, align 4
  %idxprom27 = sext i32 %21 to i64
  %arrayidx28 = getelementptr inbounds [100 x i32], [100 x i32]* %arrayidx26, i64 0, i64 %idxprom27
  %22 = load i32, i32* %arrayidx28, align 4
  %23 = load i32, i32* %j, align 4
  %idxprom29 = sext i32 %23 to i64
  %arrayidx30 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom29
  %24 = load i32, i32* %arrayidx30, align 4
  %add31 = add nsw i32 %24, %22
  store i32 %add31, i32* %arrayidx30, align 4
  %25 = load i32, i32* %i, align 4
  %idxprom32 = sext i32 %25 to i64
  %arrayidx33 = getelementptr inbounds [100 x [100 x float]], [100 x [100 x float]]* %d, i64 0, i64 %idxprom32
  %26 = load i32, i32* %j, align 4
  %idxprom34 = sext i32 %26 to i64
  %arrayidx35 = getelementptr inbounds [100 x float], [100 x float]* %arrayidx33, i64 0, i64 %idxprom34
  %27 = load float, float* %arrayidx35, align 4
  %28 = load i32, i32* %j, align 4
  %idxprom36 = sext i32 %28 to i64
  %arrayidx37 = getelementptr inbounds [100 x float], [100 x float]* %c, i64 0, i64 %idxprom36
  %29 = load float, float* %arrayidx37, align 4
  %add38 = fadd float %29, %27
  store float %add38, float* %arrayidx37, align 4
  br label %for.inc39

for.inc39:                                        ; preds = %for.body24
  %30 = load i32, i32* %j, align 4
  %inc40 = add nsw i32 %30, 1
  store i32 %inc40, i32* %j, align 4
  br label %for.cond21

for.end41:                                        ; preds = %for.cond21
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end41
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %31 = load i32, i32* %.omp.iv, align 4
  %add42 = add nsw i32 %31, 1
  store i32 %add42, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %14) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  store i32 0, i32* %i, align 4
  br label %for.cond43

for.cond43:                                       ; preds = %for.inc52, %omp.loop.exit
  %32 = load i32, i32* %i, align 4
  %cmp44 = icmp slt i32 %32, 100
  br i1 %cmp44, label %for.body46, label %for.end54

for.body46:                                       ; preds = %for.cond43
  %33 = load i32, i32* %i, align 4
  %34 = load i32, i32* %i, align 4
  %idxprom47 = sext i32 %34 to i64
  %arrayidx48 = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 %idxprom47
  %35 = load i32, i32* %arrayidx48, align 4
  %36 = load i32, i32* %i, align 4
  %37 = load i32, i32* %i, align 4
  %idxprom49 = sext i32 %37 to i64
  %arrayidx50 = getelementptr inbounds [100 x float], [100 x float]* %c, i64 0, i64 %idxprom49
  %38 = load float, float* %arrayidx50, align 4
  %conv51 = fpext float %38 to double
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i64 0, i64 0), i32 %33, i32 %35, i32 %36, double %conv51)
  br label %for.inc52

for.inc52:                                        ; preds = %for.body46
  %39 = load i32, i32* %i, align 4
  %inc53 = add nsw i32 %39, 1
  store i32 %inc53, i32* %i, align 4
  br label %for.cond43

for.end54:                                        ; preds = %for.cond43
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
