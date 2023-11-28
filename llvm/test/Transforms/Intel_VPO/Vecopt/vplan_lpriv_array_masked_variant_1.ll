; RUN: opt -S -passes=vplan-vec -vplan-force-vf=2 -vplan-print-after-create-masked-vplan -vplan-vec-scenario="n0;v2;m2" < %s | FileCheck %s -check-prefixes=CHECK,LLVMIR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-print-after-create-masked-vplan -vplan-enable-hir-private-arrays -vplan-force-vf=2 -vplan-vec-scenario="n0;v2;m2" -disable-output %s 2>&1 | FileCheck %s -check-prefixes=CHECK,HIR

; Incomming HIR
; <0>          BEGIN REGION { }
; <2>                %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LASTPRIVATE(&((%b3.i.lpriv)[0])) ]
; <21>
; <21>               + DO i1 = 0, %n + -1, 1   <DO_LOOP> <simd>
; <7>                |   (%b3.i.lpriv)[i1][3] = %a;
; <21>               + END LOOP
; <21>
; <17>               @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; <19>               %res = (%b3.i.lpriv)[0][1];
; <20>               ret %res;
; <0>          END REGION

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i16 @foo(i16 %a, i32 %n) {
; CHECK-LABEL:  VPlan after emitting masked variant:
; CHECK:    [[BB11:BB[0-9]+]]: # preds: new_latch
; CHECK-NEXT:     [DA: Uni] i32 [[VP8:%.*]] = induction-final{add} i32 0 i32 1
; CHECK-NEXT:     [DA: Uni] private-final-array-masked ptr [[VP1:%.*]] ptr [[B3_I_LPRIV0:%.*]] i1 [[VP5:%.*]]
; CHECK-NEXT:     [DA: Uni] br [[BB12:BB[0-9]+]]
;
; ****** CG check for LLVM IR ******
; LLVMIR:	VPlannedBB25:
; LLVMIR:  [[BITCAST:%.*]] = bitcast <2 x i1> {{%.*}} to i2
; LLVMIR:  %ctlz = call i2 @llvm.ctlz.i2(i2 [[BITCAST]], i1 true)
; LLVMIR:  [[SUB:%.*]] = sub i2 1, %ctlz
; LLVMIR:  %priv.extract = extractelement <2 x ptr> %.vec.base.addr, i2 [[SUB]]
; LLVMIR:  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %b3.i.lpriv, ptr align 2 %priv.extract, i64 24, i1 false)
; LLVMIR-NEXT:  br label %VPlannedBB24
;
; ****** Resulting HIR code check ******
; HIR:            %bsfintmask = bitcast.<2 x i1>.i2(%.vec14);
; HIR-NEXT:       %bsf = @llvm.ctlz.i2(%bsfintmask,  1);
; HIR-NEXT:       %ext.lane = 1  -  %bsf;
; HIR-NEXT:       @llvm.memcpy.p0.p0.i64(&((%b3.i.lpriv)[0]),  &(([12 x i16]*)(%priv.mem9)[0][%ext.lane]),  24,  0);

omp.inner.for.body.i.lr.ph:
  %b3.i.lpriv = alloca [12 x i16], align 1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:TYPED"(ptr %b3.i.lpriv, i16 0, i32 12) ]
  br label %omp.inner.for.body.i

omp.inner.for.body.i:
  %.omp.iv.i.local.03 = phi i32 [ %add5.i, %omp.body.continue.i ], [ 0, %DIR.OMP.SIMD.1 ]

  %arrayidx.i = getelementptr inbounds [12 x i16], ptr %b3.i.lpriv, i32 %.omp.iv.i.local.03, i32 3
  store i16 %a, ptr %arrayidx.i

  br label %omp.body.continue.i

omp.body.continue.i:
  %add5.i = add nuw nsw i32 %.omp.iv.i.local.03, 1
  %exitcond.not = icmp eq i32 %add5.i, %n
  br i1 %exitcond.not, label %omp.inner.for.cond.i.DIR.OMP.END.SIMD.5.i.loopexit_crit_edge, label %omp.inner.for.body.i

omp.inner.for.cond.i.DIR.OMP.END.SIMD.5.i.loopexit_crit_edge:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  %idx = getelementptr inbounds [12 x i16], ptr %b3.i.lpriv, i64 0, i64 1
  %res = load i16, ptr %idx

  ret i16 %res
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
