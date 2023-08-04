; There are two loops with same expression tree for the power intrinsics. We use only one extracted loop
; with temp array to store the results for power function.
; The largest loop upper bounds are used for the extracted loop.
; We expand the loop upper bounds by the largest distances for each dimension between minref and maxref.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-store-result-into-temp-array,print<hir>" -hir-create-function-level-region -hir-details -disable-output 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Store Result Into Temp Array ***
;Function: jacobian_

;      BEGIN REGION { }
;               %"jacobian_$M.0" = undef;
;            + DO i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;            |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;            |   |   %mod = i2 + 2  %  %"jacobian_$NY_fetch";
;            |   |
;            |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;            |   |   |   %mod31 = i3 + 3  %  %"jacobian_$NX_fetch";
;            |   |   |   %"jacobian_$Q[][][][]_fetch" = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3+ 1][0];
;            |   |   |   %div = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][1]  /  %"jacobian_$Q[][][][]_fetch";
;            |   |   |   %div65 = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][2]  /  %"jacobian_$Q[][][][]_fetch";
;            |   |   |   %mul77 = %div  *  %div;
;            |   |   |   %mul79 = %div65  *  %div65;
;            |   |   |   %add80 = %mul77  +  %mul79;
;            |   |   |   %mul81 = %add80  *  2.000000e+00;
;            |   |   |   %div83 = %mul81  /  %"jacobian_$Q[][][][]_fetch";
;            |   |   |   %func_result = @llvm.pow.f64(%div83,  2.500000e+00);
;            |   |   |   %"jacobian_$Q[]86[][][]_fetch" = (%"jacobian_$Q")[i1 + 2][%mod][%mod31][0];
;            |   |   |   %div111 = (%"jacobian_$Q")[i1 + 2][%mod][%mod31][1]  /  %"jacobian_$Q[]86[][][]_fetch";
;            |   |   |   %div134 = (%"jacobian_$Q")[i1 + 2][%mod][%mod31][2]  /  %"jacobian_$Q[]86[][][]_fetch";
;            |   |   |   %mul148 = %div111  *  %div111;
;            |   |   |   %mul150 = %div134  *  %div134;
;            |   |   |   %add151 = %mul148  +  %mul150;
;            |   |   |   %mul152 = %add151  *  2.000000e+00;
;            |   |   |   %div154 = %mul152  /  %"jacobian_$Q[]86[][][]_fetch";
;            |   |   |   %func_result146 = @llvm.pow.f64(%div154,  2.500000e+00);
;            |   |   |   %add155 = %"jacobian_$M.0"  +  %func_result;
;            |   |   |   %"jacobian_$M.0" = %add155  +  %func_result146;
;            |   |   + END LOOP
;            |   + END LOOP
;            + END LOOP
;

;               %"jacobian_$M.6" = %"jacobian_$M.0";
;            + DO i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + 1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483649>  <LEGAL_MAX_TC = 2147483649>
;            |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;            |   |   %mod198 = i2 + 2  %  %"jacobian_$NY_fetch";
;            |   |
;            |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
;            |   |   |   %mod206 = i3 + 3  %  %"jacobian_$NX_fetch";
;            |   |   |   %"jacobian_$Q[]209[][][]_fetch" = (%"jacobian_$Q")[i1 + 2][i2 + 1][i3 + 1][0];
;            |   |   |   %div236 = (%"jacobian_$Q")[i1 + 2][i2 + 1][i3 + 1][1]  /  %"jacobian_$Q[]209[][][]_fetch";
;            |   |   |   %div259 = (%"jacobian_$Q")[i1 + 2][i2 + 1][i3 + 1][2]  /  %"jacobian_$Q[]209[][][]_fetch";
;            |   |   |   %mul273 = %div236  *  %div236;
;            |   |   |   %mul275 = %div259  *  %div259;
;            |   |   |   %add276 = %mul273  +  %mul275;
;            |   |   |   %mul277 = %add276  *  2.000000e+00;
;            |   |   |   %div279 = %mul277  /  %"jacobian_$Q[]209[][][]_fetch";
;            |   |   |   %func_result271 = @llvm.pow.f64(%div279,  2.500000e+00);
;            |   |   |   %"jacobian_$Q[]282[][][]_fetch" = (%"jacobian_$Q")[i1 + 3][%mod198][%mod206][0];
;            |   |   |   %div307 = (%"jacobian_$Q")[i1 + 3][%mod198][%mod206][1]  /  %"jacobian_$Q[]282[][][]_fetch";
;            |   |   |   %div330 = (%"jacobian_$Q")[i1 + 3][%mod198][%mod206][2]  /  %"jacobian_$Q[]282[][][]_fetch";
;            |   |   |   %mul344 = %div307  *  %div307;
;            |   |   |   %mul346 = %div330  *  %div330;
;            |   |   |   %add347 = %mul344  +  %mul346;
;            |   |   |   %mul348 = %add347  *  2.000000e+00;
;            |   |   |   %div350 = %mul348  /  %"jacobian_$Q[]282[][][]_fetch";
;            |   |   |   %func_result342 = @llvm.pow.f64(%div350,  2.500000e+00);
;            |   |   |   %add352 = %"jacobian_$M.6"  +  %func_result271;
;            |   |   |   %"jacobian_$M.6" = %add352  +  %func_result342;
;            |   |   + END LOOP
;            |   + END LOOP
;            + END LOOP

;
;*** IR Dump After HIR Store Result Into Temp Array ***
;Function: jacobian_
;
; CHECK:    BEGIN REGION { modified }
; CHECK:           %array_size = sext.i32.i64(%"jacobian_$NY_fetch") + 1  *  sext.i32.i64(%"jacobian_$NX_fetch") + 1;
; CHECK:           %array_size7 = sext.i32.i64(%"jacobian_$NZ_fetch1") + 4  *  %array_size;
; CHECK:           %TempArray = alloca %array_size7;
;
; CHECK:              + DO i64 i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + 3, 1   <DO_LOOP>
; CHECK:              |   + DO i64 i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch"), 1   <DO_LOOP>
; CHECK:              |   |   + DO i64 i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch"), 1   <DO_LOOP>
; CHECK:              |   |   |   %"jacobian_$Q[][][][]_fetch" = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][0];
; CHECK:              |   |   |   %div = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][1]  /  %"jacobian_$Q[][][][]_fetch";
; CHECK:              |   |   |   %div65 = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][2]  /  %"jacobian_$Q[][][][]_fetch";
; CHECK:              |   |   |   %mul77 = %div  *  %div;
; CHECK:              |   |   |   %mul79 = %div65  *  %div65;
; CHECK:              |   |   |   %add80 = %mul77  +  %mul79;
; CHECK:              |   |   |   %mul81 = %add80  *  2.000000e+00;
; CHECK:              |   |   |   %div83 = %mul81  /  %"jacobian_$Q[][][][]_fetch";
; CHECK:              |   |   |   (%TempArray)[i1][i2][i3] = @llvm.pow.f64(%div83,  2.500000e+00);
; CHECK:              |   |   |      <LVAL-REG> (LINEAR {{.*}} %TempArray)[LINEAR i64 i1][LINEAR i64 i2][LINEAR i64 i3] inbounds  {sb:[[ALLOCASB:.*]]}

; CHECK:              |   |   + END LOOP
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
;
;
; CHECK:              + DO i64 i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + -1, 1   <DO_LOOP>
; CHECK:              |   + DO i64 i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   %mod = i2 + 2  %  %"jacobian_$NY_fetch";
; CHECK:              |   |
; CHECK:              |   |   + DO i64 i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   |   %mod31 = i3 + 3  %  %"jacobian_$NX_fetch";
; CHECK:              |   |   |   %func_result = (%TempArray)[i1][i2][i3];
; CHECK:              |   |   |      <RVAL-REG> (LINEAR {{.*}} %TempArray)[LINEAR i64 i1][LINEAR i64 i2][LINEAR i64 i3] inbounds  {sb:[[ALLOCASB]]}

; CHECK:              |   |   |   %func_result146 = (%TempArray)[i1 + 1][zext.i32.i64(%mod) + -1][zext.i32.i64(%mod31) + -1];
; CHECK:              |   |   |   %add155 = %"jacobian_$M.0"  +  %func_result;
; CHECK:              |   |   |   %"jacobian_$M.0" = %add155  +  %func_result146;
; CHECK:              |   |   + END LOOP
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP
;
; CHECK:              %"jacobian_$M.6" = %"jacobian_$M.0";
;
; CHECK:              + DO i64 i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + 1, 1   <DO_LOOP>
; CHECK:              |   + DO i64 i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   %mod198 = i2 + 2  %  %"jacobian_$NY_fetch";
; CHECK:              |   |
; CHECK:              |   |   + DO i64 i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1   <DO_LOOP>
; CHECK:              |   |   |   %mod206 = i3 + 3  %  %"jacobian_$NX_fetch";
; CHECK:              |   |   |   %func_result271 = (%TempArray)[i1 + 1][i2][i3];
; CHECK:              |   |   |   <RVAL-REG> (LINEAR {{.*}} %TempArray)[LINEAR i64 i1 + 1][LINEAR i64 i2][LINEAR i64 i3] inbounds  {sb:[[ALLOCASB]]}

; CHECK:              |   |   |   %func_result342 = (%TempArray)[i1 + 2][zext.i32.i64(%mod198) + -1][zext.i32.i64(%mod206) + -1];
; CHECK:              |   |   |   %add352 = %"jacobian_$M.6"  +  %func_result271;
; CHECK:              |   |   |   %"jacobian_$M.6" = %add352  +  %func_result342;
; CHECK:              |   |   + END LOOP
; CHECK:              |   + END LOOP
; CHECK:              + END LOOP


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nounwind uwtable
define void @jacobian_(ptr noalias nocapture readonly %"jacobian_$Q", ptr noalias nocapture readonly %"jacobian_$NX", ptr noalias nocapture readonly %"jacobian_$NY", ptr noalias nocapture readonly %"jacobian_$NZ") local_unnamed_addr #0 {
alloca_0:
  %"var$1" = alloca [8 x i64], align 16
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca { double }, align 8
  %"jacobian_$NX_fetch" = load i32, ptr %"jacobian_$NX", align 1
  %"jacobian_$NY_fetch" = load i32, ptr %"jacobian_$NY", align 1
  %int_sext = sext i32 %"jacobian_$NX_fetch" to i64
  %mul = mul nsw i64 %int_sext, 40
  %int_sext32 = sext i32 %"jacobian_$NY_fetch" to i64
  %mul33 = mul nsw i64 %mul, %int_sext32
  %"jacobian_$NZ_fetch1" = load i32, ptr %"jacobian_$NZ", align 1
  %rel = icmp slt i32 %"jacobian_$NZ_fetch1", 1
  %rel5 = icmp slt i32 %"jacobian_$NY_fetch", 1
  %or.cond = or i1 %rel5, %rel
  %rel9 = icmp slt i32 %"jacobian_$NX_fetch", 1
  %or.cond475 = or i1 %rel9, %or.cond
  br i1 %or.cond475, label %end_label1, label %bb26.preheader.preheader

bb26.preheader.preheader:                         ; preds = %alloca_0
  %0 = add nuw nsw i32 %"jacobian_$NX_fetch", 1
  %1 = add nuw nsw i32 %"jacobian_$NY_fetch", 1
  %2 = add nuw nsw i32 %"jacobian_$NZ_fetch1", 1
  %wide.trip.count500 = sext i32 %2 to i64
  %wide.trip.count495 = sext i32 %1 to i64
  %wide.trip.count491 = sext i32 %0 to i64
  br label %bb26.preheader

bb26.preheader:                                   ; preds = %bb26.preheader.preheader, %bb27
  %indvars.iv497 = phi i64 [ 1, %bb26.preheader.preheader ], [ %indvars.iv.next498, %bb27 ]
  %"jacobian_$M.0" = phi double [ undef, %bb26.preheader.preheader ], [ %add156.lcssa.lcssa, %bb27 ]
  %indvars.iv.next498 = add nuw nsw i64 %indvars.iv497, 1
  %"jacobian_$Q[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul33, ptr elementtype(double) %"jacobian_$Q", i64 %indvars.iv.next498)
  %3 = add nuw nsw i64 %indvars.iv497, 2
  %"jacobian_$Q[]86" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul33, ptr elementtype(double) %"jacobian_$Q", i64 %3)
  br label %bb31.preheader

bb31.preheader:                                   ; preds = %bb32, %bb26.preheader
  %indvars.iv493 = phi i64 [ %indvars.iv.next494, %bb32 ], [ 1, %bb26.preheader ]
  %"jacobian_$M.1" = phi double [ %add156.lcssa, %bb32 ], [ %"jacobian_$M.0", %bb26.preheader ]
  %indvars.iv.next494 = add nuw nsw i64 %indvars.iv493, 1
  %4 = trunc i64 %indvars.iv.next494 to i32
  %mod = srem i32 %4, %"jacobian_$NY_fetch"
  %"jacobian_$Q[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) %"jacobian_$Q[]", i64 %indvars.iv493)
  %int_sext95 = zext i32 %mod to i64
  %"jacobian_$Q[]86[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) %"jacobian_$Q[]86", i64 %int_sext95)
  br label %bb31

bb31:                                             ; preds = %bb31.preheader, %bb31
  %indvars.iv488 = phi i64 [ 1, %bb31.preheader ], [ %indvars.iv.next489, %bb31 ]
  %"jacobian_$M.2" = phi double [ %"jacobian_$M.1", %bb31.preheader ], [ %add156, %bb31 ]
  %5 = trunc i64 %indvars.iv488 to i32
  %6 = add i32 %5, 2
  %mod31 = srem i32 %6, %"jacobian_$NX_fetch"
  %"jacobian_$Q[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) %"jacobian_$Q[][]", i64 %indvars.iv488)
  %"jacobian_$Q[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[][][]", i64 1)
  %"jacobian_$Q[][][][]_fetch" = load double, ptr %"jacobian_$Q[][][][]", align 1
  %"jacobian_$Q[]50[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[][][]", i64 2)
  %"jacobian_$Q[]50[][][]_fetch" = load double, ptr %"jacobian_$Q[]50[][][]", align 1
  %div = fdiv double %"jacobian_$Q[]50[][][]_fetch", %"jacobian_$Q[][][][]_fetch"
  %"jacobian_$Q[]61[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[][][]", i64 3)
  %"jacobian_$Q[]61[][][]_fetch" = load double, ptr %"jacobian_$Q[]61[][][]", align 1
  %div65 = fdiv double %"jacobian_$Q[]61[][][]_fetch", %"jacobian_$Q[][][][]_fetch"
  %mul77 = fmul double %div, %div
  %mul79 = fmul double %div65, %div65
  %add80 = fadd double %mul77, %mul79
  %mul81 = fmul double %add80, 2.000000e+00
  %div83 = fdiv double %mul81, %"jacobian_$Q[][][][]_fetch"
  %func_result = tail call double @llvm.pow.f64(double %div83, double 2.500000e+00)
  %int_sext93 = zext i32 %mod31 to i64
  %"jacobian_$Q[]86[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) %"jacobian_$Q[]86[]", i64 %int_sext93)
  %"jacobian_$Q[]86[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]86[][]", i64 1)
  %"jacobian_$Q[]86[][][]_fetch" = load double, ptr %"jacobian_$Q[]86[][][]", align 1
  %"jacobian_$Q[]103[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]86[][]", i64 2)
  %"jacobian_$Q[]103[][][]_fetch" = load double, ptr %"jacobian_$Q[]103[][][]", align 1
  %div111 = fdiv double %"jacobian_$Q[]103[][][]_fetch", %"jacobian_$Q[]86[][][]_fetch"
  %"jacobian_$Q[]124[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]86[][]", i64 3)
  %"jacobian_$Q[]124[][][]_fetch" = load double, ptr %"jacobian_$Q[]124[][][]", align 1
  %div134 = fdiv double %"jacobian_$Q[]124[][][]_fetch", %"jacobian_$Q[]86[][][]_fetch"
  %mul148 = fmul double %div111, %div111
  %mul150 = fmul double %div134, %div134
  %add151 = fadd double %mul148, %mul150
  %mul152 = fmul double %add151, 2.000000e+00
  %div154 = fdiv double %mul152, %"jacobian_$Q[]86[][][]_fetch"
  %func_result146 = tail call double @llvm.pow.f64(double %div154, double 2.500000e+00)
  %add155 = fadd double %"jacobian_$M.2", %func_result
  %add156 = fadd double %add155, %func_result146
  %indvars.iv.next489 = add nuw nsw i64 %indvars.iv488, 1
  %exitcond492 = icmp eq i64 %indvars.iv.next489, %wide.trip.count491
  br i1 %exitcond492, label %bb32, label %bb31

bb32:                                             ; preds = %bb31
  %add156.lcssa = phi double [ %add156, %bb31 ]
  %exitcond496 = icmp eq i64 %indvars.iv.next494, %wide.trip.count495
  br i1 %exitcond496, label %bb27, label %bb31.preheader

bb27:                                             ; preds = %bb32
  %add156.lcssa.lcssa = phi double [ %add156.lcssa, %bb32 ]
  %exitcond501 = icmp eq i64 %indvars.iv.next498, %wide.trip.count500
  br i1 %exitcond501, label %bb103.preheader.preheader, label %bb26.preheader

bb103.preheader.preheader:                        ; preds = %bb27
  %add156.lcssa.lcssa.lcssa = phi double [ %add156.lcssa.lcssa, %bb27 ]
  %7 = add nsw i32 %"jacobian_$NZ_fetch1", 3
  %wide.trip.count486 = sext i32 %7 to i64
  br label %bb103.preheader

bb103.preheader:                                  ; preds = %bb103.preheader.preheader, %bb104
  %indvars.iv482 = phi i64 [ 1, %bb103.preheader.preheader ], [ %indvars.iv.next483, %bb104 ]
  %"jacobian_$M.6" = phi double [ %add156.lcssa.lcssa.lcssa, %bb103.preheader.preheader ], [ %add353.lcssa.lcssa, %bb104 ]
  %8 = add nuw nsw i64 %indvars.iv482, 2
  %"jacobian_$Q[]209" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul33, ptr elementtype(double) nonnull %"jacobian_$Q", i64 %8)
  %9 = add nuw nsw i64 %indvars.iv482, 3
  %"jacobian_$Q[]282" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul33, ptr elementtype(double) nonnull %"jacobian_$Q", i64 %9)
  br label %bb108.preheader

bb108.preheader:                                  ; preds = %bb109, %bb103.preheader
  %indvars.iv478 = phi i64 [ %indvars.iv.next479, %bb109 ], [ 1, %bb103.preheader ]
  %"jacobian_$M.7" = phi double [ %add353.lcssa, %bb109 ], [ %"jacobian_$M.6", %bb103.preheader ]
  %indvars.iv.next479 = add nuw nsw i64 %indvars.iv478, 1
  %10 = trunc i64 %indvars.iv.next479 to i32
  %mod198 = srem i32 %10, %"jacobian_$NY_fetch"
  %"jacobian_$Q[]209[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) %"jacobian_$Q[]209", i64 %indvars.iv478)
  %int_sext291 = zext i32 %mod198 to i64
  %"jacobian_$Q[]282[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) %"jacobian_$Q[]282", i64 %int_sext291)
  br label %bb108

bb108:                                            ; preds = %bb108.preheader, %bb108
  %indvars.iv = phi i64 [ 1, %bb108.preheader ], [ %indvars.iv.next, %bb108 ]
  %"jacobian_$M.8" = phi double [ %"jacobian_$M.7", %bb108.preheader ], [ %add353, %bb108 ]
  %11 = trunc i64 %indvars.iv to i32
  %12 = add i32 %11, 2
  %mod206 = srem i32 %12, %"jacobian_$NX_fetch"
  %"jacobian_$Q[]209[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) %"jacobian_$Q[]209[]", i64 %indvars.iv)
  %"jacobian_$Q[]209[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]209[][]", i64 1)
  %"jacobian_$Q[]209[][][]_fetch" = load double, ptr %"jacobian_$Q[]209[][][]", align 1
  %"jacobian_$Q[]228[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]209[][]", i64 2)
  %"jacobian_$Q[]228[][][]_fetch" = load double, ptr %"jacobian_$Q[]228[][][]", align 1
  %div236 = fdiv double %"jacobian_$Q[]228[][][]_fetch", %"jacobian_$Q[]209[][][]_fetch"
  %"jacobian_$Q[]249[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]209[][]", i64 3)
  %"jacobian_$Q[]249[][][]_fetch" = load double, ptr %"jacobian_$Q[]249[][][]", align 1
  %div259 = fdiv double %"jacobian_$Q[]249[][][]_fetch", %"jacobian_$Q[]209[][][]_fetch"
  %mul273 = fmul double %div236, %div236
  %mul275 = fmul double %div259, %div259
  %add276 = fadd double %mul273, %mul275
  %mul277 = fmul double %add276, 2.000000e+00
  %div279 = fdiv double %mul277, %"jacobian_$Q[]209[][][]_fetch"
  %func_result271 = tail call double @llvm.pow.f64(double %div279, double 2.500000e+00)
  %int_sext289 = zext i32 %mod206 to i64
  %"jacobian_$Q[]282[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) %"jacobian_$Q[]282[]", i64 %int_sext289)
  %"jacobian_$Q[]282[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]282[][]", i64 1)
  %"jacobian_$Q[]282[][][]_fetch" = load double, ptr %"jacobian_$Q[]282[][][]", align 1
  %"jacobian_$Q[]299[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]282[][]", i64 2)
  %"jacobian_$Q[]299[][][]_fetch" = load double, ptr %"jacobian_$Q[]299[][][]", align 1
  %div307 = fdiv double %"jacobian_$Q[]299[][][]_fetch", %"jacobian_$Q[]282[][][]_fetch"
  %"jacobian_$Q[]320[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]282[][]", i64 3)
  %"jacobian_$Q[]320[][][]_fetch" = load double, ptr %"jacobian_$Q[]320[][][]", align 1
  %div330 = fdiv double %"jacobian_$Q[]320[][][]_fetch", %"jacobian_$Q[]282[][][]_fetch"
  %mul344 = fmul double %div307, %div307
  %mul346 = fmul double %div330, %div330
  %add347 = fadd double %mul344, %mul346
  %mul348 = fmul double %add347, 2.000000e+00
  %div350 = fdiv double %mul348, %"jacobian_$Q[]282[][][]_fetch"
  %func_result342 = tail call double @llvm.pow.f64(double %div350, double 2.500000e+00)
  %add352 = fadd double %"jacobian_$M.8", %func_result271
  %add353 = fadd double %add352, %func_result342
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count491
  br i1 %exitcond, label %bb109, label %bb108

bb109:                                            ; preds = %bb108
  %add353.lcssa = phi double [ %add353, %bb108 ]
  %exitcond481 = icmp eq i64 %indvars.iv.next479, %wide.trip.count495
  br i1 %exitcond481, label %bb104, label %bb108.preheader

bb104:                                            ; preds = %bb109
  %add353.lcssa.lcssa = phi double [ %add353.lcssa, %bb109 ]
  %indvars.iv.next483 = add nuw nsw i64 %indvars.iv482, 1
  %exitcond487 = icmp eq i64 %indvars.iv.next483, %wide.trip.count486
  br i1 %exitcond487, label %bb100, label %bb103.preheader

bb100:                                            ; preds = %bb104
  %add353.lcssa.lcssa.lcssa = phi double [ %add353.lcssa.lcssa, %bb104 ]
  store i8 48, ptr %"(&)val$", align 1
  %.fca.1.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i64 0, i64 1
  store i8 1, ptr %.fca.1.gep, align 1
  %.fca.2.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i64 0, i64 2
  store i8 1, ptr %.fca.2.gep, align 1
  %.fca.3.gep = getelementptr inbounds [4 x i8], ptr %"(&)val$", i64 0, i64 3
  store i8 0, ptr %.fca.3.gep, align 1
  store double %add353.lcssa.lcssa.lcssa, ptr %argblock, align 8
  %func_result385 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %"var$1", i32 -1, i64 1239157112576, ptr nonnull %"(&)val$", ptr nonnull %argblock) #4
  br label %end_label1

end_label1:                                       ; preds = %alloca_0, %bb100
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #1

; Function Attrs: nounwind readnone speculatable willreturn
declare double @llvm.pow.f64(double, double) #2

; Function Attrs: nofree
declare i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr #3

attributes #0 = { nofree nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind readnone speculatable willreturn }
attributes #3 = { nofree "intel-lang"="fortran" }
attributes #4 = { nounwind }

!omp_offload.info = !{}
