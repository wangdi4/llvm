; RUN: opt %s -S -passes=vplan-vec -vplan-nested-simd-strategy=outermost 2>&1 | FileCheck %s
; RUN: opt %s -disable-output -passes=vplan-vec -vplan-nested-simd-strategy=innermost -vplan-print-after-vpentity-instrs -mtriple=x86_64-unknown-linux-gnu | FileCheck %s --check-prefix=INNER

define i32 @main() {
;
; Verify that vectorizer bails out from outer loop vectorization if inner loop has a reduction.
; CHECK-NOT: VPlan
;
; INNER:     ptr [[VP_D_LINEAR_IV:%.*]] = allocate-priv i32, OrigAlign = 4
; INNER:     ptr [[VP_TMP1_RED:%.*]] = allocate-priv i32, OrigAlign = 4
; INNER:     i32 [[VP_LOAD:%.*]] = load ptr [[TMP1_RED0:%.*]]
; INNER:     i32 [[VP_TMP1_REDRED_INIT:%.*]] = reduction-init i32 0 i32 [[VP_LOAD]]
; INNER:     store i32 [[VP_TMP1_REDRED_INIT]] ptr [[VP_TMP1_RED]]
;
; INNER:     i32 [[VP2:%.*]] = load ptr [[VP_D_LINEAR_IV]]
; INNER:     i32 [[VP3:%.*]] = load ptr [[VP_TMP1_RED]]
; INNER:     i32 [[VP_ADD14:%.*]] = add i32 [[VP3]] i32 [[VP2]]
; INNER:     store i32 [[VP_ADD14]] ptr [[VP_TMP1_RED]]
;
; INNER:     i32 [[VP_LOAD_2:%.*]] = load ptr [[VP_TMP1_RED]]
; INNER:     i32 [[VP_TMP1_REDRED_FINAL:%.*]] = reduction-final{u_add} i32 [[VP_LOAD_2]]
; INNER:     store i32 [[VP_TMP1_REDRED_FINAL]] ptr [[TMP1_RED0]]
;
entry:
  %tmp1.red = alloca i32, i32 0, align 4
  %d.linear.iv = alloca i32, align 4
  br label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.cond9.omp.inner.for.end_crit_edge, %omp.inner.for.body.lr.ph
  br label %omp.inner.for.body11.lr.ph

omp.inner.for.body11.lr.ph:                       ; preds = %omp.inner.for.body
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD:TYPED"(ptr %tmp1.red, i32 0, i32 1), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %d.linear.iv, i32 0, i32 1, i32 0) ]
  br label %omp.inner.for.body11

omp.inner.for.body11:                             ; preds = %omp.inner.for.body11, %omp.inner.for.body11.lr.ph
  %.omp.iv7.local.044 = phi i32 [ 0, %omp.inner.for.body11.lr.ph ], [ %add15, %omp.inner.for.body11 ]
  %2 = load i32, ptr %d.linear.iv, align 4
  %3 = load i32, ptr %tmp1.red, align 4
  %add14 = add i32 %3, %2
  store i32 %add14, ptr %tmp1.red, align 4
  %add15 = add i32 %.omp.iv7.local.044, 1
  %cmp10 = icmp sgt i32 0, 0
  br i1 %cmp10, label %omp.inner.for.body11, label %omp.inner.for.cond9.omp.inner.for.end_crit_edge

omp.inner.for.cond9.omp.inner.for.end_crit_edge:  ; preds = %omp.inner.for.body11
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %omp.inner.for.body

omp.inner.for.cond.omp.inner.for.end20_crit_edge: ; No predecessors!
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  ret i32 0
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
