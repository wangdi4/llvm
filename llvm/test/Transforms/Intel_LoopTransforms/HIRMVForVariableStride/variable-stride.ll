; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-mv-variable-stride -print-before=hir-mv-variable-stride -print-after=hir-mv-variable-stride -hir-details-refs -hir-print-only=0 < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-mv-variable-stride,print<hir>" -hir-details-refs -aa-pipeline="basic-aa" -hir-print-only=0 < %s 2>&1 | FileCheck %s

; Check if loops with memrefs having variable strides are multiversioned.
; Also, it checks outermost loop possible is MVed.

; CHECK: Function: step2d_tile_

; CHECK:        BEGIN REGION { }
; CHECK:              + DO i1 = 0, sext.i32.i64(%"step2d_tile_$M2_fetch") + -1 * sext.i32.i64(%"step2d_tile_$M1_fetch"), 1   <DO_LOOP>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%"step2d_tile_$M2_fetch") + -1 * sext.i32.i64(%"step2d_tile_$M1_fetch"), 1   <DO_LOOP>
; CHECK:              |   |   %add114 = (%"step2d_tile_$ZETA_$field0$_fetch")[1:%"step2d_tile_$M1_fetch":%func_result38_fetch(double*:0)][%"step2d_tile_$L2_fetch":i1 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result30_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result22_fetch(double*:0)]  +  (%"step2d_tile_$H_$field0$_fetch")[%"step2d_tile_$L2_fetch":i1 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result82_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result72_fetch(double*:0)];
; CHECK:              |   |   (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":i1 + sext.i32.i64(%"step2d_tile_$M1_fetch"):8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64(%"step2d_tile_$M1_fetch"):8(double*:0)] = %add114;
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:        END REGION

; CHECK: Function: step2d_tile_

; CHECK:        BEGIN REGION { }
; CHECK:              if (%func_result22_fetch == 8 && %func_result72_fetch == 8)
; CHECK:              {
; CHECK:                 + DO i1 = 0, sext.i32.i64(%"step2d_tile_$M2_fetch") + -1 * sext.i32.i64(%"step2d_tile_$M1_fetch"), 1   <DO_LOOP>
; CHECK:                 |   + DO i2 = 0, sext.i32.i64(%"step2d_tile_$M2_fetch") + -1 * sext.i32.i64(%"step2d_tile_$M1_fetch"), 1   <DO_LOOP>
; CHECK:                 |   |   %add114 = (%"step2d_tile_$ZETA_$field0$_fetch")[1:%"step2d_tile_$M1_fetch":%func_result38_fetch(double*:0)][%"step2d_tile_$L2_fetch":i1 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result30_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64(%"step2d_tile_$M1_fetch"):8(double*:0)]  +  (%"step2d_tile_$H_$field0$_fetch")[%"step2d_tile_$L2_fetch":i1 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result82_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64(%"step2d_tile_$M1_fetch"):8(double*:0)];
; CHECK:                 |   |   (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":i1 + sext.i32.i64(%"step2d_tile_$M1_fetch"):8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64(%"step2d_tile_$M1_fetch"):8(double*:0)] = %add114;
; CHECK:                 |   + END LOOP
; CHECK:                 + END LOOP
; CHECK:              }
; CHECK:              else
; CHECK:              {
; CHECK:                 + DO i1 = 0, sext.i32.i64(%"step2d_tile_$M2_fetch") + -1 * sext.i32.i64(%"step2d_tile_$M1_fetch"), 1   <DO_LOOP>
; CHECK:                 |   + DO i2 = 0, sext.i32.i64(%"step2d_tile_$M2_fetch") + -1 * sext.i32.i64(%"step2d_tile_$M1_fetch"), 1   <DO_LOOP>
; CHECK:                 |   |   %add114 = (%"step2d_tile_$ZETA_$field0$_fetch")[1:%"step2d_tile_$M1_fetch":%func_result38_fetch(double*:0)][%"step2d_tile_$L2_fetch":i1 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result30_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result22_fetch(double*:0)]  +  (%"step2d_tile_$H_$field0$_fetch")[%"step2d_tile_$L2_fetch":i1 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result82_fetch(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64(%"step2d_tile_$M1_fetch"):%func_result72_fetch(double*:0)];
; CHECK:                 |   |   (%"step2d_tile_$DRHS")[%"step2d_tile_$L2_fetch":i1 + sext.i32.i64(%"step2d_tile_$M1_fetch"):8 * sext.i32.i64((1 + (-1 * %"step2d_tile_$L1_fetch") + %"step2d_tile_$U1_fetch"))(double*:0)][%"step2d_tile_$L1_fetch":i2 + sext.i32.i64(%"step2d_tile_$M1_fetch"):8(double*:0)] = %add114;
; CHECK:                 |   + END LOOP
; CHECK:                 + END LOOP
; CHECK:              }
; CHECK:        END REGION


;Module Before HIR
; ModuleID = 'step2d.f90'
source_filename = "step2d.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @step2d_tile_({ double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* noalias dereferenceable(120) nocapture readonly %"step2d_tile_$ZETA", { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* noalias dereferenceable(96) nocapture readonly %"step2d_tile_$H", i32* noalias nocapture readonly %"step2d_tile_$L1", i32* noalias nocapture readonly %"step2d_tile_$L2", i32* noalias nocapture readonly %"step2d_tile_$U1", i32* noalias nocapture readonly %"step2d_tile_$U2", i32* noalias nocapture readonly %"step2d_tile_$M1", i32* noalias nocapture readonly %"step2d_tile_$M2") local_unnamed_addr #0 {
alloca:
  %"var$3" = alloca [8 x i64], align 16
  %addressof = alloca [4 x i8], align 1
  %ARGBLOCK_0 = alloca { double }, align 8
  %"step2d_tile_$ZETA_$field0$" = getelementptr inbounds { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"step2d_tile_$ZETA", i64 0, i32 0
  %"step2d_tile_$ZETA_$field6$_$field1$" = getelementptr inbounds { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { double*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"step2d_tile_$ZETA", i64 0, i32 6, i64 0, i32 1
  %"step2d_tile_$H_$field6$_$field1$" = getelementptr inbounds { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"step2d_tile_$H", i64 0, i32 6, i64 0, i32 1
  %"step2d_tile_$L1_fetch" = load i32, i32* %"step2d_tile_$L1", align 4
  %"step2d_tile_$L2_fetch" = load i32, i32* %"step2d_tile_$L2", align 4
  %"step2d_tile_$U1_fetch" = load i32, i32* %"step2d_tile_$U1", align 4
  %"step2d_tile_$U2_fetch" = load i32, i32* %"step2d_tile_$U2", align 4
  %sub = sub i32 1, %"step2d_tile_$L1_fetch"
  %add = add i32 %sub, %"step2d_tile_$U1_fetch"
  %int_sext = sext i32 %add to i64
  %rel = icmp sgt i64 %int_sext, 0
  %slct = select i1 %rel, i64 %int_sext, i64 0
  %mul = shl nsw i64 %slct, 3
  %sub2 = sub i32 1, %"step2d_tile_$L2_fetch"
  %add4 = add i32 %sub2, %"step2d_tile_$U2_fetch"
  %int_sext6 = sext i32 %add4 to i64
  %rel8 = icmp sgt i64 %int_sext6, 0
  %slct10 = select i1 %rel8, i64 %int_sext6, i64 0
  %mul12 = mul nsw i64 %mul, %slct10
  %div = lshr exact i64 %mul12, 3
  %"step2d_tile_$DRHS" = alloca double, i64 %div, align 8
  %mul120 = shl nsw i64 %int_sext, 3
  %"step2d_tile_$M1_fetch" = load i32, i32* %"step2d_tile_$M1", align 4
  %"step2d_tile_$M2_fetch" = load i32, i32* %"step2d_tile_$M2", align 4
  %rel14 = icmp slt i32 %"step2d_tile_$M2_fetch", %"step2d_tile_$M1_fetch"
  br i1 %rel14, label %alloca.bb53_crit_edge, label %bb8.preheader

alloca.bb53_crit_edge:                            ; preds = %alloca
  %.pre = sext i32 %"step2d_tile_$L1_fetch" to i64
  %.pre328 = sext i32 %"step2d_tile_$L2_fetch" to i64
  %.pre329 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$ZETA_$field6$_$field1$", i32 0)
  %.pre330 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$ZETA_$field6$_$field1$", i32 1)
  %.pre331 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$ZETA_$field6$_$field1$", i32 2)
  br label %bb53

bb8.preheader:                                    ; preds = %alloca
  %"step2d_tile_$H_$field0$" = getelementptr inbounds { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }, { double*, i64, i64, i64, i64, i64, [2 x { i64, i64, i64 }] }* %"step2d_tile_$H", i64 0, i32 0
  %func_result22 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$ZETA_$field6$_$field1$", i32 0)
  %int_sext26 = sext i32 %"step2d_tile_$L1_fetch" to i64
  %func_result30 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$ZETA_$field6$_$field1$", i32 1)
  %int_sext34 = sext i32 %"step2d_tile_$L2_fetch" to i64
  %func_result38 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$ZETA_$field6$_$field1$", i32 2)
  %int_sext42 = sext i32 %"step2d_tile_$M1_fetch" to i64
  %func_result72 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$H_$field6$_$field1$", i32 0)
  %func_result82 = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull %"step2d_tile_$H_$field6$_$field1$", i32 1)
  %0 = sext i32 %"step2d_tile_$M2_fetch" to i64
  %"step2d_tile_$ZETA_$field0$_fetch" = load double*, double** %"step2d_tile_$ZETA_$field0$", align 8
  %func_result22_fetch = load i64, i64* %func_result22, align 8
  %func_result30_fetch = load i64, i64* %func_result30, align 8
  %func_result38_fetch = load i64, i64* %func_result38, align 8
  %func_result66 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %func_result38_fetch, double* %"step2d_tile_$ZETA_$field0$_fetch", i64 %int_sext42)
  %"step2d_tile_$H_$field0$_fetch" = load double*, double** %"step2d_tile_$H_$field0$", align 8
  %func_result72_fetch = load i64, i64* %func_result72, align 8
  %func_result82_fetch = load i64, i64* %func_result82, align 8
  br label %bb12.preheader

bb12.preheader:                                   ; preds = %bb13, %bb8.preheader
  %indvars.iv326 = phi i64 [ %int_sext42, %bb8.preheader ], [ %indvars.iv.next327, %bb13 ]
  %func_result68 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext34, i64 %func_result30_fetch, double* %func_result66, i64 %indvars.iv326)
  %func_result110 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext34, i64 %func_result82_fetch, double* %"step2d_tile_$H_$field0$_fetch", i64 %indvars.iv326)
  %func_result148 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext34, i64 %mul120, double* nonnull %"step2d_tile_$DRHS", i64 %indvars.iv326)
  br label %bb12

bb12:                                             ; preds = %bb12.preheader, %bb12
  %indvars.iv = phi i64 [ %int_sext42, %bb12.preheader ], [ %indvars.iv.next, %bb12 ]
  %func_result70 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext26, i64 %func_result22_fetch, double* %func_result68, i64 %indvars.iv)
  %func_result70_fetch = load double, double* %func_result70, align 8
  %func_result112 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext26, i64 %func_result72_fetch, double* %func_result110, i64 %indvars.iv)
  %func_result112_fetch = load double, double* %func_result112, align 8
  %add114 = fadd double %func_result70_fetch, %func_result112_fetch
  %func_result150 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext26, i64 8, double* nonnull %func_result148, i64 %indvars.iv)
  store double %add114, double* %func_result150, align 8
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %rel160 = icmp slt i64 %indvars.iv, %0
  br i1 %rel160, label %bb12, label %bb13

bb13:                                             ; preds = %bb12
  %indvars.iv.next327 = add nsw i64 %indvars.iv326, 1
  %rel170 = icmp slt i64 %indvars.iv326, %0
  br i1 %rel170, label %bb12.preheader, label %bb53.loopexit

bb53.loopexit:                                    ; preds = %bb13
  br label %bb53

bb53:                                             ; preds = %bb53.loopexit, %alloca.bb53_crit_edge
  %func_result202.pre-phi = phi i64* [ %.pre331, %alloca.bb53_crit_edge ], [ %func_result38, %bb53.loopexit ]
  %func_result190.pre-phi = phi i64* [ %.pre330, %alloca.bb53_crit_edge ], [ %func_result30, %bb53.loopexit ]
  %func_result178.pre-phi = phi i64* [ %.pre329, %alloca.bb53_crit_edge ], [ %func_result22, %bb53.loopexit ]
  %int_sext235.pre-phi = phi i64 [ %.pre328, %alloca.bb53_crit_edge ], [ %int_sext34, %bb53.loopexit ]
  %int_sext232.pre-phi = phi i64 [ %.pre, %alloca.bb53_crit_edge ], [ %int_sext26, %bb53.loopexit ]
  %func_result172 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext235.pre-phi, i64 %mul120, double* nonnull %"step2d_tile_$DRHS", i64 8)
  %func_result174 = call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext232.pre-phi, i64 8, double* nonnull %func_result172, i64 8)
  %1 = bitcast double* %func_result174 to i64*
  %func_result174_fetch324 = load i64, i64* %1, align 8
  %"step2d_tile_$ZETA_$field0$_fetch176" = load double*, double** %"step2d_tile_$ZETA_$field0$", align 8
  %func_result178_fetch = load i64, i64* %func_result178.pre-phi, align 8
  %sub186 = add nsw i32 %"step2d_tile_$M1_fetch", -1
  %int_sext188 = sext i32 %sub186 to i64
  %func_result190_fetch = load i64, i64* %func_result190.pre-phi, align 8
  %sub198 = add nsw i32 %"step2d_tile_$M2_fetch", -2
  %int_sext200 = sext i32 %sub198 to i64
  %func_result202_fetch = load i64, i64* %func_result202.pre-phi, align 8
  %func_result226 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %func_result202_fetch, double* %"step2d_tile_$ZETA_$field0$_fetch176", i64 8)
  %func_result228 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext235.pre-phi, i64 %func_result190_fetch, double* %func_result226, i64 %int_sext200)
  %func_result230 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext232.pre-phi, i64 %func_result178_fetch, double* %func_result228, i64 %int_sext188)
  %2 = bitcast double* %func_result230 to i64*
  store i64 %func_result174_fetch324, i64* %2, align 8
  %func_result276 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 %int_sext235.pre-phi, i64 %func_result190_fetch, double* %func_result226, i64 9)
  %func_result278 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 %int_sext232.pre-phi, i64 %func_result178_fetch, double* %func_result276, i64 1)
  %3 = bitcast double* %func_result278 to i64*
  %func_result278_fetch325 = load i64, i64* %3, align 8
  %.fca.0.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 0
  store i8 48, i8* %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 1
  store i8 1, i8* %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 2
  store i8 1, i8* %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], [4 x i8]* %addressof, i64 0, i64 3
  store i8 0, i8* %.fca.3.gep, align 1
  %4 = bitcast { double }* %ARGBLOCK_0 to i64*
  store i64 %func_result278_fetch325, i64* %4, align 8
  %bit_cast = bitcast [8 x i64]* %"var$3" to i8*
  %bit_cast282 = bitcast { double }* %ARGBLOCK_0 to i8*
  %func_result284 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %bit_cast, i32 -1, i64 1239157112576, i8* nonnull %.fca.0.gep, i8* nonnull %bit_cast282)
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32) #1

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr

attributes #0 = { "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable }

!omp_offload.info = !{}
