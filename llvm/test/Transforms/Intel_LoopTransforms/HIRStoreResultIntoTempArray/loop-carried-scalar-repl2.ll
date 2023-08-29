; There are two insts including pow() functions, and they have the same express trees. We extract the code and restore
; the result into a temp array.
;
; In this case, MinRef is (%"jacobian_$Q")[i1 + 1][i2 + 1][i3][0], and MaxRef is (%"jacobian_$Q")[i1 + 2][%mod][%mod27][0].
; We set TempArray's dimenion by [i1][i2][i3], the use of it will be shifted accordingly to reduce the array size.
;
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-store-result-into-temp-array,print<hir>" -hir-details-dims -disable-output 2>&1 < %s | FileCheck %s
;
;*** IR Dump Before HIR Store Result Into Temp Array ***
;Function: jacobian_
;
;<0>          BEGIN REGION { }
;<124:36>           + DO i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + -1, 1
;<125:35>           |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1
;<18:17>            |   |   %mod = i2 + 3  %  %"jacobian_$NY_fetch" + 1;
;<126:34>           |   |
;<126:34>           |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1
;<30:19>            |   |   |   %mod27 = i3 + 2  %  %"jacobian_$NX_fetch";
;<36:20>            |   |   |   %"jacobian_$Q[]53[][][]_fetch" = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3][0];
;<37:20>            |   |   |   %div = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3][1]  /  %"jacobian_$Q[]53[][][]_fetch";
;<41:21>            |   |   |   %div87 = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3][2]  /  %"jacobian_$Q[]53[][][]_fetch";
;<43:22>            |   |   |   %mul99 = %div  *  %div;
;<44:22>            |   |   |   %mul101 = %div87  *  %div87;
;<45:22>            |   |   |   %add102 = %mul99  +  %mul101;
;<46:22>            |   |   |   %mul103 = %add102  *  2.000000e+00;
;<48:23>            |   |   |   %mul107 = %mul103  *  2.000000e+00;
;<49:23>            |   |   |   %div106 = %mul107  /  %"jacobian_$Q[]53[][][]_fetch";
;<51:24>            |   |   |   %func_result = @llvm.pow.f64(%div106,  2.500000e+00);
;<59:27>            |   |   |   %"jacobian_$Q[]147[][][]_fetch" = (%"jacobian_$Q")[i1 + 2][%mod][zext.i32.i64(%mod27)][0];
;<60:27>            |   |   |   %div155 = (%"jacobian_$Q")[i1 + 2][%mod][zext.i32.i64(%mod27)][1]  /  %"jacobian_$Q[]147[][][]_fetch";
;<64:28>            |   |   |   %div208 = (%"jacobian_$Q")[i1 + 2][%mod][zext.i32.i64(%mod27)][2]  /  %"jacobian_$Q[]147[][][]_fetch";
;<66:29>            |   |   |   %mul221 = %div155  *  %div155;
;<67:29>            |   |   |   %mul223 = %div208  *  %div208;
;<68:29>            |   |   |   %add224 = %mul221  +  %mul223;
;<69:29>            |   |   |   %mul225 = %add224  *  2.000000e+00;
;<71:30>            |   |   |   %mul237 = %mul225  *  2.000000e+00;
;<72:30>            |   |   |   %div236 = %mul237  /  %"jacobian_$Q[]147[][][]_fetch";
;<74:31>            |   |   |   %func_result250 = @llvm.pow.f64(%div236,  2.500000e+00);
;<76:33>            |   |   |   %add251 = %"jacobian_$M.0"  +  %func_result250;
;<77:33>            |   |   |   %"jacobian_$M.0" = %func_result  +  %add251;
;<126:34>           |   |   + END LOOP
;<125:35>           |   + END LOOP
;<124:36>           + END LOOP
;<0>          END REGION
;
;<105>        BEGIN REGION { }
;<111:38>           (%addressof)[0][0] = 48;
;<113:38>           (%addressof)[0][1] = 1;
;<115:38>           (%addressof)[0][2] = 1;
;<117:38>           (%addressof)[0][3] = 0;
;<119:38>           (%ARGBLOCK_0)[0].0 = %add252.lcssa.lcssa.lcssa;
;<122:38>           %func_result288 = @for_write_seq_lis(&((i8*)(%"var$1")[0]),  -1,  1239157112576,  &((%addressof)[0][0]),  &((i8*)(%ARGBLOCK_0)[0]));
;<105>        END REGION
;
;*** IR Dump After HIR Store Result Into Temp Array ***
;Function: jacobian_

; CHECK:      BEGIN REGION { modified }
; CHECK:            %call = @llvm.stacksave.p0();
; CHECK:            %array_size = sext.i32.i64(%"jacobian_$NY_fetch") + 2  *  sext.i32.i64(%"jacobian_$NX_fetch");
; CHECK:            %array_size3 = sext.i32.i64(%"jacobian_$NZ_fetch1") + 1  *  %array_size;
; CHECK:            %TempArray = alloca %array_size3;
; CHECK:            + DO i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1"), 1
; CHECK:            |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + 1, 1
; CHECK:            |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1
;                   |   |   |   %"jacobian_$Q[]53[][][]_fetch" = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3][0];
;                   |   |   |   %div = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3][1]  /  %"jacobian_$Q[]53[][][]_fetch";
;                   |   |   |   %div87 = (%"jacobian_$Q")[i1 + 1][i2 + 1][i3][2]  /  %"jacobian_$Q[]53[][][]_fetch";
;                   |   |   |   %mul99 = %div  *  %div;
;                   |   |   |   %mul101 = %div87  *  %div87;
;                   |   |   |   %add102 = %mul99  +  %mul101;
;                   |   |   |   %mul103 = %add102  *  2.000000e+00;
;                   |   |   |   %mul107 = %mul103  *  2.000000e+00;
;                   |   |   |   %div106 = %mul107  /  %"jacobian_$Q[]53[][][]_fetch";
; CHECK:            |   |   |   (%TempArray)[0:i1:8 * (sext.i32.i64(%"jacobian_$NX_fetch") * (2 + sext.i32.i64(%"jacobian_$NY_fetch")))(double:0)][0:i2:8 * sext.i32.i64(%"jacobian_$NX_fetch")(double:0)][0:i3:8(double:0)] = @llvm.pow.f64(%div106,  2.500000e+00);
;                   |   |   + END LOOP
;                   |   + END LOOP
;                   + END LOOP
; CHECK:            + DO i1 = 0, sext.i32.i64(%"jacobian_$NZ_fetch1") + -1, 1
; CHECK:            |   + DO i2 = 0, sext.i32.i64(%"jacobian_$NY_fetch") + -1, 1
;                   |   |   %mod = i2 + 3  %  %"jacobian_$NY_fetch" + 1;
;                   |   |
; CHECK:            |   |   + DO i3 = 0, sext.i32.i64(%"jacobian_$NX_fetch") + -1, 1
;                   |   |   |   %mod27 = i3 + 2  %  %"jacobian_$NX_fetch";
; CHECK:            |   |   |   %func_result = (%TempArray)[0:i1:8 * (sext.i32.i64(%"jacobian_$NX_fetch") * (2 + sext.i32.i64(%"jacobian_$NY_fetch")))(double:0)][0:i2:8 * sext.i32.i64(%"jacobian_$NX_fetch")(double:0)][0:i3:8(double:0)];
; CHECK:            |   |   |   %func_result250 = (%TempArray)[0:i1 + 1:8 * (sext.i32.i64(%"jacobian_$NX_fetch") * (2 + sext.i32.i64(%"jacobian_$NY_fetch")))(double:0)][0:zext.i32.i64(%mod) + -1:8 * sext.i32.i64(%"jacobian_$NX_fetch")(double:0)][0:zext.i32.i64(%mod27):8(double:0)];
;                   |   |   |   %add251 = %"jacobian_$M.0"  +  %func_result250;
;                   |   |   |   %"jacobian_$M.0" = %func_result  +  %add251;
;                   |   |   + END LOOP
;                   |   + END LOOP
;                   + END LOOP
; CHECK:            @llvm.stackrestore.p0(&((%call)
;             END REGION

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
  %int_sext28 = sext i32 %"jacobian_$NY_fetch" to i64, !dbg !19
  %mul29 = mul nsw i64 %mul, %int_sext28, !dbg !19
  %"jacobian_$NZ_fetch1" = load i32, ptr %"jacobian_$NZ", align 1, !dbg !20
  %rel = icmp slt i32 %"jacobian_$NZ_fetch1", 2, !dbg !21
  %rel4 = icmp slt i32 %"jacobian_$NY_fetch", 2, !dbg !22
  %or.cond = or i1 %rel4, %rel, !dbg !21
  %rel8 = icmp slt i32 %"jacobian_$NX_fetch", 2, !dbg !23
  %or.cond356 = or i1 %rel8, %or.cond, !dbg !21
  br i1 %or.cond356, label %bb1, label %bb21.preheader, !dbg !21

bb21.preheader:                                   ; preds = %alloca_0
  %add20 = add nuw nsw i32 %"jacobian_$NY_fetch", 1, !dbg !24
  %0 = add nuw nsw i32 %"jacobian_$NX_fetch", 1, !dbg !25
  %1 = add nuw nsw i32 %"jacobian_$NZ_fetch1", 1, !dbg !25
  %wide.trip.count365 = sext i32 %1 to i64, !dbg !26
  %wide.trip.count360 = sext i32 %add20 to i64, !dbg !27
  %wide.trip.count = sext i32 %0 to i64, !dbg !28
  br label %bb21, !dbg !25

bb21:                                             ; preds = %bb21.preheader, %bb26
  %indvars.iv362 = phi i64 [ 1, %bb21.preheader ], [ %indvars.iv.next363, %bb26 ]
  %"jacobian_$M.0" = phi double [ undef, %bb21.preheader ], [ %add252.lcssa.lcssa, %bb26 ]
  call void @llvm.dbg.value(metadata double %"jacobian_$M.0", metadata !29, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata i64 %indvars.iv362, metadata !32, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata i32 1, metadata !33, metadata !DIExpression()), !dbg !31
  %indvars.iv.next363 = add nuw nsw i64 %indvars.iv362, 1, !dbg !26
  %"jacobian_$Q[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul29, ptr elementtype(double) %"jacobian_$Q", i64 %indvars.iv.next363), !dbg !19
  %2 = add nuw nsw i64 %indvars.iv362, 2, !dbg !34
  %"jacobian_$Q[]120" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 3, i64 1, i64 %mul29, ptr elementtype(double) %"jacobian_$Q", i64 %2), !dbg !34
  br label %bb30.preheader, !dbg !35

bb30.preheader:                                   ; preds = %bb31, %bb21
  %indvars.iv357 = phi i64 [ 1, %bb21 ], [ %indvars.iv.next358, %bb31 ]
  %"jacobian_$M.1" = phi double [ %"jacobian_$M.0", %bb21 ], [ %add252.lcssa, %bb31 ]
  call void @llvm.dbg.value(metadata double %"jacobian_$M.1", metadata !29, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata i64 %indvars.iv357, metadata !33, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata i32 undef, metadata !36, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata i32 1, metadata !37, metadata !DIExpression()), !dbg !31
  %3 = trunc i64 %indvars.iv357 to i32, !dbg !38
  %4 = add i32 %3, 2, !dbg !38
  %mod = srem i32 %4, %add20, !dbg !38
  call void @llvm.dbg.value(metadata i32 %mod, metadata !36, metadata !DIExpression()), !dbg !31
  %"jacobian_$Q[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) %"jacobian_$Q[]", i64 %indvars.iv357), !dbg !19
  %int_sext161 = zext i32 %mod to i64, !dbg !34
  %"jacobian_$Q[]120[]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %mul, ptr elementtype(double) %"jacobian_$Q[]120", i64 %int_sext161), !dbg !34
  br label %bb30, !dbg !28

bb30:                                             ; preds = %bb30.preheader, %bb30
  %indvars.iv = phi i64 [ 1, %bb30.preheader ], [ %indvars.iv.next, %bb30 ]
  %"jacobian_$M.2" = phi double [ %"jacobian_$M.1", %bb30.preheader ], [ %add252, %bb30 ]
  call void @llvm.dbg.value(metadata double %"jacobian_$M.2", metadata !29, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata i64 %indvars.iv, metadata !37, metadata !DIExpression()), !dbg !31
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !39
  %5 = trunc i64 %indvars.iv.next to i32, !dbg !40
  %mod27 = srem i32 %5, %"jacobian_$NX_fetch", !dbg !40
  call void @llvm.dbg.value(metadata i32 %mod27, metadata !41, metadata !DIExpression()), !dbg !31
  %"jacobian_$Q[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) %"jacobian_$Q[][]", i64 %indvars.iv), !dbg !19
  %"jacobian_$Q[][][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[][][]", i64 2), !dbg !19
  %"jacobian_$Q[][][][]_fetch" = load double, ptr %"jacobian_$Q[][][][]", align 1, !dbg !19
  %"jacobian_$Q[]53[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[][][]", i64 1), !dbg !19
  %"jacobian_$Q[]53[][][]_fetch" = load double, ptr %"jacobian_$Q[]53[][][]", align 1, !dbg !19
  %div = fdiv double %"jacobian_$Q[][][][]_fetch", %"jacobian_$Q[]53[][][]_fetch", !dbg !42
  call void @llvm.dbg.value(metadata double %div, metadata !43, metadata !DIExpression()), !dbg !31
  %"jacobian_$Q[]64[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[][][]", i64 3), !dbg !44
  %"jacobian_$Q[]64[][][]_fetch" = load double, ptr %"jacobian_$Q[]64[][][]", align 1, !dbg !44
  %div87 = fdiv double %"jacobian_$Q[]64[][][]_fetch", %"jacobian_$Q[]53[][][]_fetch", !dbg !45
  call void @llvm.dbg.value(metadata double %div87, metadata !46, metadata !DIExpression()), !dbg !31
  %mul99 = fmul double %div, %div, !dbg !47
  %mul101 = fmul double %div87, %div87, !dbg !48
  %add102 = fadd double %mul99, %mul101, !dbg !49
  %mul103 = fmul double %add102, 2.000000e+00, !dbg !50
  call void @llvm.dbg.value(metadata double %mul103, metadata !51, metadata !DIExpression()), !dbg !31
  %mul107 = fmul double %mul103, 2.000000e+00, !dbg !52
  %div106 = fdiv double %mul107, %"jacobian_$Q[]53[][][]_fetch", !dbg !53
  call void @llvm.dbg.value(metadata double %div106, metadata !54, metadata !DIExpression()), !dbg !31
  %func_result = tail call double @llvm.pow.f64(double %div106, double 2.500000e+00), !dbg !55
  call void @llvm.dbg.value(metadata double %func_result, metadata !56, metadata !DIExpression()), !dbg !31
  %add157 = add nuw nsw i32 %mod27, 1, !dbg !34
  %int_sext158 = zext i32 %add157 to i64, !dbg !34
  %"jacobian_$Q[]120[][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 40, ptr elementtype(double) %"jacobian_$Q[]120[]", i64 %int_sext158), !dbg !34
  %"jacobian_$Q[]120[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]120[][]", i64 2), !dbg !34
  %"jacobian_$Q[]120[][][]_fetch" = load double, ptr %"jacobian_$Q[]120[][][]", align 1, !dbg !34
  %"jacobian_$Q[]147[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]120[][]", i64 1), !dbg !34
  %"jacobian_$Q[]147[][][]_fetch" = load double, ptr %"jacobian_$Q[]147[][][]", align 1, !dbg !34
  %div155 = fdiv double %"jacobian_$Q[]120[][][]_fetch", %"jacobian_$Q[]147[][][]_fetch", !dbg !57
  call void @llvm.dbg.value(metadata double %div155, metadata !58, metadata !DIExpression()), !dbg !31
  %"jacobian_$Q[]169[][][]" = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %"jacobian_$Q[]120[][]", i64 3), !dbg !59
  %"jacobian_$Q[]169[][][]_fetch" = load double, ptr %"jacobian_$Q[]169[][][]", align 1, !dbg !59
  %div208 = fdiv double %"jacobian_$Q[]169[][][]_fetch", %"jacobian_$Q[]147[][][]_fetch", !dbg !60
  call void @llvm.dbg.value(metadata double %div208, metadata !61, metadata !DIExpression()), !dbg !31
  %mul221 = fmul double %div155, %div155, !dbg !62
  %mul223 = fmul double %div208, %div208, !dbg !63
  %add224 = fadd double %mul221, %mul223, !dbg !64
  %mul225 = fmul double %add224, 2.000000e+00, !dbg !65
  call void @llvm.dbg.value(metadata double %mul225, metadata !66, metadata !DIExpression()), !dbg !31
  %mul237 = fmul double %mul225, 2.000000e+00, !dbg !67
  %div236 = fdiv double %mul237, %"jacobian_$Q[]147[][][]_fetch", !dbg !68
  call void @llvm.dbg.value(metadata double %div236, metadata !69, metadata !DIExpression()), !dbg !31
  %func_result250 = tail call double @llvm.pow.f64(double %div236, double 2.500000e+00), !dbg !70
  call void @llvm.dbg.value(metadata double %func_result250, metadata !71, metadata !DIExpression()), !dbg !31
  %add251 = fadd double %"jacobian_$M.2", %func_result250, !dbg !72
  %add252 = fadd double %func_result, %add251, !dbg !73
  call void @llvm.dbg.value(metadata double %add252, metadata !29, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next, metadata !37, metadata !DIExpression()), !dbg !31
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count, !dbg !28
  br i1 %exitcond, label %bb31, label %bb30, !dbg !28

bb31:                                             ; preds = %bb30
  %add252.lcssa = phi double [ %add252, %bb30 ], !dbg !73
  call void @llvm.dbg.value(metadata double %add252.lcssa, metadata !29, metadata !DIExpression()), !dbg !31
  %indvars.iv.next358 = add nuw nsw i64 %indvars.iv357, 1, !dbg !27
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next358, metadata !33, metadata !DIExpression()), !dbg !31
  %exitcond361 = icmp eq i64 %indvars.iv.next358, %wide.trip.count360, !dbg !27
  br i1 %exitcond361, label %bb26, label %bb30.preheader, !dbg !27

bb26:                                             ; preds = %bb31
  %add252.lcssa.lcssa = phi double [ %add252.lcssa, %bb31 ], !dbg !73
  call void @llvm.dbg.value(metadata double %add252.lcssa.lcssa, metadata !29, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata double %add252.lcssa.lcssa, metadata !29, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata double %add252.lcssa.lcssa, metadata !29, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata i64 %indvars.iv.next363, metadata !32, metadata !DIExpression()), !dbg !31
  %exitcond366 = icmp eq i64 %indvars.iv.next363, %wide.trip.count365, !dbg !26
  br i1 %exitcond366, label %bb133, label %bb21, !dbg !26

bb133:                                            ; preds = %bb26
  %add252.lcssa.lcssa.lcssa = phi double [ %add252.lcssa.lcssa, %bb26 ], !dbg !73
  call void @llvm.dbg.value(metadata double %add252.lcssa.lcssa.lcssa, metadata !29, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata double %add252.lcssa.lcssa.lcssa, metadata !29, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata double %add252.lcssa.lcssa.lcssa, metadata !29, metadata !DIExpression()), !dbg !31
  store i8 48, ptr %addressof, align 1, !dbg !74
  %.fca.1.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 1, !dbg !74
  store i8 1, ptr %.fca.1.gep, align 1, !dbg !74
  %.fca.2.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 2, !dbg !74
  store i8 1, ptr %.fca.2.gep, align 1, !dbg !74
  %.fca.3.gep = getelementptr inbounds [4 x i8], ptr %addressof, i64 0, i64 3, !dbg !74
  store i8 0, ptr %.fca.3.gep, align 1, !dbg !74
  store double %add252.lcssa.lcssa.lcssa, ptr %ARGBLOCK_0, align 8, !dbg !74
  %func_result288 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_write_seq_lis(ptr nonnull %"var$1", i32 -1, i64 1239157112576, ptr nonnull %addressof, ptr nonnull %ARGBLOCK_0), !dbg !74
  br label %bb1, !dbg !75

bb1:                                              ; preds = %alloca_0, %bb133
  ret void, !dbg !75
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
!24 = !DILocation(line: 17, column: 25, scope: !5)
!25 = !DILocation(line: 16, column: 10, scope: !5)
!26 = !DILocation(line: 36, column: 9, scope: !5)
!27 = !DILocation(line: 35, column: 11, scope: !5)
!28 = !DILocation(line: 34, column: 13, scope: !5)
!29 = !DILocalVariable(name: "m", scope: !5, file: !3, line: 9, type: !30)
!30 = !DIBasicType(name: "REAL*8", size: 64, encoding: DW_ATE_float)
!31 = !DILocation(line: 0, scope: !5)
!32 = !DILocalVariable(name: "k", scope: !5, file: !3, line: 7, type: !12)
!33 = !DILocalVariable(name: "j", scope: !5, file: !3, line: 7, type: !12)
!34 = !DILocation(line: 27, column: 16, scope: !5)
!35 = !DILocation(line: 18, column: 13, scope: !5)
!36 = !DILocalVariable(name: "n", scope: !5, file: !3, line: 7, type: !12)
!37 = !DILocalVariable(name: "i", scope: !5, file: !3, line: 7, type: !12)
!38 = !DILocation(line: 17, column: 15, scope: !5)
!39 = !DILocation(line: 19, column: 23, scope: !5)
!40 = !DILocation(line: 19, column: 18, scope: !5)
!41 = !DILocalVariable(name: "l", scope: !5, file: !3, line: 7, type: !12)
!42 = !DILocation(line: 20, column: 35, scope: !5)
!43 = !DILocalVariable(name: "a", scope: !5, file: !3, line: 9, type: !30)
!44 = !DILocation(line: 21, column: 16, scope: !5)
!45 = !DILocation(line: 21, column: 35, scope: !5)
!46 = !DILocalVariable(name: "b", scope: !5, file: !3, line: 9, type: !30)
!47 = !DILocation(line: 22, column: 23, scope: !5)
!48 = !DILocation(line: 22, column: 31, scope: !5)
!49 = !DILocation(line: 22, column: 27, scope: !5)
!50 = !DILocation(line: 22, column: 36, scope: !5)
!51 = !DILocalVariable(name: "e", scope: !5, file: !3, line: 9, type: !30)
!52 = !DILocation(line: 23, column: 22, scope: !5)
!53 = !DILocation(line: 23, column: 28, scope: !5)
!54 = !DILocalVariable(name: "f", scope: !5, file: !3, line: 9, type: !30)
!55 = !DILocation(line: 24, column: 22, scope: !5)
!56 = !DILocalVariable(name: "g", scope: !5, file: !3, line: 9, type: !30)
!57 = !DILocation(line: 27, column: 38, scope: !5)
!58 = !DILocalVariable(name: "ab", scope: !5, file: !3, line: 9, type: !30)
!59 = !DILocation(line: 28, column: 16, scope: !5)
!60 = !DILocation(line: 28, column: 38, scope: !5)
!61 = !DILocalVariable(name: "bb", scope: !5, file: !3, line: 9, type: !30)
!62 = !DILocation(line: 29, column: 25, scope: !5)
!63 = !DILocation(line: 29, column: 35, scope: !5)
!64 = !DILocation(line: 29, column: 30, scope: !5)
!65 = !DILocation(line: 29, column: 41, scope: !5)
!66 = !DILocalVariable(name: "eb", scope: !5, file: !3, line: 9, type: !30)
!67 = !DILocation(line: 30, column: 24, scope: !5)
!68 = !DILocation(line: 30, column: 30, scope: !5)
!69 = !DILocalVariable(name: "fb", scope: !5, file: !3, line: 9, type: !30)
!70 = !DILocation(line: 31, column: 24, scope: !5)
!71 = !DILocalVariable(name: "gb", scope: !5, file: !3, line: 9, type: !30)
!72 = !DILocation(line: 33, column: 22, scope: !5)
!73 = !DILocation(line: 33, column: 27, scope: !5)
!74 = !DILocation(line: 38, column: 9, scope: !5)
!75 = !DILocation(line: 39, column: 9, scope: !5)
