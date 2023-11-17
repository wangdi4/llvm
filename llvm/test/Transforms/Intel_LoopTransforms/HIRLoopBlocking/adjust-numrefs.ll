; REQUIRES: asserts
; RUN: opt -mattr=+avx2 -enable-intel-advanced-opts -intel-libirc-allowed -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-sinking-for-perfect-loopnest,hir-loop-blocking,print<hir>' -disable-output -debug-only=hir-loop-blocking 2>&1 < %s | FileCheck %s

; Verify that the loop is not blocked.
; There are less than 20% of memrefs that has missing IVs or outer level IVs in the innermost dimension.

; Before and after of loop blocking
;
;               if (%slct.1 >= %KTS_fetch.9)
;               {
;                  + DO i1 = 0, -1 * sext.i32.i64(%JTS_fetch.11) + (-2 + %wide.trip.count) + 2, 1   <DO_LOOP>
;                  |   + DO i2 = 0, zext.i32.i64(((-1 * %KTS_fetch.9) + %slct.1)), 1   <DO_LOOP>
;                  |   |   + DO i3 = 0, zext.i32.i64((2 + (-1 * %ITS_fetch.7) + %slct.2)), 1   <DO_LOOP>
;                  |   |   |   %"RDZW[]_fetch.184" = (%RDZW)[i2 + sext.i32.i64(%KTS_fetch.9)];
;                  |   |   |   %add.10 = (%MU_OLD)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i3 + sext.i32.i64((-1 + %ITS_fetch.7))]  +  (%MUB)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %mul.12 = %add.10  *  (%FIELD_OLD)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %"MSFTY[][]_fetch.106" = (%MSFTY)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %mul.19 = %"MSFTY[][]_fetch.106"  *  (%MSFTX)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %sub.30 = (%FQXL)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7)) + 1]  -  (%FQXL)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %mul.25 = %sub.30  *  %RDX_fetch.107;
;                  |   |   |   %sub.43 = (%FQYL)[i1 + sext.i32.i64(%JTS_fetch.11)][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))]  -  (%FQYL)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %mul.31 = %sub.43  *  %RDY_fetch.140;
;                  |   |   |   %add.41 = %mul.31  +  %mul.25;
;                  |   |   |   %mul.32 = %add.41  *  %mul.19;
;                  |   |   |   %mul.34 = %"RDZW[]_fetch.184"  *  %"MSFTY[][]_fetch.106";
;                  |   |   |   %sub.56 = (%FQZL)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9) + 1][i3 + sext.i32.i64((-1 + %ITS_fetch.7))]  -  (%FQZL)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %mul.40 = %sub.56  *  %mul.34;
;                  |   |   |   %add.51 = %mul.40  +  %mul.32;
;                  |   |   |   %mul.41 = %add.51  *  %DT_fetch.80;
;                  |   |   |   %sub.57 = %mul.12  -  %mul.41;
;                  |   |   |   %"FQX[][][]_fetch.254" = (%FQX)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7)) + 1];
;                  |   |   |   %func_result = @llvm.maxnum.f32(%"FQX[][][]_fetch.254",  0.000000e+00);
;                  |   |   |   %"FQX[][][]_fetch.264" = (%FQX)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %func_result114 = @llvm.minnum.f32(%"FQX[][][]_fetch.264",  0.000000e+00);
;                  |   |   |   %sub.70 = %func_result  -  %func_result114;
;                  |   |   |   %mul.48 = %sub.70  *  %RDX_fetch.107;
;                  |   |   |   %"FQY[][][]_fetch.287" = (%FQY)[i1 + sext.i32.i64(%JTS_fetch.11)][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %func_result131 = @llvm.maxnum.f32(%"FQY[][][]_fetch.287",  0.000000e+00);
;                  |   |   |   %func_result146 = @llvm.minnum.f32((%FQY)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))],  0.000000e+00);
;                  |   |   |   %sub.83 = %func_result131  -  %func_result146;
;                  |   |   |   %mul.54 = %sub.83  *  %RDY_fetch.140;
;                  |   |   |   %add.70 = %mul.54  +  %mul.48;
;                  |   |   |   %mul.55 = %add.70  *  %mul.19;
;                  |   |   |   %"FQZ[][][]_fetch.330" = (%FQZ)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9) + 1][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |   %func_result177 = @llvm.minnum.f32(%"FQZ[][][]_fetch.330",  0.000000e+00);
;                  |   |   |   %func_result192 = @llvm.maxnum.f32((%FQZ)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))],  0.000000e+00);
;                  |   |   |   %sub.96 = %func_result177  -  %func_result192;
;                  |   |   |   %mul.62 = %sub.96  *  %mul.34;
;                  |   |   |   %add.80 = %mul.55  +  %mul.62;
;                  |   |   |   %mul.63 = %add.80  *  %DT_fetch.80;
;                  |   |   |   if (%mul.63 > %sub.57)
;                  |   |   |   {
;                  |   |   |      %add.81 = %mul.63  +  0x3BC79CA100000000;
;                  |   |   |      %div.1 = %sub.57  /  %add.81;
;                  |   |   |      %func_result215 = @llvm.maxnum.f32(%div.1,  0.000000e+00);
;                  |   |   |      if (%"FQX[][][]_fetch.254" > 0.000000e+00)
;                  |   |   |      {
;                  |   |   |         %mul.64 = %func_result215  *  %"FQX[][][]_fetch.254";
;                  |   |   |         (%FQX)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7)) + 1] = %mul.64;
;                  |   |   |      }
;                  |   |   |      if (%"FQX[][][]_fetch.264" < 0.000000e+00)
;                  |   |   |      {
;                  |   |   |         %mul.65 = %"FQX[][][]_fetch.264"  *  %func_result215;
;                  |   |   |         (%FQX)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))] = %mul.65;
;                  |   |   |      }
;                  |   |   |      if (%"FQY[][][]_fetch.287" > 0.000000e+00)
;                  |   |   |      {
;                  |   |   |         %mul.66 = %func_result215  *  %"FQY[][][]_fetch.287";
;                  |   |   |         (%FQY)[i1 + sext.i32.i64(%JTS_fetch.11)][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))] = %mul.66;
;                  |   |   |      }
;                  |   |   |      %"FQY[][][]_fetch.444" = (%FQY)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |      if (%"FQY[][][]_fetch.444" < 0.000000e+00)
;                  |   |   |      {
;                  |   |   |         %mul.67 = %"FQY[][][]_fetch.444"  *  %func_result215;
;                  |   |   |         (%FQY)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))] = %mul.67;
;                  |   |   |      }
;                  |   |   |      if (%"FQZ[][][]_fetch.330" < 0.000000e+00)
;                  |   |   |      {
;                  |   |   |         %mul.68 = %func_result215  *  %"FQZ[][][]_fetch.330";
;                  |   |   |         (%FQZ)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9) + 1][i3 + sext.i32.i64((-1 + %ITS_fetch.7))] = %mul.68;
;                  |   |   |      }
;                  |   |   |      %"FQZ[][][]_fetch.504" = (%FQZ)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))];
;                  |   |   |      if (%"FQZ[][][]_fetch.504" > 0.000000e+00)
;                  |   |   |      {
;                  |   |   |         %mul.69 = %"FQZ[][][]_fetch.504"  *  %func_result215;
;                  |   |   |         (%FQZ)[i1 + sext.i32.i64(%JTS_fetch.11) + -1][i2 + sext.i32.i64(%KTS_fetch.9)][i3 + sext.i32.i64((-1 + %ITS_fetch.7))] = %mul.69;
;                  |   |   |      }
;                  |   |   |   }
;                  |   |   + END LOOP
;                  |   + END LOOP
;                  + END LOOP
;         }

; CHECK: Ref statistics before adjustment for Region: 0
; CHECK:   # total refs: 26
; CHECK: Level 1: # NumRefsMissingAtLevel: 1, # NumRefsWithSmallStrides: 0
; CHECK: Level 2: # NumRefsMissingAtLevel: 4, # NumRefsWithSmallStrides: 1
; CHECK: Level 3: # NumRefsMissingAtLevel: 1, # NumRefsWithSmallStrides: 0
; CHECK: Ref statistics after adjustment for Region: 0
; CHECK:   # total refs: 26
; CHECK: Level 1: # NumRefsMissingAtLevel: 0, # NumRefsWithSmallStrides: 0
; CHECK: Level 2: # NumRefsMissingAtLevel: 0, # NumRefsWithSmallStrides: 0
; CHECK: Level 3: # NumRefsMissingAtLevel: 0, # NumRefsWithSmallStrides: 0

; CHECK:    BEGIN REGION { modified }
; CHECK:      DO i1
; CHECK:      DO i2
; CHECK:      DO i3
; CHECK-NOT:  DO i4


; The following commands show that in the old setting, where disable-hir-loop-blocking-loop-depth-check-advanced was true, loopnest was not loop-blocked by
; bailing out logic of depth-check.
; RUN: opt -mattr=+avx2 -enable-intel-advanced-opts -intel-libirc-allowed -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-sinking-for-perfect-loopnest,hir-loop-blocking,print<hir>' -disable-output -debug-only=hir-loop-blocking -disable-hir-loop-blocking-loop-depth-check-advanced=false 2>&1 < %s | FileCheck %s --check-prefix OLD

; OLD: Failed: at MaxDimension < LoopNestDepth 3,3

; ModuleID = 'test-short.f90'
source_filename = "test-short.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree nosync nounwind memory(argmem: readwrite) uwtable
define void @module_advect_em_mp_advect_scalar_pd_(ptr noalias nocapture readnone dereferenceable(4) %FIELD, ptr noalias nocapture readonly dereferenceable(4) %FIELD_OLD, ptr noalias nocapture readnone dereferenceable(4) %TENDENCY, ptr noalias nocapture readnone dereferenceable(4) %H_TENDENCY, ptr noalias nocapture readnone dereferenceable(4) %Z_TENDENCY, ptr noalias nocapture readnone dereferenceable(4) %RU, ptr noalias nocapture readnone dereferenceable(4) %RV, ptr noalias nocapture readnone dereferenceable(4) %ROM, ptr noalias nocapture readnone dereferenceable(4) %MUT, ptr noalias nocapture readonly dereferenceable(4) %MUB, ptr noalias nocapture readonly dereferenceable(4) %MU_OLD, ptr noalias nocapture readonly dereferenceable(4) %TIME_STEP, ptr noalias nocapture readonly dereferenceable(35164) %CONFIG_FLAGS, ptr noalias nocapture readonly dereferenceable(4) %TENDDEC, ptr noalias nocapture readnone dereferenceable(4) %MSFUX, ptr noalias nocapture readnone dereferenceable(4) %MSFUY, ptr noalias nocapture readnone dereferenceable(4) %MSFVX, ptr noalias nocapture readnone dereferenceable(4) %MSFVY, ptr noalias nocapture readonly dereferenceable(4) %MSFTX, ptr noalias nocapture readonly dereferenceable(4) %MSFTY, ptr noalias nocapture readnone dereferenceable(4) %FZM, ptr noalias nocapture readnone dereferenceable(4) %FZP, ptr noalias nocapture readonly dereferenceable(4) %RDX, ptr noalias nocapture readonly dereferenceable(4) %RDY, ptr noalias nocapture readonly dereferenceable(4) %RDZW, ptr noalias nocapture readonly dereferenceable(4) %DT, ptr noalias nocapture readonly dereferenceable(4) %IDS, ptr noalias nocapture readonly dereferenceable(4) %IDE, ptr noalias nocapture readonly dereferenceable(4) %JDS, ptr noalias nocapture readonly dereferenceable(4) %JDE, ptr noalias nocapture readonly dereferenceable(4) %KDS, ptr noalias nocapture readonly dereferenceable(4) %KDE, ptr noalias nocapture readonly dereferenceable(4) %IMS, ptr noalias nocapture readonly dereferenceable(4) %IME, ptr noalias nocapture readonly dereferenceable(4) %JMS, ptr noalias nocapture readonly dereferenceable(4) %JME, ptr noalias nocapture readonly dereferenceable(4) %KMS, ptr noalias nocapture readonly dereferenceable(4) %KME, ptr noalias nocapture readonly dereferenceable(4) %ITS, ptr noalias nocapture readonly dereferenceable(4) %ITE, ptr noalias nocapture readonly dereferenceable(4) %JTS, ptr noalias nocapture readonly dereferenceable(4) %JTE, ptr noalias nocapture readonly dereferenceable(4) %KTS, ptr noalias nocapture readonly dereferenceable(4) %KTE, ptr noalias nocapture dereferenceable(4) %FQX, ptr noalias nocapture dereferenceable(4) %FQY, ptr noalias nocapture dereferenceable(4) %FQZ, ptr noalias nocapture readonly dereferenceable(4) %FQXL, ptr noalias nocapture readonly dereferenceable(4) %FQYL, ptr noalias nocapture readonly dereferenceable(4) %FQZL) local_unnamed_addr #1 {
alloca_1:
  %IMS_fetch.1 = load i32, ptr %IMS, align 1
  %IME_fetch.2 = load i32, ptr %IME, align 1
  %KMS_fetch.3 = load i32, ptr %KMS, align 1
  %KME_fetch.4 = load i32, ptr %KME, align 1
  %ITS_fetch.7 = load i32, ptr %ITS, align 1
  %ITE_fetch.8 = load i32, ptr %ITE, align 1
  %KTS_fetch.9 = load i32, ptr %KTS, align 1
  %KTE_fetch.10 = load i32, ptr %KTE, align 1
  %JTS_fetch.11 = load i32, ptr %JTS, align 1
  %reass.sub = sub i32 %IME_fetch.2, %IMS_fetch.1
  %add.4 = add i32 %reass.sub, 1
  %int_sext = sext i32 %add.4 to i64
  %mul.1 = shl nsw i64 %int_sext, 2
  %reass.sub801 = sub i32 %KME_fetch.4, %KMS_fetch.3
  %add.12 = add i32 %reass.sub801, 1
  %int_sext7 = sext i32 %add.12 to i64
  %mul.8 = mul nsw i64 %mul.1, %int_sext7
  %0 = sub i32 %ITE_fetch.8, %ITS_fetch.7
  %add.24 = add i32 %0, 4
  %int_sext13 = sext i32 %add.24 to i64
  %mul.20 = shl nsw i64 %int_sext13, 2
  %reass.sub802 = sub i32 %KTE_fetch.10, %KTS_fetch.9
  %add.25 = add i32 %reass.sub802, 1
  %int_sext14 = sext i32 %add.25 to i64
  %mul.21 = mul nsw i64 %mul.20, %int_sext14
  %sub.23 = add nsw i32 %JTS_fetch.11, -1
  %int_sext19 = sext i32 %KMS_fetch.3 to i64
  %JTE_fetch.19 = load i32, ptr %JTE, align 1
  %JDE_fetch.20 = load i32, ptr %JDE, align 1
  %sub.5 = add nsw i32 %JDE_fetch.20, -1
  %rel.3.not = icmp sgt i32 %JTE_fetch.19, %sub.5
  %slct.3 = select i1 %rel.3.not, i32 %sub.5, i32 %JTE_fetch.19
  %add.2 = add nsw i32 %slct.3, 1
  %rel.4 = icmp slt i32 %add.2, %sub.23
  br i1 %rel.4, label %do.end_do22, label %do.body21.preheader

do.body21.preheader:                              ; preds = %alloca_1
  %IDE_fetch.17 = load i32, ptr %IDE, align 1
  %sub.3 = add i32 %IDE_fetch.17, -1
  %rel.2.not = icmp sgt i32 %ITE_fetch.8, %sub.3
  %slct.2 = select i1 %rel.2.not, i32 %sub.3, i32 %ITE_fetch.8
  %add.1 = add nsw i32 %slct.2, 1
  %KDE_fetch.14 = load i32, ptr %KDE, align 1
  %sub.1 = add i32 %KDE_fetch.14, -1
  %rel.1.not = icmp sgt i32 %KTE_fetch.10, %sub.1
  %slct.1 = select i1 %rel.1.not, i32 %sub.1, i32 %KTE_fetch.10
  %sub.18 = add i32 %ITS_fetch.7, -1
  %JMS_fetch.5 = load i32, ptr %JMS, align 1
  %rel.5 = icmp slt i32 %slct.1, %KTS_fetch.9
  %rel.6 = icmp slt i32 %add.1, %sub.18
  %int_sext23 = sext i32 %IMS_fetch.1 to i64
  %int_sext25 = sext i32 %JMS_fetch.5 to i64
  %DT_fetch.80 = load float, ptr %DT, align 1
  %RDX_fetch.107 = load float, ptr %RDX, align 1
  %int_sext45 = sext i32 %sub.18 to i64
  %int_sext47 = sext i32 %KTS_fetch.9 to i64
  %int_sext49 = sext i32 %sub.23 to i64
  %RDY_fetch.140 = load float, ptr %RDY, align 1
  %1 = add nsw i32 %slct.2, 2
  %2 = add i32 %slct.1, 1
  %3 = sext i32 %JTS_fetch.11 to i64
  %4 = add nsw i64 %3, -1
  %5 = add nsw i32 %slct.3, 2
  %wide.trip.count = sext i32 %5 to i64
  br label %do.body21

do.body21:                                        ; preds = %do.body21.preheader, %do.end_do26
  %indvars.iv808 = phi i64 [ %4, %do.body21.preheader ], [ %indvars.iv.next809.pre-phi, %do.end_do26 ]
  br i1 %rel.5, label %do.body21.do.end_do26_crit_edge, label %do.body25.preheader

do.body21.do.end_do26_crit_edge:                  ; preds = %do.body21
  %.pre = add nsw i64 %indvars.iv808, 1
  br label %do.end_do26

do.body25.preheader:                              ; preds = %do.body21
  %"MUB[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext25, i64 %mul.1, ptr nonnull elementtype(float) %MUB, i64 %indvars.iv808)
  %"MU_OLD[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext25, i64 %mul.1, ptr nonnull elementtype(float) %MU_OLD, i64 %indvars.iv808)
  %"FIELD_OLD[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext25, i64 %mul.8, ptr nonnull elementtype(float) %FIELD_OLD, i64 %indvars.iv808)
  %"MSFTX[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext25, i64 %mul.1, ptr nonnull elementtype(float) %MSFTX, i64 %indvars.iv808)
  %"MSFTY[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext25, i64 %mul.1, ptr nonnull elementtype(float) %MSFTY, i64 %indvars.iv808)
  %"FQXL[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext49, i64 %mul.21, ptr nonnull elementtype(float) %FQXL, i64 %indvars.iv808)
  %6 = add nsw i64 %indvars.iv808, 1
  %"FQYL[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext49, i64 %mul.21, ptr nonnull elementtype(float) %FQYL, i64 %6)
  %"FQYL[]72" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext49, i64 %mul.21, ptr nonnull elementtype(float) %FQYL, i64 %indvars.iv808)
  %"FQZL[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext49, i64 %mul.21, ptr nonnull elementtype(float) %FQZL, i64 %indvars.iv808)
  %"FQX[]211" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext49, i64 %mul.21, ptr nonnull elementtype(float) %FQX, i64 %indvars.iv808)
  %"FQY[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext49, i64 %mul.21, ptr nonnull elementtype(float) %FQY, i64 %6)
  %"FQY[]144" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext49, i64 %mul.21, ptr nonnull elementtype(float) %FQY, i64 %indvars.iv808)
  %"FQZ[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 %int_sext49, i64 %mul.21, ptr nonnull elementtype(float) %FQZ, i64 %indvars.iv808)
  br label %do.body25

do.body25:                                        ; preds = %do.body25.preheader, %do.end_do30
  %indvars.iv803 = phi i64 [ %int_sext47, %do.body25.preheader ], [ %indvars.iv.next804.pre-phi, %do.end_do30 ]
  br i1 %rel.6, label %do.body25.do.end_do30_crit_edge, label %do.body29.preheader

do.body25.do.end_do30_crit_edge:                  ; preds = %do.body25
  %.pre812 = add nsw i64 %indvars.iv803, 1
  br label %do.end_do30

do.body29.preheader:                              ; preds = %do.body25
  %"FIELD_OLD[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext19, i64 %mul.1, ptr nonnull elementtype(float) %"FIELD_OLD[]", i64 %indvars.iv803)
  %"FQXL[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQXL[]", i64 %indvars.iv803)
  %"FQYL[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQYL[]", i64 %indvars.iv803)
  %"FQYL[][]73" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQYL[]72", i64 %indvars.iv803)
  %"RDZW[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext19, i64 4, ptr nonnull elementtype(float) %RDZW, i64 %indvars.iv803)
  %"RDZW[]_fetch.184" = load float, ptr %"RDZW[]", align 1
  %7 = add nsw i64 %indvars.iv803, 1
  %"FQZL[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQZL[]", i64 %7)
  %"FQZL[][]96" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQZL[]", i64 %indvars.iv803)
  %"FQX[][]212" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQX[]211", i64 %indvars.iv803)
  %"FQY[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQY[]", i64 %indvars.iv803)
  %"FQY[]144[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQY[]144", i64 %indvars.iv803)
  %"FQZ[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQZ[]", i64 %7)
  %"FQZ[]190[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 %int_sext47, i64 %mul.20, ptr nonnull elementtype(float) %"FQZ[]", i64 %indvars.iv803)
  br label %do.body29

do.body29:                                        ; preds = %do.body29.preheader, %bb15_endif
  %indvars.iv = phi i64 [ %int_sext45, %do.body29.preheader ], [ %indvars.iv.next, %bb15_endif ]
  %"MUB[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext23, i64 4, ptr nonnull elementtype(float) %"MUB[]", i64 %indvars.iv)
  %"MUB[][]_fetch.44" = load float, ptr %"MUB[][]", align 1
  %"MU_OLD[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext23, i64 4, ptr nonnull elementtype(float) %"MU_OLD[]", i64 %indvars.iv)
  %"MU_OLD[][]_fetch.57" = load float, ptr %"MU_OLD[][]", align 1
  %add.10 = fadd fast float %"MU_OLD[][]_fetch.57", %"MUB[][]_fetch.44"
  %"FIELD_OLD[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext23, i64 4, ptr nonnull elementtype(float) %"FIELD_OLD[][]", i64 %indvars.iv)
  %"FIELD_OLD[][][]_fetch.79" = load float, ptr %"FIELD_OLD[][][]", align 1
  %mul.12 = fmul fast float %add.10, %"FIELD_OLD[][][]_fetch.79"
  %"MSFTX[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext23, i64 4, ptr nonnull elementtype(float) %"MSFTX[]", i64 %indvars.iv)
  %"MSFTX[][]_fetch.93" = load float, ptr %"MSFTX[][]", align 1
  %"MSFTY[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext23, i64 4, ptr nonnull elementtype(float) %"MSFTY[]", i64 %indvars.iv)
  %"MSFTY[][]_fetch.106" = load float, ptr %"MSFTY[][]", align 1
  %mul.19 = fmul fast float %"MSFTY[][]_fetch.106", %"MSFTX[][]_fetch.93"
  %indvars.iv.next = add nsw i64 %indvars.iv, 1
  %"FQXL[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQXL[][]", i64 %indvars.iv.next)
  %"FQXL[][][]_fetch.129" = load float, ptr %"FQXL[][][]", align 1
  %"FQXL[][][]59" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQXL[][]", i64 %indvars.iv)
  %"FQXL[][][]_fetch.139" = load float, ptr %"FQXL[][][]59", align 1
  %sub.30 = fsub fast float %"FQXL[][][]_fetch.129", %"FQXL[][][]_fetch.139"
  %mul.25 = fmul fast float %sub.30, %RDX_fetch.107
  %"FQYL[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQYL[][]", i64 %indvars.iv)
  %"FQYL[][][]_fetch.162" = load float, ptr %"FQYL[][][]", align 1
  %"FQYL[][][]74" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQYL[][]73", i64 %indvars.iv)
  %"FQYL[][][]_fetch.172" = load float, ptr %"FQYL[][][]74", align 1
  %sub.43 = fsub fast float %"FQYL[][][]_fetch.162", %"FQYL[][][]_fetch.172"
  %mul.31 = fmul fast float %sub.43, %RDY_fetch.140
  %add.41 = fadd fast float %mul.31, %mul.25
  %mul.32 = fmul fast float %add.41, %mul.19
  %mul.34 = fmul fast float %"RDZW[]_fetch.184", %"MSFTY[][]_fetch.106"
  %"FQZL[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQZL[][]", i64 %indvars.iv)
  %"FQZL[][][]_fetch.206" = load float, ptr %"FQZL[][][]", align 1
  %"FQZL[][][]97" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQZL[][]96", i64 %indvars.iv)
  %"FQZL[][][]_fetch.216" = load float, ptr %"FQZL[][][]97", align 1
  %sub.56 = fsub fast float %"FQZL[][][]_fetch.206", %"FQZL[][][]_fetch.216"
  %mul.40 = fmul fast float %sub.56, %mul.34
  %add.51 = fadd fast float %mul.40, %mul.32
  %mul.41 = fmul fast float %add.51, %DT_fetch.80
  %sub.57 = fsub fast float %mul.12, %mul.41
  %"FQX[][][]213" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQX[][]212", i64 %indvars.iv.next)
  %"FQX[][][]_fetch.254" = load float, ptr %"FQX[][][]213", align 1
  %func_result = tail call fast float @llvm.maxnum.f32(float %"FQX[][][]_fetch.254", float 0.000000e+00)
  %"FQX[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQX[][]212", i64 %indvars.iv)
  %"FQX[][][]_fetch.264" = load float, ptr %"FQX[][][]", align 1
  %func_result114 = tail call fast float @llvm.minnum.f32(float %"FQX[][][]_fetch.264", float 0.000000e+00)
  %sub.70 = fsub fast float %func_result, %func_result114
  %mul.48 = fmul fast float %sub.70, %RDX_fetch.107
  %"FQY[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQY[][]", i64 %indvars.iv)
  %"FQY[][][]_fetch.287" = load float, ptr %"FQY[][][]", align 1
  %func_result131 = tail call fast float @llvm.maxnum.f32(float %"FQY[][][]_fetch.287", float 0.000000e+00)
  %"FQY[]144[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQY[]144[]", i64 %indvars.iv)
  %"FQY[]144[][]_fetch.297" = load float, ptr %"FQY[]144[][]", align 1
  %func_result146 = tail call fast float @llvm.minnum.f32(float %"FQY[]144[][]_fetch.297", float 0.000000e+00)
  %sub.83 = fsub fast float %func_result131, %func_result146
  %mul.54 = fmul fast float %sub.83, %RDY_fetch.140
  %add.70 = fadd fast float %mul.54, %mul.48
  %mul.55 = fmul fast float %add.70, %mul.19
  %"FQZ[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQZ[][]", i64 %indvars.iv)
  %"FQZ[][][]_fetch.330" = load float, ptr %"FQZ[][][]", align 1
  %func_result177 = tail call fast float @llvm.minnum.f32(float %"FQZ[][][]_fetch.330", float 0.000000e+00)
  %"FQZ[]190[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 %int_sext45, i64 4, ptr nonnull elementtype(float) %"FQZ[]190[]", i64 %indvars.iv)
  %"FQZ[]190[][]_fetch.340" = load float, ptr %"FQZ[]190[][]", align 1
  %func_result192 = tail call fast float @llvm.maxnum.f32(float %"FQZ[]190[][]_fetch.340", float 0.000000e+00)
  %sub.96 = fsub fast float %func_result177, %func_result192
  %mul.62 = fmul fast float %sub.96, %mul.34
  %add.80 = fadd fast float %mul.55, %mul.62
  %mul.63 = fmul fast float %add.80, %DT_fetch.80
  %rel.7 = fcmp fast ogt float %mul.63, %sub.57
  br i1 %rel.7, label %bb_new71_then, label %bb15_endif

bb_new75_then:                                    ; preds = %bb_new71_then
  %mul.64 = fmul fast float %func_result215, %"FQX[][][]_fetch.254"
  store float %mul.64, ptr %"FQX[][][]213", align 1
  br label %bb3_endif

bb3_endif:                                        ; preds = %bb_new71_then, %bb_new75_then
  %rel.9 = fcmp fast olt float %"FQX[][][]_fetch.264", 0.000000e+00
  br i1 %rel.9, label %bb_new77_then, label %bb5_endif

bb_new77_then:                                    ; preds = %bb3_endif
  %mul.65 = fmul fast float %"FQX[][][]_fetch.264", %func_result215
  store float %mul.65, ptr %"FQX[][][]", align 1
  br label %bb5_endif

bb5_endif:                                        ; preds = %bb3_endif, %bb_new77_then
  %rel.10 = fcmp fast ogt float %"FQY[][][]_fetch.287", 0.000000e+00
  br i1 %rel.10, label %bb_new79_then, label %bb7_endif

bb_new79_then:                                    ; preds = %bb5_endif
  %mul.66 = fmul fast float %func_result215, %"FQY[][][]_fetch.287"
  store float %mul.66, ptr %"FQY[][][]", align 1
  br label %bb7_endif

bb7_endif:                                        ; preds = %bb5_endif, %bb_new79_then
  %"FQY[][][]_fetch.444" = load float, ptr %"FQY[]144[][]", align 1
  %rel.11 = fcmp fast olt float %"FQY[][][]_fetch.444", 0.000000e+00
  br i1 %rel.11, label %bb_new81_then, label %bb9_endif

bb_new81_then:                                    ; preds = %bb7_endif
  %mul.67 = fmul fast float %"FQY[][][]_fetch.444", %func_result215
  store float %mul.67, ptr %"FQY[]144[][]", align 1
  br label %bb9_endif

bb9_endif:                                        ; preds = %bb7_endif, %bb_new81_then
  %rel.12 = fcmp fast olt float %"FQZ[][][]_fetch.330", 0.000000e+00
  br i1 %rel.12, label %bb_new83_then, label %bb11_endif

bb_new83_then:                                    ; preds = %bb9_endif
  %mul.68 = fmul fast float %func_result215, %"FQZ[][][]_fetch.330"
  store float %mul.68, ptr %"FQZ[][][]", align 1
  br label %bb11_endif

bb11_endif:                                       ; preds = %bb9_endif, %bb_new83_then
  %"FQZ[][][]_fetch.504" = load float, ptr %"FQZ[]190[][]", align 1
  %rel.13 = fcmp fast ogt float %"FQZ[][][]_fetch.504", 0.000000e+00
  br i1 %rel.13, label %bb_new85_then, label %bb15_endif

bb_new85_then:                                    ; preds = %bb11_endif
  %mul.69 = fmul fast float %"FQZ[][][]_fetch.504", %func_result215
  store float %mul.69, ptr %"FQZ[]190[][]", align 1
  br label %bb15_endif

bb_new71_then:                                    ; preds = %do.body29
  %add.81 = fadd fast float %mul.63, 0x3BC79CA100000000
  %div.1 = fdiv fast float %sub.57, %add.81
  %func_result215 = tail call fast float @llvm.maxnum.f32(float %div.1, float 0.000000e+00)
  %rel.8 = fcmp fast ogt float %"FQX[][][]_fetch.254", 0.000000e+00
  br i1 %rel.8, label %bb_new75_then, label %bb3_endif

bb15_endif:                                       ; preds = %do.body29, %bb_new85_then, %bb11_endif
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond = icmp eq i32 %1, %lftr.wideiv
  br i1 %exitcond, label %do.end_do30.loopexit, label %do.body29

do.end_do30.loopexit:                             ; preds = %bb15_endif
  br label %do.end_do30

do.end_do30:                                      ; preds = %do.end_do30.loopexit, %do.body25.do.end_do30_crit_edge
  %indvars.iv.next804.pre-phi = phi i64 [ %.pre812, %do.body25.do.end_do30_crit_edge ], [ %7, %do.end_do30.loopexit ]
  %lftr.wideiv806 = trunc i64 %indvars.iv.next804.pre-phi to i32
  %exitcond807 = icmp eq i32 %2, %lftr.wideiv806
  br i1 %exitcond807, label %do.end_do26.loopexit, label %do.body25

do.end_do26.loopexit:                             ; preds = %do.end_do30
  br label %do.end_do26

do.end_do26:                                      ; preds = %do.end_do26.loopexit, %do.body21.do.end_do26_crit_edge
  %indvars.iv.next809.pre-phi = phi i64 [ %.pre, %do.body21.do.end_do26_crit_edge ], [ %6, %do.end_do26.loopexit ]
  %exitcond811 = icmp eq i64 %indvars.iv.next809.pre-phi, %wide.trip.count
  br i1 %exitcond811, label %do.end_do22.loopexit, label %do.body21

do.end_do22.loopexit:                             ; preds = %do.end_do26
  br label %do.end_do22

do.end_do22:                                      ; preds = %do.end_do22.loopexit, %alloca_1
  ret void
}

; Function Attrs: mustprogress nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.maxnum.f32(float, float) #3

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare float @llvm.minnum.f32(float, float) #3
