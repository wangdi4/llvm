; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-LOCAL --check-prefix=ALL
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-fast-reduction-ctrl=0 -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-fast-reduction-ctrl=0 -vpo-paropt-keep-blocks-order=false -S %s | FileCheck %s --check-prefix=USE-REC --check-prefix=ALL


;
; #define N 100
;
; int foo(int *a, int n)
; {
;   int i, j;
;   int b[n][n];
;   for (i=0; i<n; i++) {
;     a[i] = 0;
;     for (j=0; j<n; j++) {
;       b[i][j] = i + j;
;     }
;   }
;
; #pragma omp parallel for reduction(+:a[0:n]) private(j)
;   for (i=0; i<n; i++) {
;     for (j=0; j<n; j++) {
;       a[j] += b[i][j];
;     }
;   }
; }
;
; int main(void)
; {
;   int i;
;   int a[N] = {0};
;   foo(a, N);
;
;   return 0;
; }


; ModuleID = 'fast_reduction_array_section_varlen_int.c'
source_filename = "fast_reduction_array_section_varlen_int.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(i32* %a, i32 %n) #0 {
entry:
  %retval = alloca i32, align 4
  %a.addr = alloca i32*, align 8
  %n.addr = alloca i32, align 4
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  %saved_stack = alloca i8*, align 8
  %__vla_expr0 = alloca i64, align 8
  %__vla_expr1 = alloca i64, align 8
  %tmp = alloca i32, align 4
  %.capture_expr. = alloca i32, align 4
  %.capture_expr.11 = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.lb = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %omp.vla.tmp = alloca i64, align 8
  %omp.vla.tmp16 = alloca i64, align 8
  store i32* %a, i32** %a.addr, align 8
  store i32 %n, i32* %n.addr, align 4
  %0 = load i32, i32* %n.addr, align 4
  %1 = zext i32 %0 to i64
  %2 = load i32, i32* %n.addr, align 4
  %3 = zext i32 %2 to i64
  %4 = call i8* @llvm.stacksave()
  store i8* %4, i8** %saved_stack, align 8
  %5 = mul nuw i64 %1, %3
  %vla = alloca i32, i64 %5, align 16
  store i64 %1, i64* %__vla_expr0, align 8
  store i64 %3, i64* %__vla_expr1, align 8
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc8, %entry
  %6 = load i32, i32* %i, align 4
  %7 = load i32, i32* %n.addr, align 4
  %cmp = icmp slt i32 %6, %7
  br i1 %cmp, label %for.body, label %for.end10

for.body:                                         ; preds = %for.cond
  %8 = load i32*, i32** %a.addr, align 8
  %9 = load i32, i32* %i, align 4
  %idxprom = sext i32 %9 to i64
  %arrayidx = getelementptr inbounds i32, i32* %8, i64 %idxprom
  store i32 0, i32* %arrayidx, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc, %for.body
  %10 = load i32, i32* %j, align 4
  %11 = load i32, i32* %n.addr, align 4
  %cmp2 = icmp slt i32 %10, %11
  br i1 %cmp2, label %for.body3, label %for.end

for.body3:                                        ; preds = %for.cond1
  %12 = load i32, i32* %i, align 4
  %13 = load i32, i32* %j, align 4
  %add = add nsw i32 %12, %13
  %14 = load i32, i32* %i, align 4
  %idxprom4 = sext i32 %14 to i64
  %15 = mul nsw i64 %idxprom4, %3
  %arrayidx5 = getelementptr inbounds i32, i32* %vla, i64 %15
  %16 = load i32, i32* %j, align 4
  %idxprom6 = sext i32 %16 to i64
  %arrayidx7 = getelementptr inbounds i32, i32* %arrayidx5, i64 %idxprom6
  store i32 %add, i32* %arrayidx7, align 4
  br label %for.inc

for.inc:                                          ; preds = %for.body3
  %17 = load i32, i32* %j, align 4
  %inc = add nsw i32 %17, 1
  store i32 %inc, i32* %j, align 4
  br label %for.cond1

for.end:                                          ; preds = %for.cond1
  br label %for.inc8

for.inc8:                                         ; preds = %for.end
  %18 = load i32, i32* %i, align 4
  %inc9 = add nsw i32 %18, 1
  store i32 %inc9, i32* %i, align 4
  br label %for.cond

for.end10:                                        ; preds = %for.cond
  %19 = load i32, i32* %n.addr, align 4
  store i32 %19, i32* %.capture_expr., align 4
  %20 = load i32, i32* %.capture_expr., align 4
  %sub = sub nsw i32 %20, 0
  %sub12 = sub nsw i32 %sub, 1
  %add13 = add nsw i32 %sub12, 1
  %div = sdiv i32 %add13, 1
  %sub14 = sub nsw i32 %div, 1
  store i32 %sub14, i32* %.capture_expr.11, align 4
  %21 = load i32, i32* %.capture_expr., align 4
  %cmp15 = icmp slt i32 0, %21
  br i1 %cmp15, label %omp.precond.then, label %omp.precond.end

omp.precond.then:                                 ; preds = %for.end10
  store i32 0, i32* %.omp.lb, align 4
  %22 = load i32, i32* %.capture_expr.11, align 4
  store i32 %22, i32* %.omp.ub, align 4
  store i64 %1, i64* %omp.vla.tmp, align 8
  store i64 %3, i64* %omp.vla.tmp16, align 8
  %23 = load i32, i32* %n.addr, align 4
  %conv = sext i32 %23 to i64
  %24 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD:ARRSECT"(i32** %a.addr, i64 1, i64 0, i64 %conv, i64 1), "QUAL.OMP.PRIVATE"(i32* %j), "QUAL.OMP.SHARED"(i32* %n.addr), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.SHARED"(i32* %vla), "QUAL.OMP.NORMALIZED.IV"(i32* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i32* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub), "QUAL.OMP.SHARED"(i64* %omp.vla.tmp), "QUAL.OMP.SHARED"(i64* %omp.vla.tmp16) ]

; ALL-NOT: "QUAL.OMP.REDUCTION.ADD:ARRSECT"
; ALL-NOT: __kmpc_atomic
; ALL: %struct.fast_red_t = type <{ i32* }>
; ALL: define internal void @foo_tree_reduce_{{.*}}(i8* %dst, i8* %src) {
; ALL-NEXT: entry:
; ALL-NEXT: %dst.cast = bitcast i8* %dst to %struct.fast_red_t*
; ALL-NEXT: %src.cast = bitcast i8* %src to %struct.fast_red_t*
; ALL-NEXT: %dst.a.addr = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %dst.cast, i32 0, i32 0
; ALL-NEXT: %src.a.addr = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %src.cast, i32 0, i32 0
; ALL-NEXT: %[[DST_PTR:[^,]+]] = load i32*, i32** %dst.a.addr, align 8
; ALL-NEXT: %[[SRC_PTR:[^,]+]] = load i32*, i32** %src.a.addr, align 8
; ALL-NEXT: %[[ARRSEC_SIZE_LOAD:[^,]+]] = load i64, i64* @[[ARRSEC_SIZE:[^,]+]], align 8
; ALL-NEXT: %[[DST_END:[^,]+]] = getelementptr i32, i32* %[[DST_PTR]], i64 %[[ARRSEC_SIZE_LOAD]]
; ALL-NEXT: %red.update.isempty = icmp eq i32* %[[DST_PTR]], %[[DST_END]]
; ALL-NEXT: br i1 %red.update.isempty, label %red.update.done, label %red.update.body
; ALL: red.update.body:
; ALL-NEXT: %red.cpy.dest.ptr = phi i32* [ %[[DST_PTR]], %entry ], [ %red.cpy.dest.inc, %red.update.body ]
; ALL-NEXT: %red.cpy.src.ptr = phi i32* [ %[[SRC_PTR]], %entry ], [ %red.cpy.src.inc, %red.update.body ]
; ALL-NEXT: %[[SRC:[^,]+]] = load i32, i32* %red.cpy.src.ptr, align 4
; ALL-NEXT: %[[DST:[^,]+]] = load i32, i32* %red.cpy.dest.ptr, align 4
; ALL-NEXT: %[[ADD:[^,]+]] = add i32 %[[DST]], %[[SRC]]
; ALL-NEXT: store i32 %[[ADD]], i32* %red.cpy.dest.ptr, align 4
; ALL-NEXT: %red.cpy.dest.inc = getelementptr i32, i32* %red.cpy.dest.ptr, i32 1
; ALL-NEXT: %red.cpy.src.inc = getelementptr i32, i32* %red.cpy.src.ptr, i32 1
; ALL-NEXT: %red.cpy.done = icmp eq i32* %red.cpy.dest.inc, %[[DST_END]]
; ALL-NEXT: br i1 %red.cpy.done, label %red.update.done, label %red.update.body
; ALL: red.update.done:
; ALL: declare i32 @__kmpc_reduce(%struct.ident_t*, i32, i32, i32, i8*, void (i8*, i8*)*, [8 x i32]*)
; ALL: declare void @__kmpc_end_reduce(%struct.ident_t*, i32, [8 x i32]*)
; ALL: %fast_red_struct{{.*}} = alloca %struct.fast_red_t, align 8

; USE-LOCAL: %[[LOCAL_MINUS_OFFSET_ADDR:[^,]+]] = alloca i32*, align 8
; USE-LOCAL: %{{.*}} = alloca i32*, align 8

; ALL: %[[SIZE:[^,]+]] = mul i64 1, %{{.*}}
; ALL-NEXT: store i64 %[[SIZE]], i64* @[[ARRSEC_SIZE]], align 8
; ALL-NEXT: %[[REC:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
; ALL-NEXT: %[[REC_ALLOCA:[^,]+]] = alloca i32, i64 %[[SIZE]], align 8
; ALL-NEXT: store i32* %[[REC_ALLOCA]], i32** %[[REC]], align 8
; USE-LOCAL: %[[LOCAL:[^,]+]] = alloca i32, i64 %[[SIZE]], align 8
; USE-LOCAL-NEXT: %[[LOCAL_MINUS_OFFSET:[^,]+]] = getelementptr i32, i32* %[[LOCAL]], i64 0
; USE-LOCAL-NEXT: store i32* %[[LOCAL_MINUS_OFFSET]], i32** %[[LOCAL_MINUS_OFFSET_ADDR]], align 8
; ALL: %[[REC2:[^,]+]] = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
; ALL-NEXT: %[[REC_LOAD:[^,]+]] = load i32*, i32** %[[REC2]], align 8

; USE-REC:  %[[END:[^,]+]] = getelementptr i32, i32* %[[REC_LOAD]], i64 %[[SIZE]]
; USE-REC-NEXT: %[[ISEMPTY:[^,]+]] = icmp eq i32* %[[REC_LOAD]], %[[END]]

; USE-LOCAL: %[[END:[^,]+]] = getelementptr i32, i32* %[[LOCAL]], i64 %[[SIZE]]
; USE-LOCAL-NEXT: %[[ISEMPTY:[^,]+]] = icmp eq i32* %[[LOCAL]], %[[END]]

; ALL: br i1 %[[ISEMPTY]], label %[[DONE:[^,]+]], label %[[BODY:[^,]+]]
; ALL: [[DONE]]:
; ALL: [[BODY]]:

; USE-LOCAL: %[[REC_MINUS_OFFSET:[^,]+]] = getelementptr i32, i32* %[[REC_LOAD]], i64 0
; USE-LOCAL-NEXT: store i32* %[[REC_MINUS_OFFSET]], i32** %[[REC_MINUS_OFFSET_ADDR:[^,]+]], align 8
; USE-LOCAL-NEXT: %[[REC_MINUS_OFFSET_LOAD:[^,]+]] = load i32*, i32** %[[REC_MINUS_OFFSET_ADDR]], align 8
; USE-LOCAL-NEXT: %[[REC_MINUS_OFFSET_LOAD_PLUS_OFFSET:[^,]+]] = getelementptr i32, i32* %[[REC_MINUS_OFFSET_LOAD]], i64 0
; USE-LOCAL: %[[REC_END:[^,]+]] = getelementptr i32, i32* %[[REC_MINUS_OFFSET_LOAD_PLUS_OFFSET]], i64 %[[SIZE]]
; USE-LOCAL-NEXT: %[[FASTRED_UPDATE_ISEMPTY:[^,]+]] = icmp eq i32* %[[REC_MINUS_OFFSET_LOAD_PLUS_OFFSET]], %[[REC_END]]
; USE-LOCAL-NEXT: br i1 %[[FASTRED_UPDATE_ISEMPTY]], label %[[FASTRED_UPDATE_DONE:[^,]+]], label %[[FASTRED_UPDATE_BODY:[^,]+]]
; USE-LOCAL: [[FASTRED_UPDATE_DONE]]:
; USE-LOCAL: [[FASTRED_UPDATE_BODY]]:
; USE-LOCAL-NEXT: %[[FASTRED_DST_PTR:[^,]+]] = phi i32* [ %{{.*}}, %{{.*}} ], [ %[[FASTRED_DST_INC:[^,]+]], %[[FASTRED_UPDATE_BODY]] ]
; USE-LOCAL-NEXT: %[[FASTRED_SRC_PTR:[^,]+]] = phi i32* [ %[[LOCAL]], %{{.*}} ], [ %[[FASTRED_SRC_INC:[^,]+]], %[[FASTRED_UPDATE_BODY]] ]
; USE-LOCAL-NEXT: %[[FASTRED_SRC_PTR_LOAD:[^,]+]] = load i32, i32* %[[FASTRED_SRC_PTR]], align 4
; USE-LOCAL-NEXT: store i32 %[[FASTRED_SRC_PTR_LOAD]], i32* %[[FASTRED_DST_PTR]], align 4
; USE-LOCAL-NEXT: %[[FASTRED_DST_INC]] = getelementptr i32, i32* %[[FASTRED_DST_PTR]], i32 1
; USE-LOCAL-NEXT: %[[FASTRED_SRC_INC]] = getelementptr i32, i32* %[[FASTRED_SRC_PTR]], i32 1
; USE-LOCAL-NEXT: %[[FASTRED_DONE:[^,]+]] = icmp eq i32* %[[FASTRED_DST_INC]], %[[REC_END]]
; USE-LOCAL-NEXT: br i1 %[[FASTRED_DONE]], label %[[FASTRED_UPDATE_DONE]], label %[[FASTRED_UPDATE_BODY]]

; ALL: call void @__kmpc_for_static_fini(%struct.ident_t* @{{.*}}, i32 %{{.*}})
; ALL: %[[BITCAST:[^,]+]] = bitcast %struct.fast_red_t* %fast_red_struct to i8*
; ALL: %[[RET:[^,]+]] = call i32 @__kmpc_reduce(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 1, i32 8, i8* %[[BITCAST]], void (i8*, i8*)* @foo_tree_reduce_{{.*}}, [8 x i32]* @{{.*}})
; ALL-NEXT: %to.tree.reduce = icmp eq i32 %[[RET]], 1
; ALL-NEXT: br i1 %to.tree.reduce, label %tree.reduce, label %tree.reduce.exit
; ALL: tree.reduce:
; ALL: tree.reduce.exit:
; ALL: %[[GLOBAL_LOAD:[^,]+]] = load i32*, i32** %[[GLOBAL_ADDR:[^,]+]], align 8
; ALL-NEXT: %[[GLOBAL_PLUS_OFFSET:[^,]+]] = getelementptr i32, i32* %[[GLOBAL_LOAD]], i64 0
; ALL: %[[GLOBAL_END:[^,]+]] = getelementptr i32, i32* %[[GLOBAL_PLUS_OFFSET]], i64 %[[SIZE]]
; ALL-NEXT: %[[RED_UPDATE_ISEMPTY:[^,]+]] = icmp eq i32* %[[GLOBAL_PLUS_OFFSET]], %[[GLOBAL_END]]
; ALL-NEXT: br i1 %[[RED_UPDATE_ISEMPTY]], label %[[RED_UPDATE_DONE:[^,]+]], label %[[RED_UPDATE_BODY:[^,]+]]
; ALL: [[RED_UPDATE_DONE]]:
; ALL: [[RED_UPDATE_BODY]]:
; ALL-NEXT: %[[CPY_DST_PTR:[^,]+]] = phi i32* [ %[[GLOBAL_PLUS_OFFSET]], %{{.*}} ], [ %[[CPY_DST_INC:[^,]+]], %[[RED_UPDATE_BODY]] ]
; ALL-NEXT: %[[CPY_SRC_PTR:[^,]+]] = phi i32* [ %[[REC_LOAD]], %{{.*}} ], [ %[[CPY_SRC_INC:[^,]+]], %[[RED_UPDATE_BODY]] ]
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

  %25 = load i64, i64* %omp.vla.tmp, align 8
  %26 = load i64, i64* %omp.vla.tmp16, align 8
  %27 = load i32, i32* %.omp.lb, align 4
  store i32 %27, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.inc, %omp.precond.then
  %28 = load i32, i32* %.omp.iv, align 4
  %29 = load i32, i32* %.omp.ub, align 4
  %cmp17 = icmp sle i32 %28, %29
  br i1 %cmp17, label %omp.inner.for.body, label %omp.inner.for.end

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %30 = load i32, i32* %.omp.iv, align 4
  %mul = mul nsw i32 %30, 1
  %add19 = add nsw i32 0, %mul
  store i32 %add19, i32* %i, align 4
  store i32 0, i32* %j, align 4
  br label %for.cond20

for.cond20:                                       ; preds = %for.inc31, %omp.inner.for.body
  %31 = load i32, i32* %j, align 4
  %32 = load i32, i32* %n.addr, align 4
  %cmp21 = icmp slt i32 %31, %32
  br i1 %cmp21, label %for.body23, label %for.end33

for.body23:                                       ; preds = %for.cond20
  %33 = load i32, i32* %i, align 4
  %idxprom24 = sext i32 %33 to i64
  %34 = mul nsw i64 %idxprom24, %26
  %arrayidx25 = getelementptr inbounds i32, i32* %vla, i64 %34
  %35 = load i32, i32* %j, align 4
  %idxprom26 = sext i32 %35 to i64
  %arrayidx27 = getelementptr inbounds i32, i32* %arrayidx25, i64 %idxprom26
  %36 = load i32, i32* %arrayidx27, align 4
  %37 = load i32*, i32** %a.addr, align 8
  %38 = load i32, i32* %j, align 4
  %idxprom28 = sext i32 %38 to i64
  %arrayidx29 = getelementptr inbounds i32, i32* %37, i64 %idxprom28
  %39 = load i32, i32* %arrayidx29, align 4
  %add30 = add nsw i32 %39, %36
  store i32 %add30, i32* %arrayidx29, align 4
  br label %for.inc31

for.inc31:                                        ; preds = %for.body23
  %40 = load i32, i32* %j, align 4
  %inc32 = add nsw i32 %40, 1
  store i32 %inc32, i32* %j, align 4
  br label %for.cond20

for.end33:                                        ; preds = %for.cond20
  br label %omp.body.continue

omp.body.continue:                                ; preds = %for.end33
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %omp.body.continue
  %41 = load i32, i32* %.omp.iv, align 4
  %add34 = add nsw i32 %41, 1
  store i32 %add34, i32* %.omp.iv, align 4
  br label %omp.inner.for.cond

omp.inner.for.end:                                ; preds = %omp.inner.for.cond
  br label %omp.loop.exit

omp.loop.exit:                                    ; preds = %omp.inner.for.end
  call void @llvm.directive.region.exit(token %24) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %omp.loop.exit, %for.end10
  %42 = load i8*, i8** %saved_stack, align 8
  call void @llvm.stackrestore(i8* %42)
  %43 = load i32, i32* %retval, align 4
  ret i32 %43
}

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %i = alloca i32, align 4
  %a = alloca [100 x i32], align 16
  store i32 0, i32* %retval, align 4
  %0 = bitcast [100 x i32]* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* align 16 %0, i8 0, i64 400, i1 false)
  %arraydecay = getelementptr inbounds [100 x i32], [100 x i32]* %a, i64 0, i64 0
  %call = call i32 @foo(i32* %arraydecay, i32 100)
  ret i32 0
}

; Function Attrs: argmemonly nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind willreturn writeonly }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
