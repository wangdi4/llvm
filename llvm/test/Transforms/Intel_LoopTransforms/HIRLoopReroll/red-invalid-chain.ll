; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

; No reroll happens.
; SR Chain - A set of instructions in a SR chain shouldn't co-exist
; in one initiation interval(II).
; Our SR Chain reroll assumes that all SR inst in one II span are in
; different SR chains. Thus only the first II is reused after the rewriting
; and all other spans are removed.
; e.g.
;  II 1: Inst 1 - Chain 1
;        Inst 1 - Chain 2
;  II 2: Inst 2 - Chain 1
;        Inst 2 - Chain 2
; to -->
;  II 1: Inst 1 - Chain 1
;        Inst 1 - Chain 2
; Thus, following pattern is not handled.
;  II 1: Inst 1 - Chain 1 (1)
;        Inst 2 - Chain 1 (2)
;  II 2: Inst 1 - Chain 2 (3)
;        Inst 2 - Chain 2 (4)
;
; TODO: Reroll can still
;       go ahead by skipping updating Chain of  SRs.
;       However, more checks may be needed to do so.

;         BEGIN REGION { }
;               + DO i1 = 0, (%len + -9)/u8, 1   <DO_LOOP>  <MAX_TC_EST = 268435455>
;               |   %1 = (<16 x i8>*)(%src1)[8 * i1];
;               |   %3 = (<16 x i8>*)(%src2)[8 * i1];
;               |   %5 = (<16 x i8>*)(%src1)[8 * i1 + 4];
;               |   %7 = (<16 x i8>*)(%src2)[8 * i1 + 4];
;               |   %psrldq.i.i57 = shufflevector %3,  poison,  <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>;
;               |   %8 = bitcast.<16 x i8>.<4 x i32>(%psrldq.i.i57);
;               |   %shuffle.i.i.i58 = shufflevector %8,  undef,  <i32 0, i32 1>;
;               |   %conv.i.i.i59 = sitofp.<2 x i32>.<2 x double>(%shuffle.i.i.i58);
;               |   %psrldq.i20.i60 = shufflevector %1,  poison,  <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>;
;               |   %9 = bitcast.<16 x i8>.<4 x i32>(%psrldq.i20.i60);
;               |   %shuffle.i.i21.i61 = shufflevector %9,  undef,  <i32 0, i32 1>;
;               |   %conv.i.i22.i62 = sitofp.<2 x i32>.<2 x double>(%shuffle.i.i21.i61);
;               |   %mul.i.i17.i64 = %conv.i.i22.i62  *  %conv.i.i.i59;
;               |(1)%add.i.i18.i65 = %v_sum0.sroa.0.0103  +  %mul.i.i17.i64;
;               |   %10 = bitcast.<16 x i8>.<4 x i32>(%3);
;               |   %shuffle.i.i11.i66 = shufflevector %10,  undef,  <i32 0, i32 1>;
;               |   %conv.i.i12.i67 = sitofp.<2 x i32>.<2 x double>(%shuffle.i.i11.i66);
;               |   %11 = bitcast.<16 x i8>.<4 x i32>(%1);
;               |   %shuffle.i.i8.i68 = shufflevector %11,  undef,  <i32 0, i32 1>;
;               |   %conv.i.i9.i69 = sitofp.<2 x i32>.<2 x double>(%shuffle.i.i8.i68);
;               |   %mul.i.i.i70 = %conv.i.i9.i69  *  %conv.i.i12.i67;
;               |(2)%v_sum0.sroa.0.0103 = %mul.i.i.i70  +  %add.i.i18.i65;
;               |   %psrldq.i.i41 = shufflevector %7,  poison,  <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>;
;               |   %12 = bitcast.<16 x i8>.<4 x i32>(%psrldq.i.i41);
;               |   %shuffle.i.i.i42 = shufflevector %12,  undef,  <i32 0, i32 1>;
;               |   %conv.i.i.i43 = sitofp.<2 x i32>.<2 x double>(%shuffle.i.i.i42);
;               |   %psrldq.i20.i44 = shufflevector %5,  poison,  <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>;
;               |   %13 = bitcast.<16 x i8>.<4 x i32>(%psrldq.i20.i44);
;               |   %shuffle.i.i21.i45 = shufflevector %13,  undef,  <i32 0, i32 1>;
;               |   %conv.i.i22.i46 = sitofp.<2 x i32>.<2 x double>(%shuffle.i.i21.i45);
;               |   %mul.i.i17.i48 = %conv.i.i22.i46  *  %conv.i.i.i43;
;               |(3)%add.i.i18.i49 = %v_sum1.sroa.0.0102  +  %mul.i.i17.i48;
;               |   %14 = bitcast.<16 x i8>.<4 x i32>(%7);
;               |   %shuffle.i.i11.i50 = shufflevector %14,  undef,  <i32 0, i32 1>;
;               |   %conv.i.i12.i51 = sitofp.<2 x i32>.<2 x double>(%shuffle.i.i11.i50);
;               |   %15 = bitcast.<16 x i8>.<4 x i32>(%5);
;               |   %shuffle.i.i8.i52 = shufflevector %15,  undef,  <i32 0, i32 1>;
;               |   %conv.i.i9.i53 = sitofp.<2 x i32>.<2 x double>(%shuffle.i.i8.i52);
;               |   %mul.i.i.i54 = %conv.i.i9.i53  *  %conv.i.i12.i51;
;               |(4)%v_sum1.sroa.0.0102 = %mul.i.i.i54  +  %add.i.i18.i49;
;               |   %add.ptr3 = &((%src1)[8 * i1 + 8]);
;               |   %add.ptr4 = &((%src2)[8 * i1 + 8]);
;               + END LOOP
;         END REGION

; CHECK-NOT:  = (<16 x i8>*)(%src1)[4 * i1];
; CHECK-NOT:  = (<16 x i8>*)(%src2)[4 * i1];

; ModuleID = 'module.ll'
source_filename = "/nfs/site/home/aeloviko/s/tmp/matmul-06b528.cpp"
target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.15.26726"

; Function Attrs: nofree nounwind sspstrong uwtable
define dso_local double @"?dotProd_32s@opt_SSE4_1@cv@@YANPEBH0H@Z"(ptr nocapture readonly %src1, ptr nocapture readonly %src2, i32 %len) local_unnamed_addr #0 {
entry:
  %sub = add nsw i32 %len, -8
  %cmp101 = icmp sgt i32 %sub, 0
  br i1 %cmp101, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %src2.addr.0106 = phi ptr [ %add.ptr4, %for.body ], [ %src2, %for.body.preheader ]
  %src1.addr.0105 = phi ptr [ %add.ptr3, %for.body ], [ %src1, %for.body.preheader ]
  %i.0104 = phi i32 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %v_sum0.sroa.0.0103 = phi <2 x double> [ %add.i.i.i71, %for.body ], [ zeroinitializer, %for.body.preheader ]
  %v_sum1.sroa.0.0102 = phi <2 x double> [ %add.i.i.i55, %for.body ], [ zeroinitializer, %for.body.preheader ]
  %0 = load <16 x i8>, ptr %src1.addr.0105, align 1
  %1 = load <16 x i8>, ptr %src2.addr.0106, align 1
  %add.ptr = getelementptr inbounds i32, ptr %src1.addr.0105, i64 4
  %2 = load <16 x i8>, ptr %add.ptr, align 1
  %add.ptr1 = getelementptr inbounds i32, ptr %src2.addr.0106, i64 4
  %3 = load <16 x i8>, ptr %add.ptr1, align 1
  %psrldq.i.i57 = shufflevector <16 x i8> %1, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %4 = bitcast <16 x i8> %psrldq.i.i57 to <4 x i32>
  %shuffle.i.i.i58 = shufflevector <4 x i32> %4, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i.i59 = sitofp <2 x i32> %shuffle.i.i.i58 to <2 x double>
  %psrldq.i20.i60 = shufflevector <16 x i8> %0, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %5 = bitcast <16 x i8> %psrldq.i20.i60 to <4 x i32>
  %shuffle.i.i21.i61 = shufflevector <4 x i32> %5, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i22.i62 = sitofp <2 x i32> %shuffle.i.i21.i61 to <2 x double>
  %mul.i.i17.i64 = fmul contract <2 x double> %conv.i.i22.i62, %conv.i.i.i59
  %add.i.i18.i65 = fadd contract <2 x double> %v_sum0.sroa.0.0103, %mul.i.i17.i64
  %6 = bitcast <16 x i8> %1 to <4 x i32>
  %shuffle.i.i11.i66 = shufflevector <4 x i32> %6, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i12.i67 = sitofp <2 x i32> %shuffle.i.i11.i66 to <2 x double>
  %7 = bitcast <16 x i8> %0 to <4 x i32>
  %shuffle.i.i8.i68 = shufflevector <4 x i32> %7, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i9.i69 = sitofp <2 x i32> %shuffle.i.i8.i68 to <2 x double>
  %mul.i.i.i70 = fmul contract <2 x double> %conv.i.i9.i69, %conv.i.i12.i67
  %add.i.i.i71 = fadd contract <2 x double> %mul.i.i.i70, %add.i.i18.i65
  %psrldq.i.i41 = shufflevector <16 x i8> %3, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %8 = bitcast <16 x i8> %psrldq.i.i41 to <4 x i32>
  %shuffle.i.i.i42 = shufflevector <4 x i32> %8, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i.i43 = sitofp <2 x i32> %shuffle.i.i.i42 to <2 x double>
  %psrldq.i20.i44 = shufflevector <16 x i8> %2, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %9 = bitcast <16 x i8> %psrldq.i20.i44 to <4 x i32>
  %shuffle.i.i21.i45 = shufflevector <4 x i32> %9, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i22.i46 = sitofp <2 x i32> %shuffle.i.i21.i45 to <2 x double>
  %mul.i.i17.i48 = fmul contract <2 x double> %conv.i.i22.i46, %conv.i.i.i43
  %add.i.i18.i49 = fadd contract <2 x double> %v_sum1.sroa.0.0102, %mul.i.i17.i48
  %10 = bitcast <16 x i8> %3 to <4 x i32>
  %shuffle.i.i11.i50 = shufflevector <4 x i32> %10, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i12.i51 = sitofp <2 x i32> %shuffle.i.i11.i50 to <2 x double>
  %11 = bitcast <16 x i8> %2 to <4 x i32>
  %shuffle.i.i8.i52 = shufflevector <4 x i32> %11, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i9.i53 = sitofp <2 x i32> %shuffle.i.i8.i52 to <2 x double>
  %mul.i.i.i54 = fmul contract <2 x double> %conv.i.i9.i53, %conv.i.i12.i51
  %add.i.i.i55 = fadd contract <2 x double> %mul.i.i.i54, %add.i.i18.i49
  %add = add nuw nsw i32 %i.0104, 8
  %add.ptr3 = getelementptr inbounds i32, ptr %src1.addr.0105, i64 8
  %add.ptr4 = getelementptr inbounds i32, ptr %src2.addr.0106, i64 8
  %cmp = icmp slt i32 %add, %sub
  br i1 %cmp, label %for.body, label %for.end.loopexit

for.end.loopexit:                                 ; preds = %for.body
  %add.i.i.i71.lcssa = phi <2 x double> [ %add.i.i.i71, %for.body ]
  %add.i.i.i55.lcssa = phi <2 x double> [ %add.i.i.i55, %for.body ]
  %add.ptr3.lcssa = phi ptr [ %add.ptr3, %for.body ]
  %add.ptr4.lcssa = phi ptr [ %add.ptr4, %for.body ]
  %12 = add i32 %len, -1
  %13 = and i32 %12, -8
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %v_sum1.sroa.0.0.lcssa = phi <2 x double> [ zeroinitializer, %entry ], [ %add.i.i.i55.lcssa, %for.end.loopexit ]
  %v_sum0.sroa.0.0.lcssa = phi <2 x double> [ zeroinitializer, %entry ], [ %add.i.i.i71.lcssa, %for.end.loopexit ]
  %i.0.lcssa = phi i32 [ 0, %entry ], [ %13, %for.end.loopexit ]
  %src1.addr.0.lcssa = phi ptr [ %src1, %entry ], [ %add.ptr3.lcssa, %for.end.loopexit ]
  %src2.addr.0.lcssa = phi ptr [ %src2, %entry ], [ %add.ptr4.lcssa, %for.end.loopexit ]
  %add.i.i = fadd contract <2 x double> %v_sum1.sroa.0.0.lcssa, %v_sum0.sroa.0.0.lcssa
  %sub6 = add nsw i32 %len, -4
  %cmp793 = icmp slt i32 %i.0.lcssa, %sub6
  br i1 %cmp793, label %for.body8.preheader, label %for.end14

for.body8.preheader:                              ; preds = %for.end
  br label %for.body8

for.body8:                                        ; preds = %for.body8, %for.body8.preheader
  %src2.addr.197 = phi ptr [ %add.ptr13, %for.body8 ], [ %src2.addr.0.lcssa, %for.body8.preheader ]
  %src1.addr.196 = phi ptr [ %add.ptr12, %for.body8 ], [ %src1.addr.0.lcssa, %for.body8.preheader ]
  %i.195 = phi i32 [ %add11, %for.body8 ], [ %i.0.lcssa, %for.body8.preheader ]
  %v_sum0.sroa.0.194 = phi <2 x double> [ %add.i.i.i, %for.body8 ], [ %add.i.i, %for.body8.preheader ]
  %14 = load <16 x i8>, ptr %src1.addr.196, align 1
  %15 = load <16 x i8>, ptr %src2.addr.197, align 1
  %psrldq.i.i = shufflevector <16 x i8> %15, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %16 = bitcast <16 x i8> %psrldq.i.i to <4 x i32>
  %shuffle.i.i.i = shufflevector <4 x i32> %16, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i.i = sitofp <2 x i32> %shuffle.i.i.i to <2 x double>
  %psrldq.i20.i = shufflevector <16 x i8> %14, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %17 = bitcast <16 x i8> %psrldq.i20.i to <4 x i32>
  %shuffle.i.i21.i = shufflevector <4 x i32> %17, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i22.i = sitofp <2 x i32> %shuffle.i.i21.i to <2 x double>
  %mul.i.i17.i = fmul contract <2 x double> %conv.i.i22.i, %conv.i.i.i
  %add.i.i18.i = fadd contract <2 x double> %v_sum0.sroa.0.194, %mul.i.i17.i
  %18 = bitcast <16 x i8> %15 to <4 x i32>
  %shuffle.i.i11.i = shufflevector <4 x i32> %18, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i12.i = sitofp <2 x i32> %shuffle.i.i11.i to <2 x double>
  %19 = bitcast <16 x i8> %14 to <4 x i32>
  %shuffle.i.i8.i = shufflevector <4 x i32> %19, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i9.i = sitofp <2 x i32> %shuffle.i.i8.i to <2 x double>
  %mul.i.i.i = fmul contract <2 x double> %conv.i.i9.i, %conv.i.i12.i
  %add.i.i.i = fadd contract <2 x double> %mul.i.i.i, %add.i.i18.i
  %add11 = add nuw nsw i32 %i.195, 4
  %add.ptr12 = getelementptr inbounds i32, ptr %src1.addr.196, i64 4
  %add.ptr13 = getelementptr inbounds i32, ptr %src2.addr.197, i64 4
  %cmp7 = icmp slt i32 %add11, %sub6
  br i1 %cmp7, label %for.body8, label %for.end14.loopexit

for.end14.loopexit:                               ; preds = %for.body8
  %add.i.i.i.lcssa = phi <2 x double> [ %add.i.i.i, %for.body8 ]
  %add11.lcssa = phi i32 [ %add11, %for.body8 ]
  %add.ptr12.lcssa = phi ptr [ %add.ptr12, %for.body8 ]
  %add.ptr13.lcssa = phi ptr [ %add.ptr13, %for.body8 ]
  br label %for.end14

for.end14:                                        ; preds = %for.end14.loopexit, %for.end
  %v_sum0.sroa.0.1.lcssa = phi <2 x double> [ %add.i.i, %for.end ], [ %add.i.i.i.lcssa, %for.end14.loopexit ]
  %i.1.lcssa = phi i32 [ %i.0.lcssa, %for.end ], [ %add11.lcssa, %for.end14.loopexit ]
  %src1.addr.1.lcssa = phi ptr [ %src1.addr.0.lcssa, %for.end ], [ %add.ptr12.lcssa, %for.end14.loopexit ]
  %src2.addr.1.lcssa = phi ptr [ %src2.addr.0.lcssa, %for.end ], [ %add.ptr13.lcssa, %for.end14.loopexit ]
  %sub16 = sub nsw i32 %len, %i.1.lcssa
  %sub.i = add i32 %sub16, -4
  %cmp.not77.i = icmp slt i32 %sub.i, 0
  br i1 %cmp.not77.i, label %for.cond35.preheader.i, label %for.body.preheader.i

for.body.preheader.i:                             ; preds = %for.end14
  %20 = zext i32 %sub.i to i64
  br label %for.body.i

for.cond35.preheader.loopexit.i:                  ; preds = %for.body.i
  %add33.i.lcssa = phi double [ %add33.i, %for.body.i ]
  %21 = and i32 %sub16, -4
  br label %for.cond35.preheader.i

for.cond35.preheader.i:                           ; preds = %for.cond35.preheader.loopexit.i, %for.end14
  %i.0.lcssa.i = phi i32 [ 0, %for.end14 ], [ %21, %for.cond35.preheader.loopexit.i ]
  %result.0.lcssa.i = phi double [ 0.000000e+00, %for.end14 ], [ %add33.i.lcssa, %for.cond35.preheader.loopexit.i ]
  %cmp3674.i = icmp slt i32 %i.0.lcssa.i, %sub16
  br i1 %cmp3674.i, label %for.body37.preheader.i, label %"??$dotProd_@H@opt_SSE4_1@cv@@YANPEBH0H@Z.exit"

for.body37.preheader.i:                           ; preds = %for.cond35.preheader.i
  %22 = zext i32 %i.0.lcssa.i to i64
  %wide.trip.count.i = sext i32 %sub16 to i64
  br label %for.body37.i

for.body.i:                                       ; preds = %for.body.i, %for.body.preheader.i
  %indvars.iv82.i = phi i64 [ 0, %for.body.preheader.i ], [ %indvars.iv.next83.i, %for.body.i ]
  %result.079.i = phi double [ 0.000000e+00, %for.body.preheader.i ], [ %add33.i, %for.body.i ]
  %ptridx.i = getelementptr inbounds i32, ptr %src1.addr.1.lcssa, i64 %indvars.iv82.i
  %23 = load i32, ptr %ptridx.i, align 4
  %conv.i = sitofp i32 %23 to double
  %ptridx2.i = getelementptr inbounds i32, ptr %src2.addr.1.lcssa, i64 %indvars.iv82.i
  %24 = load i32, ptr %ptridx2.i, align 4
  %conv3.i = sitofp i32 %24 to double
  %mul.i = fmul contract double %conv.i, %conv3.i
  %25 = or i64 %indvars.iv82.i, 1
  %ptridx5.i = getelementptr inbounds i32, ptr %src1.addr.1.lcssa, i64 %25
  %26 = load i32, ptr %ptridx5.i, align 4
  %conv6.i = sitofp i32 %26 to double
  %ptridx9.i = getelementptr inbounds i32, ptr %src2.addr.1.lcssa, i64 %25
  %27 = load i32, ptr %ptridx9.i, align 4
  %conv10.i = sitofp i32 %27 to double
  %mul11.i = fmul contract double %conv6.i, %conv10.i
  %add12.i = fadd contract double %mul.i, %mul11.i
  %28 = or i64 %indvars.iv82.i, 2
  %ptridx15.i = getelementptr inbounds i32, ptr %src1.addr.1.lcssa, i64 %28
  %29 = load i32, ptr %ptridx15.i, align 4
  %conv16.i = sitofp i32 %29 to double
  %ptridx19.i = getelementptr inbounds i32, ptr %src2.addr.1.lcssa, i64 %28
  %30 = load i32, ptr %ptridx19.i, align 4
  %conv20.i = sitofp i32 %30 to double
  %mul21.i = fmul contract double %conv16.i, %conv20.i
  %add22.i = fadd contract double %add12.i, %mul21.i
  %31 = or i64 %indvars.iv82.i, 3
  %ptridx25.i = getelementptr inbounds i32, ptr %src1.addr.1.lcssa, i64 %31
  %32 = load i32, ptr %ptridx25.i, align 4
  %conv26.i = sitofp i32 %32 to double
  %ptridx29.i = getelementptr inbounds i32, ptr %src2.addr.1.lcssa, i64 %31
  %33 = load i32, ptr %ptridx29.i, align 4
  %conv30.i = sitofp i32 %33 to double
  %mul31.i = fmul contract double %conv26.i, %conv30.i
  %add32.i = fadd contract double %add22.i, %mul31.i
  %add33.i = fadd contract double %result.079.i, %add32.i
  %indvars.iv.next83.i = add nuw nsw i64 %indvars.iv82.i, 4
  %cmp.not.i = icmp ugt i64 %indvars.iv.next83.i, %20
  br i1 %cmp.not.i, label %for.cond35.preheader.loopexit.i, label %for.body.i

for.body37.i:                                     ; preds = %for.body37.i, %for.body37.preheader.i
  %indvars.iv.i = phi i64 [ %22, %for.body37.preheader.i ], [ %indvars.iv.next.i, %for.body37.i ]
  %result.176.i = phi double [ %result.0.lcssa.i, %for.body37.preheader.i ], [ %add45.i, %for.body37.i ]
  %ptridx39.i = getelementptr inbounds i32, ptr %src1.addr.1.lcssa, i64 %indvars.iv.i
  %34 = load i32, ptr %ptridx39.i, align 4
  %conv40.i = sitofp i32 %34 to double
  %ptridx42.i = getelementptr inbounds i32, ptr %src2.addr.1.lcssa, i64 %indvars.iv.i
  %35 = load i32, ptr %ptridx42.i, align 4
  %conv43.i = sitofp i32 %35 to double
  %mul44.i = fmul contract double %conv40.i, %conv43.i
  %add45.i = fadd contract double %result.176.i, %mul44.i
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, %wide.trip.count.i
  br i1 %exitcond.not.i, label %"??$dotProd_@H@opt_SSE4_1@cv@@YANPEBH0H@Z.exit.loopexit", label %for.body37.i

"??$dotProd_@H@opt_SSE4_1@cv@@YANPEBH0H@Z.exit.loopexit": ; preds = %for.body37.i
  %add45.i.lcssa = phi double [ %add45.i, %for.body37.i ]
  br label %"??$dotProd_@H@opt_SSE4_1@cv@@YANPEBH0H@Z.exit"

"??$dotProd_@H@opt_SSE4_1@cv@@YANPEBH0H@Z.exit":  ; preds = %"??$dotProd_@H@opt_SSE4_1@cv@@YANPEBH0H@Z.exit.loopexit", %for.cond35.preheader.i
  %result.1.lcssa.i = phi double [ %result.0.lcssa.i, %for.cond35.preheader.i ], [ %add45.i.lcssa, %"??$dotProd_@H@opt_SSE4_1@cv@@YANPEBH0H@Z.exit.loopexit" ]
  %idx.sroa.0.0.vec.extract.i = extractelement <2 x double> %v_sum0.sroa.0.1.lcssa, i32 0
  %idx.sroa.0.8.vec.extract.i = extractelement <2 x double> %v_sum0.sroa.0.1.lcssa, i32 1
  %add.i = fadd contract double %idx.sroa.0.0.vec.extract.i, %idx.sroa.0.8.vec.extract.i
  %add18 = fadd contract double %add.i, %result.1.lcssa.i
  ret double %add18
}



