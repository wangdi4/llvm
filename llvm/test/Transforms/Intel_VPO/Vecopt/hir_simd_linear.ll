; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec,print<hir>' -disable-output -vplan-print-after-plain-cfg -vplan-entities-dump < %s 2>&1 | FileCheck %s

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

define void @_Z3fooPlS_(i64* nocapture noundef writeonly %lp1, i64* nocapture noundef readnone %lp2) {
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
; CHECK:       IntInduction(+) Start: i64 [[VP_LOAD:%.*]] Step: i64 2 StartVal: ? EndVal: ? need close form  Memory: i64* [[L2_LINEAR0:%.*]]

; CHECK:       BEGIN REGION { modified }
; CHECK-NEXT:        [[PRIV_MEM_BC0:%.*]] = &((i64*)([[PRIV_MEM0:%.*]])[0])
; CHECK-NEXT:        [[DOTUNIFLOAD0:%.*]] = ([[L2_LINEAR0]])[0]
; CHECK-NEXT:        [[IND_VEC_STEP0:%.*]] = 2  *  <i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:        [[TMP0:%.*]] = [[DOTUNIFLOAD0]]  +  [[IND_VEC_STEP0]]
; CHECK-NEXT:        (<4 x i64>*)([[PRIV_MEM0]])[0] = [[TMP0]]
; CHECK-NEXT:        [[IND_STEP_INIT0:%.*]] = 2  *  4
; CHECK-NEXT:        [[PHI_TEMP0:%.*]] = [[TMP0]]

; CHECK:             + DO i1 = 0, 99, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:        |   (<4 x i64>*)([[PRIV_MEM0]])[0] = [[PHI_TEMP0]]
; CHECK-NEXT:        |   [[DOTVEC0:%.*]] = (<4 x i64>*)([[PRIV_MEM0]])[0]
; CHECK-NEXT:        |   (<4 x i64>*)([[LP10:%.*]])[i1] = [[DOTVEC0]]
; CHECK-NEXT:        |   (<4 x i64>*)([[PRIV_MEM0]])[0] = [[DOTVEC0]] + 2
; CHECK-NEXT:        |   [[SERIAL_TEMP0:%.*]] = undef
; CHECK:             |   [[PHI_TEMP0]] = [[IND_STEP_INIT0]] + [[PHI_TEMP0]]
; CHECK-NEXT:        + END LOOP

; CHECK:             [[DOTVEC100:%.*]] = (<4 x i64>*)([[PRIV_MEM0]])[0]
; CHECK-NEXT:        [[TMP1:%.*]] = 2  *  100
; CHECK-NEXT:        [[FINI_LVAL0:%.*]] = [[DOTUNIFLOAD0]]  +  [[TMP1]]
; CHECK-NEXT:        ([[L2_LINEAR0]])[0] = [[FINI_LVAL0]]
; CHECK-NEXT:  END REGION
;

DIR.OMP.SIMD.1:
  %l2.linear = alloca i64, align 8
  %l1.linear.iv = alloca i64, align 8
  store i64 100, i64* %l2.linear, align 8
  br label %DIR.OMP.SIMD.118

DIR.OMP.SIMD.118:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4), "QUAL.OMP.LINEAR:TYPED"(i64* %l2.linear, i64 0, i32 1, i32 2), "QUAL.OMP.LINEAR:IV.TYPED"(i64* %l1.linear.iv, i64 0, i32 1, i32 1), "QUAL.OMP.NORMALIZED.IV:TYPED"(i8* null, i64 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(i8* null, i64 0) ]
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.118
  %.omp.iv.local.011 = phi i64 [ 0, %DIR.OMP.SIMD.118 ], [ %add2, %omp.inner.for.body ]
  %1 = load i64, i64* %l2.linear, align 8
  %arrayidx = getelementptr inbounds i64, i64* %lp1, i64 %.omp.iv.local.011
  store i64 %1, i64* %arrayidx, align 8
  %add1 = add nsw i64 %1, 2
  store i64 %add1, i64* %l2.linear, align 8
  %call = call noundef i64 @_Z3bazPl(i64* noundef nonnull %l2.linear)
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

declare dso_local noundef i64 @_Z3bazPl(i64* noundef) #0

attributes #0 = { nounwind }
