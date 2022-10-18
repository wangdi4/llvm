; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-vplan-vec -disable-output -vplan-print-after-plain-cfg -vplan-entities-dump -print-after=hir-vplan-vec < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; *** IR Dump Before vpo::VPlanDriverHIRPass ***
; <0>          BEGIN REGION { }
; <2>                %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.SIMDLEN(4),  QUAL.OMP.LINEAR:TYPED(&((%l2.linear)[0])012),  QUAL.OMP.LINEAR:IV.TYPED(&((%l1.linear.iv)[0])011),  QUAL.OMP.NORMALIZED.IV:TYPED(null0),  QUAL.OMP.NORMALIZED.UB:TYPED(null0) ]
; <21>
; <21>               + DO i1 = 0, 99, 1   <DO_LOOP> <simd>
; <6>                |   %1 = (%l2.linear)[0];
; <8>                |   (%lp1)[i1] = %1;
; <10>               |   (%l2.linear)[0] = %1 + 2;
; <11>               |   %call = @_Z3bazPl(&((%l2.linear)[0]));
; <21>               + END LOOP
; <21>
; <19>               @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; <0>          END REGION

define void @_Z3fooPlS_(i32* nocapture noundef writeonly %lp1) {
; CHECK-LABEL:  VPlan after importing plain CFG:
; CHECK-NEXT:  VPlan IR for: _Z3fooPlS_:HIR.#{{[0-9]+}}
; CHECK-NEXT:  External Defs Start:
; CHECK-DAG:     [[VP0:%.*]] = {%l2.linear}
; CHECK-DAG:     [[VP1:%.*]] = {%lp1}
; CHECK-DAG:     [[VP2:%.*]] = {%l1.linear.iv}
; CHECK-NEXT:  External Defs End:
; CHECK-NEXT:  Loop Entities of the loop with header [[BB0:BB[0-9]+]]
; CHECK:       Induction list
; CHECK-NEXT:   IntInduction(+) Start: i64 0 Step: i64 1 StartVal: i64 0 EndVal: i64 99 BinOp: i64 [[VP3:%.*]] = add i64 [[VP4:%.*]] i64 1
; CHECK-NEXT:    Linked values: i64 [[VP4]], i64 [[VP3]],
; CHECK:       IntInduction(+) Start: i32 [[VP_LOAD:%.*]] Step: i32 2 StartVal: ? EndVal: ? need close form  Memory: i32* [[L2_LINEAR0:%.*]]

; CHECK:       *** IR Dump After vpo::VPlanDriverHIRPass ***
; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        %priv.mem.bc = &((i32*)(%priv.mem)[0]);
; CHECK-NEXT:        %.unifload = (%l2.linear)[0];
; CHECK-NEXT:        %ind.vec.step = 2  *  <i32 0, i32 1, i32 2, i32 3>;
; CHECK-NEXT:        %0 = %.unifload  +  %ind.vec.step;
; CHECK-NEXT:        (<4 x i32>*)(%priv.mem)[0] = %0;
; CHECK-NEXT:        %ind.step.init = 2  *  4;
; CHECK-NEXT:        %phi.temp = %0;

; CHECK:             + DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:        |   (<4 x i32>*)(%priv.mem)[0] = %phi.temp;
; CHECK-NEXT:        |   %.vec = (<4 x i32>*)(%priv.mem)[0];
; CHECK-NEXT:        |   (<4 x i32>*)(%lp1)[i1] = %.vec;
; CHECK-NEXT:        |   (<4 x i32>*)(%priv.mem)[0] = %.vec + 2;
; CHECK-NEXT:        |   %serial.temp = undef;
; CHECK:             |   %phi.temp = %ind.step.init + %phi.temp;
; CHECK-NEXT:        + END LOOP

; CHECK:             %.vec10 = (<4 x i32>*)(%priv.mem)[0];
; CHECK-NEXT:        %cast.crd = trunc.i64.i32(100);
; CHECK-NEXT:        %1 = 2  *  %cast.crd;
; CHECK-NEXT:        %.unifload = %.unifload  +  %1;
; CHECK-NEXT:        (%l2.linear)[0] = %.unifload;
; CHECK-NEXT:  END REGION
;
DIR.OMP.SIMD.1:
  %l2.linear = alloca i32, align 8
  %l1.linear.iv = alloca i64, align 8
  store i32 100, i32* %l2.linear, align 8
  br label %DIR.OMP.SIMD.118

DIR.OMP.SIMD.118:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:TYPED"(i32* %l2.linear, i32 0, i32 1, i32 2), "QUAL.OMP.LINEAR:IV.TYPED"(i64* %l1.linear.iv, i64 0, i32 1, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i8* null, i64 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(i8* null, i64 0) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.118
  %.omp.iv.local.011 = phi i64 [ 0, %DIR.OMP.SIMD.118 ], [ %add2, %omp.inner.for.body ]
  %1 = load i32, i32* %l2.linear, align 8
  %arrayidx = getelementptr inbounds i32, i32* %lp1, i64 %.omp.iv.local.011
  store i32 %1, i32* %arrayidx, align 8
  %add1 = add nsw i32 %1, 2
  store i32 %add1, i32* %l2.linear, align 8
  %call = call noundef i64 @_Z3bazPl(i32* noundef nonnull %l2.linear)
  %add2 = add nuw nsw i64 %.omp.iv.local.011, 1
  %exitcond.not = icmp eq i64 %add2, 100
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.117, label %omp.inner.for.body

DIR.OMP.END.SIMD.117:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.117
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local noundef i64 @_Z3bazPl(i32* noundef) #0

attributes #0 = { nounwind }
