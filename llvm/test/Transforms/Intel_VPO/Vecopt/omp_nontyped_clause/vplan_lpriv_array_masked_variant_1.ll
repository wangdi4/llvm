; RUN: opt -S -vplan-vec -vplan-force-vf=2 -vplan-print-after-create-masked-vplan -vplan-vec-scenario="n0;v2;m2" < %s | FileCheck %s -check-prefixes=CHECK,LLVMIR
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-print-after-create-masked-vplan -vplan-force-vf=2 -vplan-vec-scenario="n0;v2;m2" -disable-output %s 2>&1 | FileCheck %s -check-prefixes=CHECK,HIR

; Incoming HIR
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
; CHECK-NEXT:     [DA: Uni] private-final-array-masked [12 x i16]* [[VP1:%.*]] [12 x i16]* [[B3_I_LPRIV0:%.*]] i1 [[VP5:%.*]]
; CHECK-NEXT:     [DA: Uni] br [[BB12:BB[0-9]+]]
;
; ****** CG check for LLVM IR ******
; LLVMIR:	VPlannedBB23:
; LLVMIR-NEXT:	  %24 = bitcast <2 x i1> %14 to i2
; LLVMIR-NEXT:	  %ctlz = call i2 @llvm.ctlz.i2(i2 %24, i1 true)
; LLVMIR-NEXT:	  %25 = sub i2 1, %ctlz
; LLVMIR-NEXT:	  %priv.extract = extractelement <2 x [12 x i16]*> %.vec.base.addr, i2 %25
; LLVMIR-NEXT:	  %26 = bitcast [12 x i16]* %b3.i.lpriv to i8*
; LLVMIR-NEXT:	  %27 = bitcast [12 x i16]* %priv.extract to i8*
; LLVMIR-NEXT:	  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %26, i8* align 2 %27, i64 24, i1 false)
; LLVMIR-NEXT:	  br label %VPlannedBB22
;
; ****** Resulting HIR code check ******
; HIR:            %bsfintmask = bitcast.<2 x i1>.i2(%.vec12);
; HIR-NEXT:       %bsf = @llvm.ctlz.i2(%bsfintmask,  1);
; HIR-NEXT:       %ext.lane = 1  -  %bsf;
; HIR-NEXT:       @llvm.memcpy.p0a12i16.p0a12i16.i64(&((%b3.i.lpriv)[0]),  &(([12 x i16]*)(%priv.mem9)[0][%ext.lane]),  24,  0);

omp.inner.for.body.i.lr.ph:
  %b3.i.lpriv = alloca [12 x i16], align 1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE"([12 x i16]* %b3.i.lpriv) ]
  br label %omp.inner.for.body.i

omp.inner.for.body.i:
  %.omp.iv.i.local.03 = phi i32 [ %add5.i, %omp.body.continue.i ], [ 0, %DIR.OMP.SIMD.1 ]

  %arrayidx.i = getelementptr inbounds [12 x i16], [12 x i16]* %b3.i.lpriv, i32 %.omp.iv.i.local.03, i32 3
  store i16 %a, i16* %arrayidx.i

  br label %omp.body.continue.i

omp.body.continue.i:
  %add5.i = add nuw nsw i32 %.omp.iv.i.local.03, 1
  %exitcond.not = icmp eq i32 %add5.i, %n
  br i1 %exitcond.not, label %omp.inner.for.cond.i.DIR.OMP.END.SIMD.5.i.loopexit_crit_edge, label %omp.inner.for.body.i

omp.inner.for.cond.i.DIR.OMP.END.SIMD.5.i.loopexit_crit_edge:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]

  %idx = getelementptr inbounds [12 x i16], [12 x i16]* %b3.i.lpriv, i64 0, i64 1
  %res = load i16, i16* %idx

  ret i16 %res
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
