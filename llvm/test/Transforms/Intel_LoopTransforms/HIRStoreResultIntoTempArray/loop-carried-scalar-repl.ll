; There are two insts including pow() functions, and they have the same express trees. We extract the code and restore
; the result into a temp array.
;
; In this case, the MinRef is (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][0] and the MaxRef is (%"jacobian_$Q")[i1 + 2][%mod][%mod26][0].
; We enlarge the array size of i1, i2 and i3, because the offset i1 + 1, i2 + 1 and i3 + 1 in MinRef.
; We enlarge the upper bound of i1 in the extracted loop, because the distance between (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][0] and (%"jacobian_$Q")[i1 + 2][%mod][%mod26][0] is 1 at this dimension.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-store-result-into-temp-array,print<hir>" -hir-create-function-level-region -disable-output 2>&1 < %s | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-store-result-into-temp-array" -hir-create-function-level-region -print-changed -disable-output 2>&1 < %s | FileCheck %s --check-prefix=CHECK-CHANGED
;
;
;*** IR Dump Before HIR Store Result Into Temp Array ***
;Function: jacobian_
;
;<0>          BEGIN REGION { }
;<13>                  %"jacobian_$M.0" = undef;
;<138:36>           + DO i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + -1, 1
;<139:35>           |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1
;<32:17>            |   |   %mod = i2 + 3  %  %"jacobian_$NY_fetch";
;<140:34>           |   |
;<140:34>           |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1
;<44:19>            |   |   |   %mod26 = i3 + 2  %  %"jacobian_$NX_fetch";
;<50:20>            |   |   |   %"jacobian_$Q[]52[][][]_fetch" = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][0];
;<51:20>            |   |   |   %div = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][1]  /  %"jacobian_$Q[]52[][][]_fetch";
;<55:21>            |   |   |   %div86 = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][2]  /  %"jacobian_$Q[]52[][][]_fetch";
;<57:22>            |   |   |   %mul98 = %div  *  %div;
;<58:22>            |   |   |   %mul100 = %div86  *  %div86;
;<59:22>            |   |   |   %add101 = %mul98  +  %mul100;
;<60:22>            |   |   |   %mul102 = %add101  *  2.000000e+00;
;<62:23>            |   |   |   %mul106 = %mul102  *  2.000000e+00;
;<63:23>            |   |   |   %div105 = %mul106  /  %"jacobian_$Q[]52[][][]_fetch";
;<65:24>            |   |   |   %func_result = @llvm.pow.f64(%div105,  2.500000e+00);
;<72:27>            |   |   |   %"jacobian_$Q[]144[][][]_fetch" = (%"jacobian_$Q")[i1 + 2][%mod][%mod26][0];
;<73:27>            |   |   |   %div152 = (%"jacobian_$Q")[i1 + 2][%mod][%mod26][1]  /  %"jacobian_$Q[]144[][][]_fetch";
;<77:28>            |   |   |   %div202 = (%"jacobian_$Q")[i1 + 2][%mod][%mod26][2]  /  %"jacobian_$Q[]144[][][]_fetch";
;<79:29>            |   |   |   %mul214 = %div152  *  %div152;
;<80:29>            |   |   |   %mul216 = %div202  *  %div202;
;<81:29>            |   |   |   %add217 = %mul214  +  %mul216;
;<82:29>            |   |   |   %mul218 = %add217  *  2.000000e+00;
;<84:30>            |   |   |   %mul230 = %mul218  *  2.000000e+00;
;<85:30>            |   |   |   %div229 = %mul230  /  %"jacobian_$Q[]144[][][]_fetch";
;<87:31>            |   |   |   %func_result242 = @llvm.pow.f64(%div229,  2.500000e+00);
;<89:33>            |   |   |   %add243 = %"jacobian_$M.0"  +  %func_result242;
;<90:33>            |   |   |   %"jacobian_$M.0" = %func_result  +  %add243;
;<140:34>           |   |   + END LOOP
;<139:35>           |   + END LOOP
;<138:36>           + END LOOP
;<123:38>              (%addressof)[0][0] = 48;
;<125:38>              (%addressof)[0][1] = 1;
;<127:38>              (%addressof)[0][2] = 1;
;<129:38>              (%addressof)[0][3] = 0;
;<131:38>              (%ARGBLOCK_0)[0].0 = %"jacobian_$M.0";
;<134:38>              %func_result280 = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((i8*)(%ARGBLOCK_0)[0]));
;<138:36>
;<137:39>           ret ;
;<0>          END REGION
;
;*** IR Dump After HIR Store Result Into Temp Array ***
;Function: jacobian_
;
; CHECK: BEGIN REGION { modified }
; CHECK:       %call = @llvm.stacksave.p0();
; CHECK:       %array_size = sext.i32.i64(%"jacobian_$NY_fetch") + 1  *  sext.i32.i64(%"jacobian_$NX_fetch") + 1;
; CHECK:       %array_size4 = sext.i32.i64(%"jacobian_$NZ_fetch1") + 1  *  %array_size;
; CHECK:       %TempArray = alloca %array_size4;
;
; CHECK:       + DO i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1"), 1
; CHECK:       |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch"), 1
; CHECK:       |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch"), 1
;              |   |   |   %"jacobian_$Q[]52[][][]_fetch" = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][0];
;              |   |   |   %div = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][1]  /  %"jacobian_$Q[]52[][][]_fetch";
;              |   |   |   %div86 = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3 + 1][2]  /  %"jacobian_$Q[]52[][][]_fetch";
;              |   |   |   %mul98 = %div  *  %div;
;              |   |   |   %mul100 = %div86  *  %div86;
;              |   |   |   %add101 = %mul98  +  %mul100;
;              |   |   |   %mul102 = %add101  *  2.000000e+00;
;              |   |   |   %mul106 = %mul102  *  2.000000e+00;
;              |   |   |   %div105 = %mul106  /  %"jacobian_$Q[]52[][][]_fetch";
; CHECK:       |   |   |   (%TempArray)[i1][i2][i3] = @llvm.pow.f64(%div105,  2.500000e+00);
;              |   |   + END LOOP
;              |   + END LOOP
;              + END LOOP
;
;
;                 %"jacobian_$M.0" = undef;
; CHECK:       + DO i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + -1, 1
; CHECK:       |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1
;              |   |   %mod = i2 + 3  %  %"jacobian_$NY_fetch";
;              |   |
; CHECK:       |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1
;              |   |   |   %mod26 = i3 + 2  %  %"jacobian_$NX_fetch";
; CHECK:       |   |   |   %func_result = (%TempArray)[i1][i2][i3];
; CHECK:       |   |   |   %func_result242 = (%TempArray)[i1 + 1][zext.i32.i64(%mod) + -1][zext.i32.i64(%mod26) + -1];
;              |   |   |   %add243 = %"jacobian_$M.0"  +  %func_result242;
;              |   |   |   %"jacobian_$M.0" = %func_result  +  %add243;
;              |   |   + END LOOP
;              |   + END LOOP
;              + END LOOP
;                 (i8*)(%addressof)[0] = 48;
;                 (%addressof)[0][1] = 1;
;                 (%addressof)[0][2] = 1;
;                 (%addressof)[0][3] = 0;
;                 (double*)(%ARGBLOCK_0)[0] = %"jacobian_$M.0";
;                 %func_result280 = @for_write_seq_lis(&((%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0]),  &((%ARGBLOCK_0)[0]));
;
; CHECK:       @llvm.stackrestore.p0(&((%call)[0]));
;              ret ;
;        END REGION



; Verify that pass is dumped with print-changed when it triggers.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRStoreResultIntoTempArray

;Module Before HIR
; ModuleID = 'j.f90'
source_filename = "j.f90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @jacobian_(ptr noalias nocapture readonly %"jacobian_$Q", ptr noalias nocapture readonly %"jacobian_$NX", ptr noalias nocapture readonly %"jacobian_$NY", ptr noalias nocapture readonly %"jacobian_$NZ") local_unnamed_addr #0 !dbg !5 {
alloca_0:
  %"var$1" = alloca [8 x i64], align 16
  %addressof = alloca [4 x i8], align 1, !dbg !8
  %ARGBLOCK_0 = alloca { double }, align 8, !dbg !8
  call void @llvm.dbg.declare(metadata ptr %"jacobian_$Q", metadata !9, metadata !DIExpression()), !dbg !8
  call void @llvm.dbg.declare(metadata ptr %"jacobian_$NX", metadata !11, metadata !DIExpression()), !dbg !13
  call void @llvm.dbg.declare(metadata ptr %"jacobian_$NY", metadata !14, metadata !DIExpression()), !dbg !15
  call void @llvm.dbg.declare(metadata ptr %"jacobian_$NZ", metadata !16, metadata !DIExpression()), !dbg !17
  %"jacobian_$NX_fetch" = load i32, ptr %"jacobian_$NX", align 1, !dbg !18
  %"jacobian_$NY_fetch" = load i32, ptr %"jacobian_$NY", align 1, !dbg !18
  %int_sext = sext i32 %"jacobian_$NX_fetch" to i64, !dbg !19
  %mul = mul nsw i64 %int_sext, 40, !dbg !19
  %int_sext27 = sext i32 %"jacobian_$NY_fetch" to i64, !dbg !19
  %mul28 = mul nsw i64 %mul, %int_sext27, !dbg !19
  %"jacobian_$NZ_fetch1" = load i32, ptr %"jacobian_$NZ", align 1, !dbg !20
  %rel = icmp slt i32 %"jacobian_$NZ_fetch1", 2, !dbg !21
  %rel4 = icmp slt i32 %"jacobian_$NY_fetch", 2, !dbg !22
  %or.cond = or i1 %rel4, %rel, !dbg !21
  %rel8 = icmp slt i32 %"jacobian_$NX_fetch", 2, !dbg !23
  %or.cond348 = or i1 %rel8, %or.cond, !dbg !21
  br i1 %or.cond348, label %bb1, label %bb21.preheader, !dbg !21

bb21.preheader:                                   ; preds = %alloca_0
  %0 = add nuw nsw i32 %"jacobian_$NX_fetch", 1, !dbg !24
  %1 = add nuw nsw i32 %"jacobian_$NY_fetch", 1, !dbg !24
  %2 = add nuw nsw i32 %"jacobian_$NZ_fetch1", 1, !dbg !24
  %wide.trip.count357 = sext i32 %2 to i64, !dbg !25
  %wide.trip.count352 = sext i32 %1 to i64, !dbg !26
  %wide.trip.count = sext i32 %0 to i64, !dbg !27
  br label %bb21, !dbg !24

bb21:                                             ; preds = %bb21.preheader, %bb26
  %indvars.iv354 = phi i64 [ 1, %bb21.preheader ], [ %indvars.iv.next355, %bb26 ]
  %"jacobian_$M.0" = phi double [ undef, %bb21.preheader ], [ %add244.lcssa.lcssa, %bb26 ]
  call void @llvm.dbg.value(metadata double %"jacobian_$M.0", metadata !28, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i64 %indvars.iv354, metadata !31, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i32 1, metadata !32, metadata !DIExpression()), !dbg !30
  %indvars.iv.next355 = add nuw nsw i64 %indvars.iv354, 1, !dbg !25
  %"jacobian_$Q[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul28, ptr elementtype(double) %"jacobian_$Q", i64 %indvars.iv.next355), !dbg !19
  %3 = add nuw nsw i64 %indvars.iv354, 2, !dbg !33
  %"jacobian_$Q[]119" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul28, ptr elementtype(double) %"jacobian_$Q", i64 %3), !dbg !33
  br label %bb30.preheader, !dbg !34

bb30.preheader:                                   ; preds = %bb31, %bb21
  %indvars.iv349 = phi i64 [ 1, %bb21 ], [ %indvars.iv.next350, %bb31 ]
  %"jacobian_$M.1" = phi double [ %"jacobian_$M.0", %bb21 ], [ %add244.lcssa, %bb31 ]
  call void @llvm.dbg.value(metadata double %"jacobian_$M.1", metadata !28, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i64 %indvars.iv349, metadata !32, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i32 undef, metadata !35, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i32 1, metadata !36, metadata !DIExpression()), !dbg !30
  %4 = trunc i64 %indvars.iv349 to i32, !dbg !37
  %5 = add i32 %4, 2, !dbg !37
  %mod = srem i32 %5, %"jacobian_$NY_fetch", !dbg !37
  call void @llvm.dbg.value(metadata i32 %mod, metadata !35, metadata !DIExpression()), !dbg !30
  %"jacobian_$Q[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) %"jacobian_$Q[]", i64 %indvars.iv349), !dbg !19
  %int_sext157 = zext i32 %mod to i64, !dbg !33
  %"jacobian_$Q[]119[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) %"jacobian_$Q[]119", i64 %int_sext157), !dbg !33
  br label %bb30, !dbg !27

bb30:                                             ; preds = %bb30.preheader, %bb30
  %indvars.iv = phi i64 [ 1, %bb30.preheader ], [ %indvars.iv.next, %bb30 ]
  %"jacobian_$M.2" = phi double [ %"jacobian_$M.1", %bb30.preheader ], [ %add244, %bb30 ]
  call void @llvm.dbg.value(metadata double %"jacobian_$M.2", metadata !28, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !36, metadata !DIExpression()), !dbg !30
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !38
  %6 = trunc i64 %indvars.iv.next to i32, !dbg !39
  %mod26 = srem i32 %6, %"jacobian_$NX_fetch", !dbg !39
  call void @llvm.dbg.value(metadata i32 %mod26, metadata !40, metadata !DIExpression()), !dbg !30
  %"jacobian_$Q[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) %"jacobian_$Q[][]", i64 %indvars.iv), !dbg !19
  %"jacobian_$Q[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[][][]", i64 2), !dbg !19
  %"jacobian_$Q[][][][]_fetch" = load double, ptr %"jacobian_$Q[][][][]", align 1, !dbg !19
  %"jacobian_$Q[]52[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[][][]", i64 1), !dbg !19
  %"jacobian_$Q[]52[][][]_fetch" = load double, ptr %"jacobian_$Q[]52[][][]", align 1, !dbg !19
  %div = fdiv double %"jacobian_$Q[][][][]_fetch", %"jacobian_$Q[]52[][][]_fetch", !dbg !41
  call void @llvm.dbg.value(metadata double %div, metadata !42, metadata !DIExpression()), !dbg !30
  %"jacobian_$Q[]63[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[][][]", i64 3), !dbg !43
  %"jacobian_$Q[]63[][][]_fetch" = load double, ptr %"jacobian_$Q[]63[][][]", align 1, !dbg !43
  %div86 = fdiv double %"jacobian_$Q[]63[][][]_fetch", %"jacobian_$Q[]52[][][]_fetch", !dbg !44
  call void @llvm.dbg.value(metadata double %div86, metadata !45, metadata !DIExpression()), !dbg !30
  %mul98 = fmul double %div, %div, !dbg !46
  %mul100 = fmul double %div86, %div86, !dbg !47
  %add101 = fadd double %mul98, %mul100, !dbg !48
  %mul102 = fmul double %add101, 2.000000e+00, !dbg !49
  call void @llvm.dbg.value(metadata double %mul102, metadata !50, metadata !DIExpression()), !dbg !30
  %mul106 = fmul double %mul102, 2.000000e+00, !dbg !51
  %div105 = fdiv double %mul106, %"jacobian_$Q[]52[][][]_fetch", !dbg !52
  call void @llvm.dbg.value(metadata double %div105, metadata !53, metadata !DIExpression()), !dbg !30
  %func_result = tail call double @llvm.pow.f64(double %div105, double 2.500000e+00), !dbg !54
  call void @llvm.dbg.value(metadata double %func_result, metadata !55, metadata !DIExpression()), !dbg !30
  %int_sext154 = zext i32 %mod26 to i64, !dbg !33
  %"jacobian_$Q[]119[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) %"jacobian_$Q[]119[]", i64 %int_sext154), !dbg !33
  %"jacobian_$Q[]119[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]119[][]", i64 2), !dbg !33
  %"jacobian_$Q[]119[][][]_fetch" = load double, ptr %"jacobian_$Q[]119[][][]", align 1, !dbg !33
  %"jacobian_$Q[]144[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]119[][]", i64 1), !dbg !33
  %"jacobian_$Q[]144[][][]_fetch" = load double, ptr %"jacobian_$Q[]144[][][]", align 1, !dbg !33
  %div152 = fdiv double %"jacobian_$Q[]119[][][]_fetch", %"jacobian_$Q[]144[][][]_fetch", !dbg !56
  call void @llvm.dbg.value(metadata double %div152, metadata !57, metadata !DIExpression()), !dbg !30
  %"jacobian_$Q[]165[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]119[][]", i64 3), !dbg !58
  %"jacobian_$Q[]165[][][]_fetch" = load double, ptr %"jacobian_$Q[]165[][][]", align 1, !dbg !58
  %div202 = fdiv double %"jacobian_$Q[]165[][][]_fetch", %"jacobian_$Q[]144[][][]_fetch", !dbg !59
  call void @llvm.dbg.value(metadata double %div202, metadata !60, metadata !DIExpression()), !dbg !30
  %mul214 = fmul double %div152, %div152, !dbg !61
  %mul216 = fmul double %div202, %div202, !dbg !62
  %add217 = fadd double %mul214, %mul216, !dbg !63
  %mul218 = fmul double %add217, 2.000000e+00, !dbg !64
  call void @llvm.dbg.value(metadata double %mul218, metadata !65, metadata !DIExpression()), !dbg !30
  %mul230 = fmul double %mul218, 2.000000e+00, !dbg !66
  %div229 = fdiv double %mul230, %"jacobian_$Q[]144[][][]_fetch", !dbg !67
  call void @llvm.dbg.value(metadata double %div229, metadata !68, metadata !DIExpression()), !dbg !30
  %func_result242 = tail call double @llvm.pow.f64(double %div229, double 2.500000e+00), !dbg !69
  call void @llvm.dbg.value(metadata double %func_result242, metadata !70, metadata !DIExpression()), !dbg !30
  %add243 = fadd double %"jacobian_$M.2", %func_result242, !dbg !71
  %add244 = fadd double %func_result, %add243, !dbg !72
  call void @llvm.dbg.value(metadata double %add244, metadata !28, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !36, metadata !DIExpression()), !dbg !30
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count, !dbg !27
  br i1 %exitcond, label %bb31, label %bb30, !dbg !27

bb31:                                             ; preds = %bb30
  %add244.lcssa = phi double [ %add244, %bb30 ], !dbg !72
  call void @llvm.dbg.value(metadata double %add244.lcssa, metadata !28, metadata !DIExpression()), !dbg !30
  %indvars.iv.next350 = add nuw nsw i64 %indvars.iv349, 1, !dbg !26
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next350, metadata !32, metadata !DIExpression()), !dbg !30
  %exitcond353 = icmp eq i64 %indvars.iv.next350, %wide.trip.count352, !dbg !26
  br i1 %exitcond353, label %bb26, label %bb30.preheader, !dbg !26

bb26:                                             ; preds = %bb31
  %add244.lcssa.lcssa = phi double [ %add244.lcssa, %bb31 ], !dbg !72
  call void @llvm.dbg.value(metadata double %add244.lcssa.lcssa, metadata !28, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata double %add244.lcssa.lcssa, metadata !28, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata double %add244.lcssa.lcssa, metadata !28, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next355, metadata !31, metadata !DIExpression()), !dbg !30
  %exitcond358 = icmp eq i64 %indvars.iv.next355, %wide.trip.count357, !dbg !25
  br i1 %exitcond358, label %bb133, label %bb21, !dbg !25

bb133:                                            ; preds = %bb26
  %add244.lcssa.lcssa.lcssa = phi double [ %add244.lcssa.lcssa, %bb26 ], !dbg !72
  call void @llvm.dbg.value(metadata double %add244.lcssa.lcssa.lcssa, metadata !28, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata double %add244.lcssa.lcssa.lcssa, metadata !28, metadata !DIExpression()), !dbg !30
  call void @llvm.dbg.value(metadata double %add244.lcssa.lcssa.lcssa, metadata !28, metadata !DIExpression()), !dbg !30
  store i8 48, ptr %addressof, align 1, !dbg !73
  %.fca.1.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 1, !dbg !73
  store i8 1, ptr %.fca.1.gep, align 1, !dbg !73
  %.fca.2.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 2, !dbg !73
  store i8 1, ptr %.fca.2.gep, align 1, !dbg !73
  %.fca.3.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 3, !dbg !73
  store i8 0, ptr %.fca.3.gep, align 1, !dbg !73
  store double %add244.lcssa.lcssa.lcssa, ptr %ARGBLOCK_0, align 8, !dbg !73
  %func_result280 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %"var$1", i32 -1, i64 1239157112576, ptr nonnull %addressof, ptr nonnull %ARGBLOCK_0), !dbg !73
  br label %bb1, !dbg !74

bb1:                                              ; preds = %alloca_0, %bb133
  ret void, !dbg !74
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

; Function Attrs: nounwind readnone speculatable willreturn
declare double @llvm.pow.f64(double, double) #1

declare i32 @for_write_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { "intel-lang"="fortran" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind readnone speculatable willreturn }
attributes #2 = { nounwind readnone speculatable }

!llvm.module.flags = !{!0, !1}
!llvm.dbg.cu = !{!2}
!omp_offload.info = !{}

!0 = !{i32 2, !"Debug Info Version", i32 3}
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !3, producer: "Intel(R) Fortran 21.0-1699a", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "j.f90", directory: "/export/iusers/linayu/xmain_llvm/llvm/llvm/test/Transforms/Intel_LoopTransforms/HIRStoreResultIntoTempArray")
!4 = !{}
!5 = distinct !DISubprogram(name: "jacobian", linkageName: "jacobian_", scope: !3, file: !3, line: 1, type: !6, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!6 = !DISubroutineType(types: !7)
!7 = !{null}
!8 = !DILocation(line: 1, column: 29, scope: !5)
!9 = !DILocalVariable(name: "q", arg: 1, scope: !5, file: !3, line: 1, type: !10)
!10 = !DIBasicType(name: "FIXME", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "nx", arg: 2, scope: !5, file: !3, line: 1, type: !12)
!12 = !DIBasicType(name: "INTEGER*4", size: 32, encoding: DW_ATE_signed)
!13 = !DILocation(line: 1, column: 32, scope: !5)
!14 = !DILocalVariable(name: "ny", arg: 3, scope: !5, file: !3, line: 1, type: !12)
!15 = !DILocation(line: 1, column: 36, scope: !5)
!16 = !DILocalVariable(name: "nz", arg: 4, scope: !5, file: !3, line: 1, type: !12)
!17 = !DILocation(line: 1, column: 40, scope: !5)
!18 = !DILocation(line: 1, column: 20, scope: !5)
!19 = !DILocation(line: 20, column: 16, scope: !5)
!20 = !DILocation(line: 11, column: 9, scope: !5)
!21 = !DILocation(line: 11, column: 16, scope: !5)
!22 = !DILocation(line: 12, column: 16, scope: !5)
!23 = !DILocation(line: 13, column: 16, scope: !5)
!24 = !DILocation(line: 16, column: 10, scope: !5)
!25 = !DILocation(line: 36, column: 9, scope: !5)
!26 = !DILocation(line: 35, column: 11, scope: !5)
!27 = !DILocation(line: 34, column: 13, scope: !5)
!28 = !DILocalVariable(name: "m", scope: !5, file: !3, line: 9, type: !29)
!29 = !DIBasicType(name: "REAL*8", size: 64, encoding: DW_ATE_float)
!30 = !DILocation(line: 0, scope: !5)
!31 = !DILocalVariable(name: "k", scope: !5, file: !3, line: 7, type: !12)
!32 = !DILocalVariable(name: "j", scope: !5, file: !3, line: 7, type: !12)
!33 = !DILocation(line: 27, column: 16, scope: !5)
!34 = !DILocation(line: 18, column: 13, scope: !5)
!35 = !DILocalVariable(name: "n", scope: !5, file: !3, line: 7, type: !12)
!36 = !DILocalVariable(name: "i", scope: !5, file: !3, line: 7, type: !12)
!37 = !DILocation(line: 17, column: 15, scope: !5)
!38 = !DILocation(line: 19, column: 23, scope: !5)
!39 = !DILocation(line: 19, column: 18, scope: !5)
!40 = !DILocalVariable(name: "l", scope: !5, file: !3, line: 7, type: !12)
!41 = !DILocation(line: 20, column: 35, scope: !5)
!42 = !DILocalVariable(name: "a", scope: !5, file: !3, line: 9, type: !29)
!43 = !DILocation(line: 21, column: 16, scope: !5)
!44 = !DILocation(line: 21, column: 35, scope: !5)
!45 = !DILocalVariable(name: "b", scope: !5, file: !3, line: 9, type: !29)
!46 = !DILocation(line: 22, column: 23, scope: !5)
!47 = !DILocation(line: 22, column: 31, scope: !5)
!48 = !DILocation(line: 22, column: 27, scope: !5)
!49 = !DILocation(line: 22, column: 36, scope: !5)
!50 = !DILocalVariable(name: "e", scope: !5, file: !3, line: 9, type: !29)
!51 = !DILocation(line: 23, column: 22, scope: !5)
!52 = !DILocation(line: 23, column: 28, scope: !5)
!53 = !DILocalVariable(name: "f", scope: !5, file: !3, line: 9, type: !29)
!54 = !DILocation(line: 24, column: 22, scope: !5)
!55 = !DILocalVariable(name: "g", scope: !5, file: !3, line: 9, type: !29)
!56 = !DILocation(line: 27, column: 36, scope: !5)
!57 = !DILocalVariable(name: "ab", scope: !5, file: !3, line: 9, type: !29)
!58 = !DILocation(line: 28, column: 16, scope: !5)
!59 = !DILocation(line: 28, column: 36, scope: !5)
!60 = !DILocalVariable(name: "bb", scope: !5, file: !3, line: 9, type: !29)
!61 = !DILocation(line: 29, column: 25, scope: !5)
!62 = !DILocation(line: 29, column: 35, scope: !5)
!63 = !DILocation(line: 29, column: 30, scope: !5)
!64 = !DILocation(line: 29, column: 41, scope: !5)
!65 = !DILocalVariable(name: "eb", scope: !5, file: !3, line: 9, type: !29)
!66 = !DILocation(line: 30, column: 24, scope: !5)
!67 = !DILocation(line: 30, column: 30, scope: !5)
!68 = !DILocalVariable(name: "fb", scope: !5, file: !3, line: 9, type: !29)
!69 = !DILocation(line: 31, column: 24, scope: !5)
!70 = !DILocalVariable(name: "gb", scope: !5, file: !3, line: 9, type: !29)
!71 = !DILocation(line: 33, column: 22, scope: !5)
!72 = !DILocation(line: 33, column: 27, scope: !5)
!73 = !DILocation(line: 38, column: 9, scope: !5)
!74 = !DILocation(line: 39, column: 9, scope: !5)
