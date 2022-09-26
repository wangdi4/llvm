; RUN: opt -vplan-vec -vplan-force-vf=2 -vplan-entities-dump -vplan-dump-induction-init-details -vplan-print-after-vpentity-instrs -vplan-dump-plan-da -S < %s 2>&1 | FileCheck %s
;
; CHECK-LABEL:VPlan after insertion of VPEntities instructions:
; CHECK:      IntInduction(+) Start: i8 %c.linear.promoted Step: i8 [[VP_ARG_STEP:%.*]] StartVal: ? EndVal: ? BinOp: i8 [[VP_IND_NEXT:%.*]] = add i8 [[VP_IND_PHI:%.*]] i8 [[VP_STEP_INIT:%.*]]
; CHECK-NEXT:   Linked values: i8 [[VP_IND_PHI]], i8 [[VP_IND_NEXT]], i8 [[VP_IND_INIT:%.*]], i8 [[VP_STEP_INIT:%.*]], i8 [[VP_IND_FINAL:%.*]],
; CHECK:      BB1: # preds: BB4
; CHECK:        i8 [[VP_IND_INIT]] = induction-init{add, StartVal: ?, EndVal: ?} i8 %c.linear.promoted i8 [[VP_ARG_STEP]]
; CHECK-NEXT:   i8 [[VP_STEP_INIT]] = induction-init-step{add} i8 [[VP_ARG_STEP]]
; CHECK:      BB3: # preds: BB2
; CHECK:        i8 [[VP_IND_FINAL]] = induction-final{add} i8 %c.linear.promoted i8 [[VP_ARG_STEP]]
;
; CHECK:      Printing Divergence info for _Z3fooPiii:omp.inner.for.body.#1
; CHECK:      Basic Block: BB1
; CHECK:        Divergent: [Shape: Random] i8 [[VP_IND_INIT:%.*]] = induction-init{add, StartVal: ?, EndVal: ?} i8 live-in1 i8 [[VP_ARG_STEP:%.*]]
; CHECK-NEXT:   Uniform: [Shape: Uniform] i8 [[VP_STEP_INIT:%.*]] = induction-init-step{add} i8 [[VP_ARG_STEP]]
; CHECK:      Basic Block: BB2
; CHECK:        Divergent: [Shape: Random] i8 [[VP_IND_PHI:%.*]] = phi  [ i8 [[VP_IND_INIT:%.*]], BB1 ],  [ i8 [[VP_IND_NEXT:%.*]], BB2 ]
; CHECK:        Divergent: [Shape: Random] i8 [[VP_IND_NEXT]] = add i8 [[VP_IND_PHI:%.*]] i8 [[VP_STEP_INIT:%.*]]
; CHECK:      Basic Block: BB3
; CHECK:        Uniform: [Shape: Uniform] i8 [[VP_IND_FINAL:%.*]] = induction-final{add} i8 %c.linear.promoted i8 [[VP_ARG_STEP:%.*]]
;
; CHECK:      define dso_local noundef i32 @_Z3fooPiii(i32* noundef %ptr, i32 noundef %step, i32 noundef %n) local_unnamed_addr {
; CHECK:      VPlannedBB1:                                      ; preds = %VPlannedBB
; CHECK-NEXT:   [[VP_BCAST_SPLATINSERT:%.*]] = insertelement <2 x i8> poison, i8 [[VP_STEP:%.*]], i32 0
; CHECK-NEXT:   [[VP_BCAST_SPLAT:%.*]] = shufflevector <2 x i8> [[VP_BCAST_SPLATINSERT]], <2 x i8> poison, <2 x i32> zeroinitializer
; CHECK-NEXT:   br label %VPlannedBB2
; CHECK:      VPlannedBB2:                                      ; preds = %VPlannedBB1
; CHECK:        [[VP_C_LINEAR_IND_START_BCAST_SPLATINSERT:%.*]] = insertelement <2 x i8> poison, i8 [[VP_C_LINEAR_PROMOTED:%.*]], i32 0
; CHECK-NEXT:   [[VP_C_LINEAR_IND_START_BCAST_SPLAT:%.*]] = shufflevector <2 x i8> [[VP_C_LINEAR_IND_START_BCAST_SPLATINSERT:%.*]], <2 x i8> poison, <2 x i32> zeroinitializer
; CHECK-NEXT:   [[VP_BCAST_SPLAT_MUL:%.*]] = mul <2 x i8> [[VP_BCAST_SPLAT]], <i8 0, i8 1>
; CHECK-NEXT:   [[VP_C_LINEAR_NEXT:%.*]] = add <2 x i8> [[VP_C_LINEAR_IND_START_BCAST_SPLAT]], [[VP_BCAST_SPLAT_MUL]]
; CHECK-NEXT:   [[VP_STEP_NEXT:%.*]] = mul i8 [[VP_STEP:%.*]], 2
; CHECK-NEXT:   [[VP_STEP_INIT_SPLATINSERT:%.*]] = insertelement <2 x i8> poison, i8 [[VP_STEP_NEXT]], i32 0
; CHECK-NEXT:   [[VP_STEP_INIT_SPLAT:%.*]] = shufflevector <2 x i8> [[VP_STEP_INIT_SPLATINSERT]], <2 x i8> poison, <2 x i32> zeroinitializer
; CHECK:      vector.body:                                      ; preds = %vector.body, %VPlannedBB2
; CHECK:        [[VP_VEC_PHI:%.*]] = phi <2 x i8> [ [[VP_C_LINEAR_NEXT]], %VPlannedBB2 ], [ [[VP_C_LINEAR:%.*]], %vector.body ]
; CHECK:        [[VP_C_LINEAR]] = add <2 x i8> [[VP_VEC_PHI]], [[VP_STEP_INIT_SPLAT]]
;
; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef i32 @_Z3fooPiii(i32* noundef %ptr, i32 noundef %step, i32 noundef %n) local_unnamed_addr #0 {
entry:
  %ptr.addr.linear = alloca i32*, align 8
  %c.linear = alloca i8, align 1
  %i.linear.iv = alloca i32, align 4
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  store i32* %ptr, i32** %ptr.addr.linear, align 8
  store i8 0, i8* %c.linear, align 1
  br label %DIR.OMP.SIMD.131

DIR.OMP.SIMD.131:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:PTR_TO_PTR.TYPED"(i32** %ptr.addr.linear, i32 0, i32 1, i32 1), "QUAL.OMP.LINEAR:TYPED"(i8* %c.linear, i32 0, i32 1, i32 %step), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.131
  %1 = bitcast i32* %i.linear.iv to i8*
  %2 = trunc i32 %step to i8
  %c.linear.promoted = load i8, i8* %c.linear, align 1
  %ptr.addr.linear.promoted = load i32*, i32** %ptr.addr.linear, align 8
  %3 = add nsw i32 %n, -1
  %4 = zext i32 %3 to i64
  %5 = add nuw nsw i64 %4, 1
  %6 = trunc i32 %n to i8
  %7 = mul i8 %6, %2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %8 = phi i32* [ %ptr.addr.linear.promoted, %DIR.OMP.SIMD.2 ], [ %incdec.ptr, %omp.inner.for.body ]
  %9 = phi i8 [ %c.linear.promoted, %DIR.OMP.SIMD.2 ], [ %conv7, %omp.inner.for.body ]
  %.omp.iv.local.024 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add8, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  %conv = sext i8 %9 to i32
  store i32 %conv, i32* %8, align 4
  %incdec.ptr = getelementptr inbounds i32, i32* %8, i64 1
  %conv7 = add i8 %9, %2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  %add8 = add nuw nsw i32 %.omp.iv.local.024, 1
  %exitcond.not = icmp eq i32 %add8, %n
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.230, label %omp.inner.for.body

DIR.OMP.END.SIMD.230:                             ; preds = %omp.inner.for.body
  %scevgep = getelementptr i32, i32* %ptr.addr.linear.promoted, i64 %5
  %10 = add i8 %c.linear.promoted, %7
  store i8 %10, i8* %c.linear, align 1
  store i32* %scevgep, i32** %ptr.addr.linear, align 8
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.230
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %phi.cast = sext i8 %10 to i32
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.4, %entry
  %c.1 = phi i32 [ 0, %entry ], [ %phi.cast, %DIR.OMP.END.SIMD.4 ]
  ret i32 %c.1
}
declare token @llvm.directive.region.entry() #1
declare void @llvm.directive.region.exit(token) #1
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture)
