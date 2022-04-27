; This test case checks the situation when the innermost loop' loop level is larger than 3
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-create-function-level-region -hir-store-result-into-temp-array -print-after=hir-store-result-into-temp-array < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-store-result-into-temp-array,print<hir>" -hir-create-function-level-region 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Store Result Into Temp Array ***
;Function: jacobian_
;
;<0>          BEGIN REGION { }
;<2>                if (%umax(%rel9, %rel5, %rel) == 0)
;<2>                {
;<6>                   %"jacobian_$N_fetch" = (%"jacobian_$N")[0];
;<8>                   %"jacobian_$M.13" = undef;
;<202>
;<23>                     %"jacobian_$M.0" = undef;
;<202>                 + DO i1 = 0, %"jacobian_$N_fetch" + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
;<203>                 |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + -1, 1   <DO_LOOP>
;<204>                 |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1   <DO_LOOP>
;<38>                  |   |   |   %mod = i3 + 2  %  %"jacobian_$NY_fetch";
;<205>                 |   |   |
;<205>                 |   |   |   + DO i4 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1   <DO_LOOP>
;<47>                  |   |   |   |   %mod33 = i4 + 3  %  %"jacobian_$NX_fetch";
;<50>                  |   |   |   |   %"jacobian_$Q[][][][]_fetch" = (%"jacobian_$Q")[i2 + 1][i3 + 1][i4 + 1][0];
;<53>                  |   |   |   |   %div = (%"jacobian_$Q")[i2 + 1][i3 + 1][i4 + 1][1]  /  %"jacobian_$Q[][][][]_fetch";
;<56>                  |   |   |   |   %div67 = (%"jacobian_$Q")[i2 + 1][i3 + 1][i4 + 1][2]  /  %"jacobian_$Q[][][][]_fetch";
;<57>                  |   |   |   |   %mul79 = %div  *  %div;
;<58>                  |   |   |   |   %mul81 = %div67  *  %div67;
;<59>                  |   |   |   |   %add82 = %mul79  +  %mul81;
;<60>                  |   |   |   |   %mul83 = %add82  *  2.000000e+00;
;<61>                  |   |   |   |   %div85 = %mul83  /  %"jacobian_$Q[][][][]_fetch";
;<62>                  |   |   |   |   %func_result = @llvm.pow.f64(%div85,  2.500000e+00);
;<66>                  |   |   |   |   %"jacobian_$Q[]88[][][]_fetch" = (%"jacobian_$Q")[i2 + 2][%mod][%mod33][0];
;<69>                  |   |   |   |   %div113 = (%"jacobian_$Q")[i2 + 2][%mod][%mod33][1]  /  %"jacobian_$Q[]88[][][]_fetch";
;<72>                  |   |   |   |   %div136 = (%"jacobian_$Q")[i2 + 2][%mod][%mod33][2]  /  %"jacobian_$Q[]88[][][]_fetch";
;<73>                  |   |   |   |   %mul150 = %div113  *  %div113;
;<74>                  |   |   |   |   %mul152 = %div136  *  %div136;
;<75>                  |   |   |   |   %add153 = %mul150  +  %mul152;
;<76>                  |   |   |   |   %mul154 = %add153  *  2.000000e+00;
;<77>                  |   |   |   |   %div156 = %mul154  /  %"jacobian_$Q[]88[][][]_fetch";
;<78>                  |   |   |   |   %func_result148 = @llvm.pow.f64(%div156,  2.500000e+00);
;<79>                  |   |   |   |   %add157 = %"jacobian_$M.0"  +  %func_result;
;<80>                  |   |   |   |   %"jacobian_$M.0" = %add157  +  %func_result148;
;<205>                 |   |   |   + END LOOP
;<204>                 |   |   + END LOOP
;<203>                 |   + END LOOP
;<203>                 |
;<206>                 |
;<206>                 |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + 1, 1   <DO_LOOP>
;<207>                 |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1   <DO_LOOP>
;<112>                 |   |   |   %mod200 = i3 + 2  %  %"jacobian_$NY_fetch";
;<208>                 |   |   |
;<208>                 |   |   |   + DO i4 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1   <DO_LOOP>
;<121>                 |   |   |   |   %mod208 = i4 + 3  %  %"jacobian_$NX_fetch";
;<124>                 |   |   |   |   %"jacobian_$Q[]211[][][]_fetch" = (%"jacobian_$Q")[i2 + 2][i3 + 1][i4 + 1][0];
;<127>                 |   |   |   |   %div238 = (%"jacobian_$Q")[i2 + 2][i3 + 1][i4 + 1][1]  /  %"jacobian_$Q[]211[][][]_fetch";
;<130>                 |   |   |   |   %div261 = (%"jacobian_$Q")[i2 + 2][i3 + 1][i4 + 1][2]  /  %"jacobian_$Q[]211[][][]_fetch";
;<131>                 |   |   |   |   %mul275 = %div238  *  %div238;
;<132>                 |   |   |   |   %mul277 = %div261  *  %div261;
;<133>                 |   |   |   |   %add278 = %mul275  +  %mul277;
;<134>                 |   |   |   |   %mul279 = %add278  *  2.000000e+00;
;<135>                 |   |   |   |   %div281 = %mul279  /  %"jacobian_$Q[]211[][][]_fetch";
;<136>                 |   |   |   |   %func_result273 = @llvm.pow.f64(%div281,  2.500000e+00);
;<140>                 |   |   |   |   %"jacobian_$Q[]284[][][]_fetch" = (%"jacobian_$Q")[i2 + 3][%mod200][%mod208][0];
;<143>                 |   |   |   |   %div309 = (%"jacobian_$Q")[i2 + 3][%mod200][%mod208][1]  /  %"jacobian_$Q[]284[][][]_fetch";
;<146>                 |   |   |   |   %div332 = (%"jacobian_$Q")[i2 + 3][%mod200][%mod208][2]  /  %"jacobian_$Q[]284[][][]_fetch";
;<147>                 |   |   |   |   %mul346 = %div309  *  %div309;
;<148>                 |   |   |   |   %mul348 = %div332  *  %div332;
;<149>                 |   |   |   |   %add349 = %mul346  +  %mul348;
;<150>                 |   |   |   |   %mul350 = %add349  *  2.000000e+00;
;<151>                 |   |   |   |   %div352 = %mul350  /  %"jacobian_$Q[]284[][][]_fetch";
;<152>                 |   |   |   |   %func_result344 = @llvm.pow.f64(%div352,  2.500000e+00);
;<153>                 |   |   |   |   %add354 = %"jacobian_$M.0"  +  %func_result273;
;<154>                 |   |   |   |   %"jacobian_$M.0" = %add354  +  %func_result344;
;<208>                 |   |   |   + END LOOP
;<207>                 |   |   + END LOOP
;<206>                 |   + END LOOP
;<202>                 + END LOOP
;<183>                    %"jacobian_$M.13" = %"jacobian_$M.0";
;<202>
;<187>                 (%"(&)val$")[0][0] = 48;
;<189>                 (%"(&)val$")[0][1] = 1;
;<191>                 (%"(&)val$")[0][2] = 1;
;<193>                 (%"(&)val$")[0][3] = 0;
;<195>                 (%argblock)[0].0 = %"jacobian_$M.13";
;<198>                 %func_result395 = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%"(&)val$")[0][0]),  &((i8*)(%argblock)[0]));
;<2>                }
;<201>              ret ;
;<0>          END REGION
;
;*** IR Dump After HIR Store Result Into Temp Array ***
;Function: jacobian_
;
; CHECK:     BEGIN REGION { modified }
; CHECK:           %array_size = sext.i32.i64(%"jacobian_$NY_fetch") + 1  *  sext.i32.i64(%"jacobian_$NX_fetch") + 1;
; CHECK:           %array_size9 = sext.i32.i64(%"jacobian_$NZ_fetch1") + 4  *  %array_size;
; CHECK:           %TempArray = alloca %array_size9;
; CHECK:           if (umax(%rel9, %rel5, %rel) == 0)
; CHECK:           {
; CHECK:              %"jacobian_$N_fetch" = (%"jacobian_$N")[0];
; CHECK:              %"jacobian_$M.13" = undef;
;
; CHECK:                 %"jacobian_$M.0" = undef;
; CHECK:              + DO i1 = 0, %"jacobian_$N_fetch" + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + 3, 1   <DO_LOOP>
; CHECK:              |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NY_fetch"), 1   <DO_LOOP>
; CHECK:              |   |   |   + DO i4 = 0, sext.i32.i64(%"jacobian_$NX_fetch"), 1   <DO_LOOP>
; CHECK:              |   |   |   |   %"jacobian_$Q[][][][]_fetch" = (%"jacobian_$Q")[i2 + 1][i3 + 1][i4 + 1][0];
; CHECK:              |   |   |   |   %div = (%"jacobian_$Q")[i2 + 1][i3 + 1][i4 + 1][1]  /  %"jacobian_$Q[][][][]_fetch";
; CHECK:              |   |   |   |   %div67 = (%"jacobian_$Q")[i2 + 1][i3 + 1][i4 + 1][2]  /  %"jacobian_$Q[][][][]_fetch";
; CHECK:              |   |   |   |   %mul79 = %div  *  %div;
; CHECK:              |   |   |   |   %mul81 = %div67  *  %div67;
; CHECK:              |   |   |   |   %add82 = %mul79  +  %mul81;
; CHECK:              |   |   |   |   %mul83 = %add82  *  2.000000e+00;
; CHECK:              |   |   |   |   %div85 = %mul83  /  %"jacobian_$Q[][][][]_fetch";
; CHECK:              |   |   |   |   (%TempArray)[i2][i3][i4] = @llvm.pow.f64(%div85,  2.500000e+00);
; CHECK:              |   |   |   + END LOOP
; CHECK:              |   |   + END LOOP
; CHECK:              |   + END LOOP
; CHECK:              |
; CHECK:              |
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + -1, 1   <DO_LOOP>
; CHECK:              |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   |   %mod = i3 + 2  %  %"jacobian_$NY_fetch";
; CHECK:              |   |   |
; CHECK:              |   |   |   + DO i4 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   |   |   %mod33 = i4 + 3  %  %"jacobian_$NX_fetch";
; CHECK:              |   |   |   |   %func_result = (%TempArray)[i2][i3][i4];
; CHECK:              |   |   |   |   %func_result148 = (%TempArray)[i2 + 1][zext.i32.i64(%mod) + -1][zext.i32.i64(%mod33) + -1];
; CHECK:              |   |   |   |   %add157 = %"jacobian_$M.0"  +  %func_result;
; CHECK:              |   |   |   |   %"jacobian_$M.0" = %add157  +  %func_result148;
; CHECK:              |   |   |   + END LOOP
; CHECK:              |   |   + END LOOP
; CHECK:              |   + END LOOP
; CHECK:              |
; CHECK:              |
; CHECK:              |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + 1, 1   <DO_LOOP>
; CHECK:              |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   |   %mod200 = i3 + 2  %  %"jacobian_$NY_fetch";
; CHECK:              |   |   |
; CHECK:              |   |   |   + DO i4 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   |   |   %mod208 = i4 + 3  %  %"jacobian_$NX_fetch";
; CHECK:              |   |   |   |   %func_result273 = (%TempArray)[i2 + 1][i3][i4];
; CHECK:              |   |   |   |   %func_result344 = (%TempArray)[i2 + 2][zext.i32.i64(%mod200) + -1][zext.i32.i64(%mod208) + -1];
; CHECK:              |   |   |   |   %add354 = %"jacobian_$M.0"  +  %func_result273;
; CHECK:              |   |   |   |   %"jacobian_$M.0" = %add354  +  %func_result344;
; CHECK:              |   |   |   + END LOOP
; CHECK:              |   |   + END LOOP
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
; CHECK:                 %"jacobian_$M.13" = %"jacobian_$M.0";
;
; CHECK:              (%"(&)val$")[0][0] = 48;
; CHECK:              (%"(&)val$")[0][1] = 1;
; CHECK:              (%"(&)val$")[0][2] = 1;
; CHECK:              (%"(&)val$")[0][3] = 0;
; CHECK:              (%argblock)[0].0 = %"jacobian_$M.13";
; CHECK:              %func_result395 = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%"(&)val$")[0][0]),  &((i8*)(%argblock)[0]));
; CHECK:           }
; CHECK:           ret ;
; CHECK:     END REGION
;
;Module Before HIR
; ModuleID = 't1.f90'
source_filename = "t1.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @jacobian_(double* noalias nocapture readonly %"jacobian_$Q", i32* noalias nocapture readonly %"jacobian_$NX", i32* noalias nocapture readonly %"jacobian_$NY", i32* noalias nocapture readonly %"jacobian_$NZ", i32* noalias nocapture readonly %"jacobian_$N") local_unnamed_addr #0 {
alloca_0:
  %"var$1" = alloca [8 x i64], align 16
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca { double }, align 8
  %"jacobian_$NX_fetch" = load i32, i32* %"jacobian_$NX", align 1
  %"jacobian_$NY_fetch" = load i32, i32* %"jacobian_$NY", align 1
  %int_sext = sext i32 %"jacobian_$NX_fetch" to i64
  %mul = mul nsw i64 %int_sext, 40
  %int_sext34 = sext i32 %"jacobian_$NY_fetch" to i64
  %mul35 = mul nsw i64 %mul, %int_sext34
  %"jacobian_$NZ_fetch1" = load i32, i32* %"jacobian_$NZ", align 1
  %rel = icmp slt i32 %"jacobian_$NZ_fetch1", 1
  %rel5 = icmp slt i32 %"jacobian_$NY_fetch", 1
  %or.cond = or i1 %rel5, %rel
  %rel9 = icmp slt i32 %"jacobian_$NX_fetch", 1
  %or.cond487 = or i1 %rel9, %or.cond
  br i1 %or.cond487, label %end_label1, label %bb20_else

bb20_else:                                        ; preds = %alloca_0
  %"jacobian_$N_fetch" = load i32, i32* %"jacobian_$N", align 1
  %rel16 = icmp slt i32 %"jacobian_$N_fetch", 1
  br i1 %rel16, label %bb23, label %bb26.preheader.preheader

bb26.preheader.preheader:                         ; preds = %bb20_else
  %0 = add nuw nsw i32 %"jacobian_$NX_fetch", 1
  %1 = add nuw nsw i32 %"jacobian_$NY_fetch", 1
  %2 = add nuw nsw i32 %"jacobian_$NZ_fetch1", 1
  %3 = add nuw nsw i32 %"jacobian_$NZ_fetch1", 3
  %4 = add nuw nsw i32 %"jacobian_$N_fetch", 1
  %wide.trip.count497 = sext i32 %2 to i64
  %wide.trip.count492 = sext i32 %1 to i64
  %wide.trip.count = sext i32 %0 to i64
  %wide.trip.count512 = sext i32 %3 to i64
  br label %bb26.preheader

bb26.preheader:                                   ; preds = %bb26.preheader.preheader, %bb104
  %"jacobian_$L.0" = phi i32 [ %add387, %bb104 ], [ 1, %bb26.preheader.preheader ]
  %"jacobian_$M.0" = phi double [ %add355.lcssa.lcssa.lcssa, %bb104 ], [ undef, %bb26.preheader.preheader ]
  br label %bb30.preheader

bb30.preheader:                                   ; preds = %bb31, %bb26.preheader
  %indvars.iv494 = phi i64 [ %indvars.iv.next495, %bb31 ], [ 1, %bb26.preheader ]
  %"jacobian_$M.1" = phi double [ %add158.lcssa.lcssa, %bb31 ], [ %"jacobian_$M.0", %bb26.preheader ]
  %indvars.iv.next495 = add nuw nsw i64 %indvars.iv494, 1
  %"jacobian_$Q[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul35, double* elementtype(double) %"jacobian_$Q", i64 %indvars.iv.next495)
  %5 = add nuw nsw i64 %indvars.iv494, 2
  %"jacobian_$Q[]88" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul35, double* elementtype(double) %"jacobian_$Q", i64 %5)
  br label %bb35.preheader

bb35.preheader:                                   ; preds = %bb36, %bb30.preheader
  %indvars.iv490 = phi i64 [ %indvars.iv.next491, %bb36 ], [ 1, %bb30.preheader ]
  %"jacobian_$M.2" = phi double [ %add158.lcssa, %bb36 ], [ %"jacobian_$M.1", %bb30.preheader ]
  %indvars.iv.next491 = add nuw nsw i64 %indvars.iv490, 1
  %6 = trunc i64 %indvars.iv.next491 to i32
  %mod = srem i32 %6, %"jacobian_$NY_fetch"
  %"jacobian_$Q[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul, double* elementtype(double) %"jacobian_$Q[]", i64 %indvars.iv490)
  %int_sext97 = zext i32 %mod to i64
  %"jacobian_$Q[]88[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul, double* elementtype(double) %"jacobian_$Q[]88", i64 %int_sext97)
  br label %bb35

bb35:                                             ; preds = %bb35.preheader, %bb35
  %indvars.iv = phi i64 [ 1, %bb35.preheader ], [ %indvars.iv.next, %bb35 ]
  %"jacobian_$M.3" = phi double [ %"jacobian_$M.2", %bb35.preheader ], [ %add158, %bb35 ]
  %7 = trunc i64 %indvars.iv to i32
  %8 = add i32 %7, 2
  %mod33 = srem i32 %8, %"jacobian_$NX_fetch"
  %"jacobian_$Q[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* elementtype(double) %"jacobian_$Q[][]", i64 %indvars.iv)
  %"jacobian_$Q[][][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[][][]", i64 1)
  %"jacobian_$Q[][][][]_fetch" = load double, double* %"jacobian_$Q[][][][]", align 1
  %"jacobian_$Q[]52[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[][][]", i64 2)
  %"jacobian_$Q[]52[][][]_fetch" = load double, double* %"jacobian_$Q[]52[][][]", align 1
  %div = fdiv double %"jacobian_$Q[]52[][][]_fetch", %"jacobian_$Q[][][][]_fetch"
  %"jacobian_$Q[]63[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[][][]", i64 3)
  %"jacobian_$Q[]63[][][]_fetch" = load double, double* %"jacobian_$Q[]63[][][]", align 1
  %div67 = fdiv double %"jacobian_$Q[]63[][][]_fetch", %"jacobian_$Q[][][][]_fetch"
  %mul79 = fmul double %div, %div
  %mul81 = fmul double %div67, %div67
  %add82 = fadd double %mul79, %mul81
  %mul83 = fmul double %add82, 2.000000e+00
  %div85 = fdiv double %mul83, %"jacobian_$Q[][][][]_fetch"
  %func_result = tail call double @llvm.pow.f64(double %div85, double 2.500000e+00)
  %int_sext95 = zext i32 %mod33 to i64
  %"jacobian_$Q[]88[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* elementtype(double) %"jacobian_$Q[]88[]", i64 %int_sext95)
  %"jacobian_$Q[]88[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[]88[][]", i64 1)
  %"jacobian_$Q[]88[][][]_fetch" = load double, double* %"jacobian_$Q[]88[][][]", align 1
  %"jacobian_$Q[]105[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[]88[][]", i64 2)
  %"jacobian_$Q[]105[][][]_fetch" = load double, double* %"jacobian_$Q[]105[][][]", align 1
  %div113 = fdiv double %"jacobian_$Q[]105[][][]_fetch", %"jacobian_$Q[]88[][][]_fetch"
  %"jacobian_$Q[]126[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[]88[][]", i64 3)
  %"jacobian_$Q[]126[][][]_fetch" = load double, double* %"jacobian_$Q[]126[][][]", align 1
  %div136 = fdiv double %"jacobian_$Q[]126[][][]_fetch", %"jacobian_$Q[]88[][][]_fetch"
  %mul150 = fmul double %div113, %div113
  %mul152 = fmul double %div136, %div136
  %add153 = fadd double %mul150, %mul152
  %mul154 = fmul double %add153, 2.000000e+00
  %div156 = fdiv double %mul154, %"jacobian_$Q[]88[][][]_fetch"
  %func_result148 = tail call double @llvm.pow.f64(double %div156, double 2.500000e+00)
  %add157 = fadd double %"jacobian_$M.3", %func_result
  %add158 = fadd double %add157, %func_result148
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %bb36, label %bb35

bb36:                                             ; preds = %bb35
  %add158.lcssa = phi double [ %add158, %bb35 ]
  %exitcond493 = icmp eq i64 %indvars.iv.next491, %wide.trip.count492
  br i1 %exitcond493, label %bb31, label %bb35.preheader

bb31:                                             ; preds = %bb36
  %add158.lcssa.lcssa = phi double [ %add158.lcssa, %bb36 ]
  %exitcond498 = icmp eq i64 %indvars.iv.next495, %wide.trip.count497
  br i1 %exitcond498, label %bb107.preheader.preheader, label %bb30.preheader

bb107.preheader.preheader:                        ; preds = %bb31
  %add158.lcssa.lcssa.lcssa = phi double [ %add158.lcssa.lcssa, %bb31 ]
  br label %bb107.preheader

bb107.preheader:                                  ; preds = %bb107.preheader.preheader, %bb108
  %indvars.iv508 = phi i64 [ 1, %bb107.preheader.preheader ], [ %indvars.iv.next509, %bb108 ]
  %"jacobian_$M.7" = phi double [ %add158.lcssa.lcssa.lcssa, %bb107.preheader.preheader ], [ %add355.lcssa.lcssa, %bb108 ]
  %9 = add nuw nsw i64 %indvars.iv508, 2
  %"jacobian_$Q[]211" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul35, double* elementtype(double) nonnull %"jacobian_$Q", i64 %9)
  %10 = add nuw nsw i64 %indvars.iv508, 3
  %"jacobian_$Q[]284" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 3, i64 1, i64 %mul35, double* elementtype(double) nonnull %"jacobian_$Q", i64 %10)
  br label %bb112.preheader

bb112.preheader:                                  ; preds = %bb113, %bb107.preheader
  %indvars.iv504 = phi i64 [ %indvars.iv.next505, %bb113 ], [ 1, %bb107.preheader ]
  %"jacobian_$M.8" = phi double [ %add355.lcssa, %bb113 ], [ %"jacobian_$M.7", %bb107.preheader ]
  %indvars.iv.next505 = add nuw nsw i64 %indvars.iv504, 1
  %11 = trunc i64 %indvars.iv.next505 to i32
  %mod200 = srem i32 %11, %"jacobian_$NY_fetch"
  %"jacobian_$Q[]211[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul, double* elementtype(double) %"jacobian_$Q[]211", i64 %indvars.iv504)
  %int_sext293 = zext i32 %mod200 to i64
  %"jacobian_$Q[]284[]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 2, i64 1, i64 %mul, double* elementtype(double) %"jacobian_$Q[]284", i64 %int_sext293)
  br label %bb112

bb112:                                            ; preds = %bb112.preheader, %bb112
  %indvars.iv499 = phi i64 [ 1, %bb112.preheader ], [ %indvars.iv.next500, %bb112 ]
  %"jacobian_$M.9" = phi double [ %"jacobian_$M.8", %bb112.preheader ], [ %add355, %bb112 ]
  %12 = trunc i64 %indvars.iv499 to i32
  %13 = add i32 %12, 2
  %mod208 = srem i32 %13, %"jacobian_$NX_fetch"
  %"jacobian_$Q[]211[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* elementtype(double) %"jacobian_$Q[]211[]", i64 %indvars.iv499)
  %"jacobian_$Q[]211[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[]211[][]", i64 1)
  %"jacobian_$Q[]211[][][]_fetch" = load double, double* %"jacobian_$Q[]211[][][]", align 1
  %"jacobian_$Q[]230[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[]211[][]", i64 2)
  %"jacobian_$Q[]230[][][]_fetch" = load double, double* %"jacobian_$Q[]230[][][]", align 1
  %div238 = fdiv double %"jacobian_$Q[]230[][][]_fetch", %"jacobian_$Q[]211[][][]_fetch"
  %"jacobian_$Q[]251[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[]211[][]", i64 3)
  %"jacobian_$Q[]251[][][]_fetch" = load double, double* %"jacobian_$Q[]251[][][]", align 1
  %div261 = fdiv double %"jacobian_$Q[]251[][][]_fetch", %"jacobian_$Q[]211[][][]_fetch"
  %mul275 = fmul double %div238, %div238
  %mul277 = fmul double %div261, %div261
  %add278 = fadd double %mul275, %mul277
  %mul279 = fmul double %add278, 2.000000e+00
  %div281 = fdiv double %mul279, %"jacobian_$Q[]211[][][]_fetch"
  %func_result273 = tail call double @llvm.pow.f64(double %div281, double 2.500000e+00)
  %int_sext291 = zext i32 %mod208 to i64
  %"jacobian_$Q[]284[][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 1, i64 1, i64 40, double* elementtype(double) %"jacobian_$Q[]284[]", i64 %int_sext291)
  %"jacobian_$Q[]284[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[]284[][]", i64 1)
  %"jacobian_$Q[]284[][][]_fetch" = load double, double* %"jacobian_$Q[]284[][][]", align 1
  %"jacobian_$Q[]301[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[]284[][]", i64 2)
  %"jacobian_$Q[]301[][][]_fetch" = load double, double* %"jacobian_$Q[]301[][][]", align 1
  %div309 = fdiv double %"jacobian_$Q[]301[][][]_fetch", %"jacobian_$Q[]284[][][]_fetch"
  %"jacobian_$Q[]322[][][]" = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %"jacobian_$Q[]284[][]", i64 3)
  %"jacobian_$Q[]322[][][]_fetch" = load double, double* %"jacobian_$Q[]322[][][]", align 1
  %div332 = fdiv double %"jacobian_$Q[]322[][][]_fetch", %"jacobian_$Q[]284[][][]_fetch"
  %mul346 = fmul double %div309, %div309
  %mul348 = fmul double %div332, %div332
  %add349 = fadd double %mul346, %mul348
  %mul350 = fmul double %add349, 2.000000e+00
  %div352 = fdiv double %mul350, %"jacobian_$Q[]284[][][]_fetch"
  %func_result344 = tail call double @llvm.pow.f64(double %div352, double 2.500000e+00)
  %add354 = fadd double %"jacobian_$M.9", %func_result273
  %add355 = fadd double %add354, %func_result344
  %indvars.iv.next500 = add nuw nsw i64 %indvars.iv499, 1
  %exitcond503 = icmp eq i64 %indvars.iv.next500, %wide.trip.count
  br i1 %exitcond503, label %bb113, label %bb112

bb113:                                            ; preds = %bb112
  %add355.lcssa = phi double [ %add355, %bb112 ]
  %exitcond507 = icmp eq i64 %indvars.iv.next505, %wide.trip.count492
  br i1 %exitcond507, label %bb108, label %bb112.preheader

bb108:                                            ; preds = %bb113
  %add355.lcssa.lcssa = phi double [ %add355.lcssa, %bb113 ]
  %indvars.iv.next509 = add nuw nsw i64 %indvars.iv508, 1
  %exitcond513 = icmp eq i64 %indvars.iv.next509, %wide.trip.count512
  br i1 %exitcond513, label %bb104, label %bb107.preheader

bb104:                                            ; preds = %bb108
  %add355.lcssa.lcssa.lcssa = phi double [ %add355.lcssa.lcssa, %bb108 ]
  %add387 = add nuw nsw i32 %"jacobian_$L.0", 1
  %exitcond514 = icmp eq i32 %add387, %4
  br i1 %exitcond514, label %bb23.loopexit, label %bb26.preheader

bb23.loopexit:                                    ; preds = %bb104
  %add355.lcssa.lcssa.lcssa.lcssa = phi double [ %add355.lcssa.lcssa.lcssa, %bb104 ]
  br label %bb23

bb23:                                             ; preds = %bb23.loopexit, %bb20_else
  %"jacobian_$M.13" = phi double [ undef, %bb20_else ], [ %add355.lcssa.lcssa.lcssa.lcssa, %bb23.loopexit ]
  %.fca.0.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 0
  store i8 48, i8* %.fca.0.gep, align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 1
  store i8 1, i8* %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 2
  store i8 1, i8* %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], [4 x i8]* %"(&)val$", i64 0, i64 3
  store i8 0, i8* %.fca.3.gep, align 1
  %BLKFIELD_ = getelementptr inbounds { double }, { double }* %argblock, i64 0, i32 0
  store double %"jacobian_$M.13", double* %BLKFIELD_, align 8
  %"(i8*)var$1" = bitcast [8 x i64]* %"var$1" to i8*
  %"(i8*)argblock" = bitcast { double }* %argblock to i8*
  %func_result395 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %"(i8*)var$1", i32 -1, i64 1239157112576, i8* nonnull %.fca.0.gep, i8* nonnull %"(i8*)argblock") #4
  br label %end_label1

end_label1:                                       ; preds = %alloca_0, %bb23
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

; Function Attrs: nounwind readnone speculatable willreturn
declare double @llvm.pow.f64(double, double) #2

; Function Attrs: nofree
declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #3

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind readnone speculatable willreturn }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nounwind }

!omp_offload.info = !{}
