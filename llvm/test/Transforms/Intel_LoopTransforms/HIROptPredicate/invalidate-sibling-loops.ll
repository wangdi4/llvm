; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-opt-predicate,hir-loop-distribute-memrec,print<hir-safe-reduction-analysis>,hir-opt-predicate,print<hir>' -hir-cost-model-throttling=0 -S < %s 2>&1 | FileCheck %s

; This test makes sure that hir-opt-predicate invalidates the analyses for the
; target loop properly. In the next example (HIR after running distribution,
; and instructions removed for simplification), the condition at <34> will be
; selected as candidate for hoisting. The target loop is <170> (first i1 loop).
; This means that the condition will be hoisted out to the region. The
; instructions in the first i2 loop (<169>) will be moved since it requires to
; generate the if/else cases. The problem is there is a safe reduction analysis
; in instruction <38>, which belongs to loop <169>, and this analysis will be
; become invalid. We need to make sure that the analysis is properly
; invalidated in order to modify loops <170>, <168> and <169>.

; <0>          BEGIN REGION { modified }
; <19>               if (umax(%i49, %i52) != 0)
; <19>               {
; <167>                 if (zext.i32.i64(%i42) >u 64)
; <167>                 {
; <170>                    + DO i1 = 0, (zext.i32.i64(%i42) + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <168>                    |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; <10>                     |   |   if (%i121 == 0)
; <10>                     |   |   {
; <10>                     |   |   }
; <10>                     |   |   else
; <10>                     |   |   {
; <26>                     |   |      if (%i119 + %i121 > (i32*)(@SOBUF)[0][8])
; <26>                     |   |      {
; <30>                     |   |         %i115 = %i115  +  1; <Safe Reduction>
; <26>                     |   |      }
; <34>                     |   |      if (trunc.i32.i1(%i70) == 0)
; <34>                     |   |      {
; <34>                     |   |      }
; <34>                     |   |      else
; <34>                     |   |      {
; <53>                     |   |         if (%i119 != 0)
; <53>                     |   |         {
; <53>                     |   |         }
; <34>                     |   |      }
; <94>                     |   |      if (trunc.i32.i1(%i47) != 0 & %i119 + %i121 != (%arg7)[i2 + sext.i32.i64(%i45) + %lb] & %i51 > -1)
; <94>                     |   |      {
; <94>                     |   |      }
; <10>                     |   |   }
; <168>                    |   + END LOOP
; <168>                    |   
; <169>                    |   
; <169>                    |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; <166>                    |   |   if (%i121 != 0)
; <166>                    |   |   {
; <165>                    |   |      if (trunc.i32.i1(%i70) == 0)
; <165>                    |   |      {
; <38>                     |   |         %i112 = %i112  +  1; <Safe Reduction>
; <165>                    |   |      }
; <165>                    |   |      else
; <165>                    |   |      {
; <65>                     |   |         %i113 = %i113  +  1; <Safe Reduction>
; <165>                    |   |      }
; <166>                    |   |   }
; <169>                    |   + END LOOP
; <170>                    + END LOOP
; <167>                 }
; <167>                 else
; <167>                 {
; <129>                    + DO i1 = 0, zext.i32.i64(%i42) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <134>                    |   if (%i121 == 0)
; <134>                    |   {
; <134>                    |   }
; <134>                    |   else
; <134>                    |   {
; <136>                    |      if (%i119 + %i121 > (i32*)(@SOBUF)[0][8])
; <136>                    |      {
; <137>                    |         %i115 = %i115  +  1; <Safe Reduction>
; <136>                    |      }
; <138>                    |      if (trunc.i32.i1(%i70) == 0)
; <138>                    |      {
; <139>                    |         %i112 = %i112  +  1; <Safe Reduction>
; <138>                    |      }
; <138>                    |      else
; <138>                    |      {
; <146>                    |         if (%i119 != 0)
; <146>                    |         {
; <146>                    |         }
; <150>                    |         %i113 = %i113  +  1; <Safe Reduction>
; <138>                    |      }
; <161>                    |      if (trunc.i32.i1(%i47) != 0 & %i119 + %i121 != (%arg7)[i1 + sext.i32.i64(%i45)] & %i51 > -1)
; <161>                    |      {
; <161>                    |      }
; <134>                    |   }
; <129>                    + END LOOP
; <167>                 }
; <19>               }
; <19>               else
; <19>               {
; <189>                 if (zext.i32.i64(%i42) >u 64)
; <189>                 {
; <192>                    + DO i1 = 0, (zext.i32.i64(%i42) + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <190>                    |   
; <190>                    |   + DO i2 = 0, %min15, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; <119>                    |   |   if (%i121 == 0)
; <119>                    |   |   {
; <119>                    |   |   }
; <119>                    |   |   else
; <119>                    |   |   {
; <125>                    |   |      if (trunc.i32.i1(%i47) != 0 & %i119 + %i121 != (%arg7)[64 * i1 + i2 + sext.i32.i64(%i45)] & %i51 > -1)
; <125>                    |   |      {
; <125>                    |   |      }
; <119>                    |   |   }
; <190>                    |   + END LOOP
; <190>                    |   
; <191>                    |   
; <191>                    |   + DO i2 = 0, %min15, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; <191>                    |   + END LOOP
; <192>                    + END LOOP
; <189>                 }
; <189>                 else
; <189>                 {
; <175>                    + DO i1 = 0, zext.i32.i64(%i42) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; <180>                    |   if (%i121 == 0)
; <180>                    |   {
; <180>                    |   }
; <180>                    |   else
; <180>                    |   {
; <185>                    |      if (trunc.i32.i1(%i47) != 0 & %i119 + %i121 != (%arg7)[i1 + sext.i32.i64(%i45)] & %i51 > -1)
; <185>                    |      {
; <185>                    |      }
; <180>                    |   }
; <175>                    + END LOOP
; <189>                 }
; <19>               }
; <0>          END REGION

; HIR after transformation (make sure the conditions were hoisted properly and
; the code won't seg-fault when printing HIR):

; CHECK: BEGIN REGION { modified }
; CHECK:       if (umax(%i49, %i52) != 0)
; CHECK:       {
; CHECK:          if (zext.i32.i64(%i42) >u 64)
; CHECK:          {
; CHECK:             if (trunc.i32.i1(%i70) == 0)
; CHECK:             {
; CHECK:                + DO i1 = 0, (zext.i32.i64(%i42) + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:                |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK:                |   + END LOOP
; CHECK:                |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK:                |   + END LOOP
; CHECK:             }
; CHECK:             else
; CHECK:             {
; CHECK:                + DO i1 = 0, (zext.i32.i64(%i42) + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:                |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK:                |   + END LOOP
; CHECK:                |   + DO i2 = 0, 64 * i1 + %min + -1 * %lb, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK:                |   + END LOOP
; CHECK:                + END LOOP
; CHECK:             }
; CHECK:          }
; CHECK:          else
; CHECK:          {
; CHECK:             if (trunc.i32.i1(%i70) == 0)
; CHECK:             {
; CHECK:                + DO i1 = 0, zext.i32.i64(%i42) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:                + END LOOP
; CHECK:             }
; CHECK:             else
; CHECK:             {
; CHECK:                + DO i1 = 0, zext.i32.i64(%i42) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:                + END LOOP
; CHECK:             }
; CHECK:          }
; CHECK:       }
; CHECK:       else
; CHECK:       {
; CHECK:          if (zext.i32.i64(%i42) >u 64)
; CHECK:          {
; CHECK:             + DO i1 = 0, (zext.i32.i64(%i42) + -1)/u64, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             |   + DO i2 = 0, %min15, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK:             |   + END LOOP
; CHECK:             |   + DO i2 = 0, %min15, 1   <DO_LOOP>  <MAX_TC_EST = 64>  <LEGAL_MAX_TC = 64>
; CHECK:             |   + END LOOP
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:          else
; CHECK:          {
; CHECK:             + DO i1 = 0, zext.i32.i64(%i42) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647>
; CHECK:             + END LOOP
; CHECK:          }
; CHECK:       }
; CHECK: END REGION

; ModuleID = 'simple.ll'
source_filename = "ld-temp.o"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc"

@SOBUF = external hidden unnamed_addr global [28 x i8], align 32
@__profc_EMPTY = external hidden global [9 x i64], section ".lprfc$M", align 8
@SOGUG = external hidden unnamed_addr global [32800 x i8], align 32
@anon.48348d8c863cf385585ee92ceca12a70.11 = external hidden unnamed_addr constant i32, align 4
@anon.48348d8c863cf385585ee92ceca12a70.13 = external hidden unnamed_addr constant i32, align 4

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
declare hidden void @DCOPY(ptr noalias nocapture readonly dereferenceable(4), ptr noalias nocapture readonly dereferenceable(8), ptr noalias nocapture readonly dereferenceable(4), ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture readonly dereferenceable(4)) #2

; Function Attrs: nounwind uwtable
declare hidden void @RAWRITE(ptr noalias nocapture readonly dereferenceable(4), ptr noalias nocapture dereferenceable(4), ptr noalias dereferenceable(8), ptr noalias nocapture readonly dereferenceable(4), ptr noalias nocapture readonly dereferenceable(4), ptr noalias nocapture readonly dereferenceable(4), ptr noalias nocapture readonly dereferenceable(4)) #3

; Function Attrs: nounwind uwtable
declare hidden void @RAREAD(ptr noalias nocapture readonly dereferenceable(4), ptr noalias nocapture readonly dereferenceable(4), ptr noalias dereferenceable(8), ptr noalias nocapture readonly dereferenceable(4), ptr noalias nocapture readonly dereferenceable(4), ptr noalias nocapture readonly dereferenceable(4)) #3

; Function Attrs: nounwind uwtable
define hidden void @EMPTY(ptr noalias nocapture dereferenceable(4) %arg, ptr noalias nocapture dereferenceable(4) %arg1, ptr noalias nocapture dereferenceable(4) %arg2, ptr noalias dereferenceable(8) %arg3, ptr noalias nocapture readonly dereferenceable(4) %arg4, ptr noalias nocapture readonly dereferenceable(4) %arg5, ptr noalias dereferenceable(8) %arg6, ptr noalias nocapture readonly dereferenceable(4) %arg7, ptr noalias nocapture readonly dereferenceable(4) %arg8, ptr noalias nocapture readonly dereferenceable(4) %arg9, ptr noalias nocapture dereferenceable(4) %arg10, ptr noalias nocapture readonly dereferenceable(4) %arg11) #3 {
bb:
  %i = load i64, ptr getelementptr inbounds ([9 x i64], ptr @__profc_EMPTY, i64 0, i64 8), align 8
  %i12 = add i64 %i, 1
  store i64 %i12, ptr getelementptr inbounds ([9 x i64], ptr @__profc_EMPTY, i64 0, i64 8), align 8
  %i13 = alloca [6 x i64], align 16
  %i14 = alloca i32, align 4
  %i15 = alloca i32, align 4
  %i16 = alloca i32, align 4
  %i17 = alloca [4 x i8], align 1
  %i18 = alloca <{ i64, ptr }>, align 8
  %i19 = alloca [4 x i8], align 1
  %i20 = alloca <{ i64 }>, align 8
  %i21 = alloca [4 x i8], align 1
  %i22 = alloca <{ i64 }>, align 8
  %i23 = alloca i32, align 4
  %i24 = alloca i32, align 4
  %i25 = alloca i32, align 4
  %i26 = alloca i32, align 4
  %i27 = alloca i32, align 4
  %i28 = alloca i32, align 4
  %i39 = load i32, ptr @SOBUF, align 32
  %i40 = sext i32 %i39 to i64
  %i41 = shl nsw i64 %i40, 3
  %i42 = load i32, ptr %arg4
  %i43 = icmp slt i32 %i42, 1
  br i1 %i43, label %bb104, label %bb44

bb44:                                             ; preds = %bb
  %i45 = load i32, ptr %arg5
  %i46 = load i32, ptr %arg10
  %i47 = load i32, ptr %arg8, align 1
  %i48 = and i32 %i47, 1
  %i49 = icmp eq i32 %i48, 0
  %i50 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %arg7, i64 1)
  %i51 = load i32, ptr %i50, align 1
  %i52 = icmp sgt i32 %i51, -1
  %i53 = or i1 %i49, %i52
  %i70 = load i32, ptr %arg9, align 1
  %i71 = and i32 %i70, 1
  %i72 = icmp eq i32 %i71, 0
  %i73 = load i32, ptr %arg11, align 1
  %i74 = icmp ne i32 %i48, 0
  %i101 = sext i32 %i45 to i64
  %i102 = zext i32 %i42 to i64
  %i103 = add nuw nsw i64 %i102, 1
  br label %bb107

bb104:                                            ; preds = %bb
  %i105 = load i64, ptr getelementptr inbounds ([9 x i64], ptr @__profc_EMPTY, i64 0, i64 1), align 8
  %i106 = add i64 %i105, 1
  store i64 %i106, ptr getelementptr inbounds ([9 x i64], ptr @__profc_EMPTY, i64 0, i64 1), align 8
  br label %bb221

bb107:                                            ; preds = %bb187, %bb44
  %i108 = phi i64 [ 1, %bb44 ], [ %i197, %bb187 ]
  %i109 = phi i32 [ %i46, %bb44 ], [ %i196, %bb187 ]
  %i110 = phi i64 [ 0, %bb44 ], [ %i189, %bb187 ]
  %i111 = phi i64 [ 0, %bb44 ], [ %i190, %bb187 ]
  %i112 = phi i64 [ 0, %bb44 ], [ %i191, %bb187 ]
  %i113 = phi i64 [ 0, %bb44 ], [ %i192, %bb187 ]
  %i114 = phi i64 [ 0, %bb44 ], [ %i193, %bb187 ]
  %i115 = phi i64 [ 0, %bb44 ], [ %i194, %bb187 ]
  %i116 = phi i64 [ 0, %bb44 ], [ %i195, %bb187 ]
  %i117 = add nsw i64 %i108, %i101
  %i118 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %arg1, i64 %i117)
  %i119 = load i32, ptr %i118
  store i32 %i119, ptr %i16, align 4
  %i120 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %arg2, i64 %i108)
  %i121 = load i32, ptr %i120
  store i32 %i121, ptr %i15, align 4
  %i122 = icmp eq i32 %i121, 0
  br i1 %i122, label %bb123, label %bb183

bb123:                                            ; preds = %bb107
  %i124 = add i64 %i116, 1
  br label %bb187

bb125:                                            ; preds = %bb159
  %i126 = add i64 %i115, 1
  br label %bb132

bb132:                                            ; preds = %bb159, %bb125
  %i133 = phi i64 [ %i115, %bb159 ], [ %i126, %bb125 ]
  %i134 = trunc i64 %i117 to i32
  br i1 %i72, label %bb152, label %bb150

bb135:                                            ; preds = %bb150
  %i136 = add i64 %i114, 1
  %i137 = load i32, ptr getelementptr inbounds ([32800 x i8], ptr @SOGUG, i64 0, i64 32784), align 16
  %i138 = add nsw i32 %i137, %i134
  store i32 %i138, ptr %i23, align 4
  call void @RAREAD(ptr nonnull getelementptr inbounds ([32800 x i8], ptr @SOGUG, i64 0, i64 32776), ptr nonnull %arg, ptr nonnull %arg6, ptr nonnull %i16, ptr nonnull %i23, ptr nonnull @anon.48348d8c863cf385585ee92ceca12a70.11) #5
  br label %bb139

bb139:                                            ; preds = %bb150, %bb135
  %i140 = phi i32 [ %i119, %bb135 ], [ 0, %bb150 ]
  %i141 = phi i64 [ %i136, %bb135 ], [ %i114, %bb150 ]
  %i142 = add i64 %i113, 1
  %i143 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i41, ptr nonnull elementtype(double) %arg3, i64 %i108)
  %i144 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i143, i64 1)
  %i145 = add nsw i32 %i140, 1
  %i146 = sext i32 %i145 to i64
  %i147 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %arg6, i64 %i146)
  call void @DCOPY(ptr nonnull %i15, ptr nonnull %i144, ptr nonnull @anon.48348d8c863cf385585ee92ceca12a70.13, ptr nonnull %i147, ptr nonnull @anon.48348d8c863cf385585ee92ceca12a70.13) #5
  store i32 0, ptr %i14, align 4
  %i148 = load i32, ptr getelementptr inbounds ([32800 x i8], ptr @SOGUG, i64 0, i64 32784), align 16
  %i149 = add nsw i32 %i148, %i134
  store i32 %i160, ptr %i24, align 4
  store i32 %i160, ptr %i25, align 4
  store i32 %i149, ptr %i26, align 4
  call void @RAWRITE(ptr nonnull getelementptr inbounds ([32800 x i8], ptr @SOGUG, i64 0, i64 32776), ptr nonnull %arg, ptr nonnull %arg6, ptr nonnull %i24, ptr nonnull %i25, ptr nonnull %i26, ptr nonnull %i14) #5
  br label %bb163

bb150:                                            ; preds = %bb132
  %i151 = icmp eq i32 %i119, 0
  br i1 %i151, label %bb139, label %bb135

bb152:                                            ; preds = %bb132
  %i153 = add i64 %i112, 1
  %i154 = sdiv i32 %i119, %i73
  store i32 %i154, ptr %i14, align 4
  %i155 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %i41, ptr nonnull elementtype(double) %arg3, i64 %i108)
  %i156 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %i155, i64 1)
  %i157 = load i32, ptr getelementptr inbounds ([32800 x i8], ptr @SOGUG, i64 0, i64 32784), align 16
  %i158 = add nsw i32 %i157, %i134
  store i32 %i160, ptr %i27, align 4
  store i32 %i158, ptr %i28, align 4
  call void @RAWRITE(ptr nonnull getelementptr inbounds ([32800 x i8], ptr @SOGUG, i64 0, i64 32776), ptr nonnull %arg, ptr nonnull %i156, ptr nonnull %i27, ptr nonnull %i15, ptr nonnull %i28, ptr nonnull %i14) #5
  br label %bb163

bb159:                                            ; preds = %bb183
  %i160 = add nsw i32 %i121, %i119
  %i161 = load i32, ptr getelementptr inbounds ([28 x i8], ptr @SOBUF, i64 0, i64 8), align 8
  %i162 = icmp sgt i32 %i160, %i161
  br i1 %i162, label %bb125, label %bb132

bb163:                                            ; preds = %bb185, %bb152, %bb139
  %i164 = phi i32 [ %i186, %bb185 ], [ %i160, %bb152 ], [ %i160, %bb139 ]
  %i165 = phi i64 [ %i112, %bb185 ], [ %i112, %bb139 ], [ %i153, %bb152 ]
  %i166 = phi i64 [ %i113, %bb185 ], [ %i142, %bb139 ], [ %i113, %bb152 ]
  %i167 = phi i64 [ %i114, %bb185 ], [ %i141, %bb139 ], [ %i114, %bb152 ]
  %i168 = phi i64 [ %i115, %bb185 ], [ %i133, %bb139 ], [ %i133, %bb152 ]
  store i32 %i164, ptr %i118
  store i32 0, ptr %i120
  %i169 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) %arg7, i64 %i117)
  %i170 = load i32, ptr %i169
  %i171 = icmp ne i32 %i164, %i170
  %i172 = and i1 %i74, %i171
  %i173 = and i1 %i172, %i52
  br i1 %i173, label %bb174, label %bb187

bb174:                                            ; preds = %bb163
  %i175 = add i64 %i111, 1
  br label %bb187

bb183:                                            ; preds = %bb107
  %i184 = add i64 %i110, 1
  br i1 %i53, label %bb159, label %bb185

bb185:                                            ; preds = %bb183
  %i186 = add nsw i32 %i121, %i119
  br label %bb163

bb187:                                            ; preds = %bb174, %bb163, %bb123
  %i188 = phi i32 [ %i119, %bb123 ], [ %i164, %bb174 ], [ %i164, %bb163 ]
  %i189 = phi i64 [ %i110, %bb123 ], [ %i184, %bb174 ], [ %i184, %bb163 ]
  %i190 = phi i64 [ %i111, %bb123 ], [ %i175, %bb174 ], [ %i111, %bb163 ]
  %i191 = phi i64 [ %i112, %bb123 ], [ %i165, %bb174 ], [ %i165, %bb163 ]
  %i192 = phi i64 [ %i113, %bb123 ], [ %i166, %bb174 ], [ %i166, %bb163 ]
  %i193 = phi i64 [ %i114, %bb123 ], [ %i167, %bb174 ], [ %i167, %bb163 ]
  %i194 = phi i64 [ %i115, %bb123 ], [ %i168, %bb174 ], [ %i168, %bb163 ]
  %i195 = phi i64 [ %i124, %bb123 ], [ %i116, %bb174 ], [ %i116, %bb163 ]
  %i196 = add nsw i32 %i188, %i109
  store i32 %i196, ptr %arg10
  %i197 = add nuw nsw i64 %i108, 1
  %i198 = icmp eq i64 %i197, %i103
  br i1 %i198, label %bb199, label %bb107

bb199:                                            ; preds = %bb187
  %i202 = phi i64 [ %i191, %bb187 ]
  %i203 = phi i64 [ %i192, %bb187 ]
  %i205 = phi i64 [ %i194, %bb187 ]
  br label %bb221

bb221:                                            ; preds = %bb199, %bb104
  ret void
}