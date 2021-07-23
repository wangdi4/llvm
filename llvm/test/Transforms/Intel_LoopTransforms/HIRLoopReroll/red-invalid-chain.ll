; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-loop-reroll  -print-before=hir-loop-reroll -print-after=hir-loop-reroll -hir-verify-cf-def-level  < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,print<hir>,hir-loop-reroll,print<hir>" -aa-pipeline="basic-aa" -hir-verify-cf-def-level < %s 2>&1 | FileCheck %s

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
define dso_local double @"?dotProd_32s@opt_SSE4_1@cv@@YANPEBH0H@Z"(i32* nocapture readonly %src1, i32* nocapture readonly %src2, i32 %len) local_unnamed_addr #0 {
entry:
  %sub = add nsw i32 %len, -8
  %cmp101 = icmp sgt i32 %sub, 0
  br i1 %cmp101, label %for.body.preheader, label %for.end

for.body.preheader:                               ; preds = %entry
  br label %for.body

for.body:                                         ; preds = %for.body, %for.body.preheader
  %src2.addr.0106 = phi i32* [ %add.ptr4, %for.body ], [ %src2, %for.body.preheader ]
  %src1.addr.0105 = phi i32* [ %add.ptr3, %for.body ], [ %src1, %for.body.preheader ]
  %i.0104 = phi i32 [ %add, %for.body ], [ 0, %for.body.preheader ]
  %v_sum0.sroa.0.0103 = phi <2 x double> [ %add.i.i.i71, %for.body ], [ zeroinitializer, %for.body.preheader ]
  %v_sum1.sroa.0.0102 = phi <2 x double> [ %add.i.i.i55, %for.body ], [ zeroinitializer, %for.body.preheader ]
  %0 = bitcast i32* %src1.addr.0105 to <16 x i8>*
  %1 = load <16 x i8>, <16 x i8>* %0, align 1, !tbaa !14, !noalias !17
  %2 = bitcast i32* %src2.addr.0106 to <16 x i8>*
  %3 = load <16 x i8>, <16 x i8>* %2, align 1, !tbaa !14, !noalias !22
  %add.ptr = getelementptr inbounds i32, i32* %src1.addr.0105, i64 4, !intel-tbaa !27
  %4 = bitcast i32* %add.ptr to <16 x i8>*
  %5 = load <16 x i8>, <16 x i8>* %4, align 1, !tbaa !14, !noalias !29
  %add.ptr1 = getelementptr inbounds i32, i32* %src2.addr.0106, i64 4, !intel-tbaa !27
  %6 = bitcast i32* %add.ptr1 to <16 x i8>*
  %7 = load <16 x i8>, <16 x i8>* %6, align 1, !tbaa !14, !noalias !34
  %psrldq.i.i57 = shufflevector <16 x i8> %3, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %8 = bitcast <16 x i8> %psrldq.i.i57 to <4 x i32>
  %shuffle.i.i.i58 = shufflevector <4 x i32> %8, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i.i59 = sitofp <2 x i32> %shuffle.i.i.i58 to <2 x double>
  %psrldq.i20.i60 = shufflevector <16 x i8> %1, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %9 = bitcast <16 x i8> %psrldq.i20.i60 to <4 x i32>
  %shuffle.i.i21.i61 = shufflevector <4 x i32> %9, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i22.i62 = sitofp <2 x i32> %shuffle.i.i21.i61 to <2 x double>
  %mul.i.i17.i64 = fmul contract <2 x double> %conv.i.i22.i62, %conv.i.i.i59
  %add.i.i18.i65 = fadd contract <2 x double> %v_sum0.sroa.0.0103, %mul.i.i17.i64
  %10 = bitcast <16 x i8> %3 to <4 x i32>
  %shuffle.i.i11.i66 = shufflevector <4 x i32> %10, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i12.i67 = sitofp <2 x i32> %shuffle.i.i11.i66 to <2 x double>
  %11 = bitcast <16 x i8> %1 to <4 x i32>
  %shuffle.i.i8.i68 = shufflevector <4 x i32> %11, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i9.i69 = sitofp <2 x i32> %shuffle.i.i8.i68 to <2 x double>
  %mul.i.i.i70 = fmul contract <2 x double> %conv.i.i9.i69, %conv.i.i12.i67
  %add.i.i.i71 = fadd contract <2 x double> %mul.i.i.i70, %add.i.i18.i65
  %psrldq.i.i41 = shufflevector <16 x i8> %7, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %12 = bitcast <16 x i8> %psrldq.i.i41 to <4 x i32>
  %shuffle.i.i.i42 = shufflevector <4 x i32> %12, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i.i43 = sitofp <2 x i32> %shuffle.i.i.i42 to <2 x double>
  %psrldq.i20.i44 = shufflevector <16 x i8> %5, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %13 = bitcast <16 x i8> %psrldq.i20.i44 to <4 x i32>
  %shuffle.i.i21.i45 = shufflevector <4 x i32> %13, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i22.i46 = sitofp <2 x i32> %shuffle.i.i21.i45 to <2 x double>
  %mul.i.i17.i48 = fmul contract <2 x double> %conv.i.i22.i46, %conv.i.i.i43
  %add.i.i18.i49 = fadd contract <2 x double> %v_sum1.sroa.0.0102, %mul.i.i17.i48
  %14 = bitcast <16 x i8> %7 to <4 x i32>
  %shuffle.i.i11.i50 = shufflevector <4 x i32> %14, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i12.i51 = sitofp <2 x i32> %shuffle.i.i11.i50 to <2 x double>
  %15 = bitcast <16 x i8> %5 to <4 x i32>
  %shuffle.i.i8.i52 = shufflevector <4 x i32> %15, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i9.i53 = sitofp <2 x i32> %shuffle.i.i8.i52 to <2 x double>
  %mul.i.i.i54 = fmul contract <2 x double> %conv.i.i9.i53, %conv.i.i12.i51
  %add.i.i.i55 = fadd contract <2 x double> %mul.i.i.i54, %add.i.i18.i49
  %add = add nuw nsw i32 %i.0104, 8
  %add.ptr3 = getelementptr inbounds i32, i32* %src1.addr.0105, i64 8, !intel-tbaa !27
  %add.ptr4 = getelementptr inbounds i32, i32* %src2.addr.0106, i64 8, !intel-tbaa !27
  %cmp = icmp slt i32 %add, %sub
  br i1 %cmp, label %for.body, label %for.end.loopexit, !llvm.loop !39

for.end.loopexit:                                 ; preds = %for.body
  %add.i.i.i71.lcssa = phi <2 x double> [ %add.i.i.i71, %for.body ]
  %add.i.i.i55.lcssa = phi <2 x double> [ %add.i.i.i55, %for.body ]
  %add.ptr3.lcssa = phi i32* [ %add.ptr3, %for.body ]
  %add.ptr4.lcssa = phi i32* [ %add.ptr4, %for.body ]
  %16 = add i32 %len, -1
  %17 = and i32 %16, -8
  br label %for.end

for.end:                                          ; preds = %for.end.loopexit, %entry
  %v_sum1.sroa.0.0.lcssa = phi <2 x double> [ zeroinitializer, %entry ], [ %add.i.i.i55.lcssa, %for.end.loopexit ]
  %v_sum0.sroa.0.0.lcssa = phi <2 x double> [ zeroinitializer, %entry ], [ %add.i.i.i71.lcssa, %for.end.loopexit ]
  %i.0.lcssa = phi i32 [ 0, %entry ], [ %17, %for.end.loopexit ]
  %src1.addr.0.lcssa = phi i32* [ %src1, %entry ], [ %add.ptr3.lcssa, %for.end.loopexit ]
  %src2.addr.0.lcssa = phi i32* [ %src2, %entry ], [ %add.ptr4.lcssa, %for.end.loopexit ]
  %add.i.i = fadd contract <2 x double> %v_sum1.sroa.0.0.lcssa, %v_sum0.sroa.0.0.lcssa
  %sub6 = add nsw i32 %len, -4
  %cmp793 = icmp slt i32 %i.0.lcssa, %sub6
  br i1 %cmp793, label %for.body8.preheader, label %for.end14

for.body8.preheader:                              ; preds = %for.end
  br label %for.body8

for.body8:                                        ; preds = %for.body8, %for.body8.preheader
  %src2.addr.197 = phi i32* [ %add.ptr13, %for.body8 ], [ %src2.addr.0.lcssa, %for.body8.preheader ]
  %src1.addr.196 = phi i32* [ %add.ptr12, %for.body8 ], [ %src1.addr.0.lcssa, %for.body8.preheader ]
  %i.195 = phi i32 [ %add11, %for.body8 ], [ %i.0.lcssa, %for.body8.preheader ]
  %v_sum0.sroa.0.194 = phi <2 x double> [ %add.i.i.i, %for.body8 ], [ %add.i.i, %for.body8.preheader ]
  %18 = bitcast i32* %src1.addr.196 to <16 x i8>*
  %19 = load <16 x i8>, <16 x i8>* %18, align 1, !tbaa !14, !noalias !41
  %20 = bitcast i32* %src2.addr.197 to <16 x i8>*
  %21 = load <16 x i8>, <16 x i8>* %20, align 1, !tbaa !14, !noalias !46
  %psrldq.i.i = shufflevector <16 x i8> %21, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %22 = bitcast <16 x i8> %psrldq.i.i to <4 x i32>
  %shuffle.i.i.i = shufflevector <4 x i32> %22, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i.i = sitofp <2 x i32> %shuffle.i.i.i to <2 x double>
  %psrldq.i20.i = shufflevector <16 x i8> %19, <16 x i8> poison, <16 x i32> <i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef, i32 undef>
  %23 = bitcast <16 x i8> %psrldq.i20.i to <4 x i32>
  %shuffle.i.i21.i = shufflevector <4 x i32> %23, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i22.i = sitofp <2 x i32> %shuffle.i.i21.i to <2 x double>
  %mul.i.i17.i = fmul contract <2 x double> %conv.i.i22.i, %conv.i.i.i
  %add.i.i18.i = fadd contract <2 x double> %v_sum0.sroa.0.194, %mul.i.i17.i
  %24 = bitcast <16 x i8> %21 to <4 x i32>
  %shuffle.i.i11.i = shufflevector <4 x i32> %24, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i12.i = sitofp <2 x i32> %shuffle.i.i11.i to <2 x double>
  %25 = bitcast <16 x i8> %19 to <4 x i32>
  %shuffle.i.i8.i = shufflevector <4 x i32> %25, <4 x i32> undef, <2 x i32> <i32 0, i32 1>
  %conv.i.i9.i = sitofp <2 x i32> %shuffle.i.i8.i to <2 x double>
  %mul.i.i.i = fmul contract <2 x double> %conv.i.i9.i, %conv.i.i12.i
  %add.i.i.i = fadd contract <2 x double> %mul.i.i.i, %add.i.i18.i
  %add11 = add nuw nsw i32 %i.195, 4
  %add.ptr12 = getelementptr inbounds i32, i32* %src1.addr.196, i64 4, !intel-tbaa !27
  %add.ptr13 = getelementptr inbounds i32, i32* %src2.addr.197, i64 4, !intel-tbaa !27
  %cmp7 = icmp slt i32 %add11, %sub6
  br i1 %cmp7, label %for.body8, label %for.end14.loopexit, !llvm.loop !51

for.end14.loopexit:                               ; preds = %for.body8
  %add.i.i.i.lcssa = phi <2 x double> [ %add.i.i.i, %for.body8 ]
  %add11.lcssa = phi i32 [ %add11, %for.body8 ]
  %add.ptr12.lcssa = phi i32* [ %add.ptr12, %for.body8 ]
  %add.ptr13.lcssa = phi i32* [ %add.ptr13, %for.body8 ]
  br label %for.end14

for.end14:                                        ; preds = %for.end14.loopexit, %for.end
  %v_sum0.sroa.0.1.lcssa = phi <2 x double> [ %add.i.i, %for.end ], [ %add.i.i.i.lcssa, %for.end14.loopexit ]
  %i.1.lcssa = phi i32 [ %i.0.lcssa, %for.end ], [ %add11.lcssa, %for.end14.loopexit ]
  %src1.addr.1.lcssa = phi i32* [ %src1.addr.0.lcssa, %for.end ], [ %add.ptr12.lcssa, %for.end14.loopexit ]
  %src2.addr.1.lcssa = phi i32* [ %src2.addr.0.lcssa, %for.end ], [ %add.ptr13.lcssa, %for.end14.loopexit ]
  %sub16 = sub nsw i32 %len, %i.1.lcssa
  %sub.i = add i32 %sub16, -4
  %cmp.not77.i = icmp slt i32 %sub.i, 0
  br i1 %cmp.not77.i, label %for.cond35.preheader.i, label %for.body.preheader.i

for.body.preheader.i:                             ; preds = %for.end14
  %26 = zext i32 %sub.i to i64
  br label %for.body.i

for.cond35.preheader.loopexit.i:                  ; preds = %for.body.i
  %add33.i.lcssa = phi double [ %add33.i, %for.body.i ]
  %27 = and i32 %sub16, -4
  br label %for.cond35.preheader.i

for.cond35.preheader.i:                           ; preds = %for.cond35.preheader.loopexit.i, %for.end14
  %i.0.lcssa.i = phi i32 [ 0, %for.end14 ], [ %27, %for.cond35.preheader.loopexit.i ]
  %result.0.lcssa.i = phi double [ 0.000000e+00, %for.end14 ], [ %add33.i.lcssa, %for.cond35.preheader.loopexit.i ]
  %cmp3674.i = icmp slt i32 %i.0.lcssa.i, %sub16
  br i1 %cmp3674.i, label %for.body37.preheader.i, label %"??$dotProd_@H@opt_SSE4_1@cv@@YANPEBH0H@Z.exit"

for.body37.preheader.i:                           ; preds = %for.cond35.preheader.i
  %28 = zext i32 %i.0.lcssa.i to i64
  %wide.trip.count.i = sext i32 %sub16 to i64
  br label %for.body37.i

for.body.i:                                       ; preds = %for.body.i, %for.body.preheader.i
  %indvars.iv82.i = phi i64 [ 0, %for.body.preheader.i ], [ %indvars.iv.next83.i, %for.body.i ]
  %result.079.i = phi double [ 0.000000e+00, %for.body.preheader.i ], [ %add33.i, %for.body.i ]
  %ptridx.i = getelementptr inbounds i32, i32* %src1.addr.1.lcssa, i64 %indvars.iv82.i
  %29 = load i32, i32* %ptridx.i, align 4, !tbaa !27
  %conv.i = sitofp i32 %29 to double
  %ptridx2.i = getelementptr inbounds i32, i32* %src2.addr.1.lcssa, i64 %indvars.iv82.i
  %30 = load i32, i32* %ptridx2.i, align 4, !tbaa !27
  %conv3.i = sitofp i32 %30 to double
  %mul.i = fmul contract double %conv.i, %conv3.i
  %31 = or i64 %indvars.iv82.i, 1
  %ptridx5.i = getelementptr inbounds i32, i32* %src1.addr.1.lcssa, i64 %31
  %32 = load i32, i32* %ptridx5.i, align 4, !tbaa !27
  %conv6.i = sitofp i32 %32 to double
  %ptridx9.i = getelementptr inbounds i32, i32* %src2.addr.1.lcssa, i64 %31
  %33 = load i32, i32* %ptridx9.i, align 4, !tbaa !27
  %conv10.i = sitofp i32 %33 to double
  %mul11.i = fmul contract double %conv6.i, %conv10.i
  %add12.i = fadd contract double %mul.i, %mul11.i
  %34 = or i64 %indvars.iv82.i, 2
  %ptridx15.i = getelementptr inbounds i32, i32* %src1.addr.1.lcssa, i64 %34
  %35 = load i32, i32* %ptridx15.i, align 4, !tbaa !27
  %conv16.i = sitofp i32 %35 to double
  %ptridx19.i = getelementptr inbounds i32, i32* %src2.addr.1.lcssa, i64 %34
  %36 = load i32, i32* %ptridx19.i, align 4, !tbaa !27
  %conv20.i = sitofp i32 %36 to double
  %mul21.i = fmul contract double %conv16.i, %conv20.i
  %add22.i = fadd contract double %add12.i, %mul21.i
  %37 = or i64 %indvars.iv82.i, 3
  %ptridx25.i = getelementptr inbounds i32, i32* %src1.addr.1.lcssa, i64 %37
  %38 = load i32, i32* %ptridx25.i, align 4, !tbaa !27
  %conv26.i = sitofp i32 %38 to double
  %ptridx29.i = getelementptr inbounds i32, i32* %src2.addr.1.lcssa, i64 %37
  %39 = load i32, i32* %ptridx29.i, align 4, !tbaa !27
  %conv30.i = sitofp i32 %39 to double
  %mul31.i = fmul contract double %conv26.i, %conv30.i
  %add32.i = fadd contract double %add22.i, %mul31.i
  %add33.i = fadd contract double %result.079.i, %add32.i
  %indvars.iv.next83.i = add nuw nsw i64 %indvars.iv82.i, 4
  %cmp.not.i = icmp ugt i64 %indvars.iv.next83.i, %26
  br i1 %cmp.not.i, label %for.cond35.preheader.loopexit.i, label %for.body.i, !llvm.loop !52

for.body37.i:                                     ; preds = %for.body37.i, %for.body37.preheader.i
  %indvars.iv.i = phi i64 [ %28, %for.body37.preheader.i ], [ %indvars.iv.next.i, %for.body37.i ]
  %result.176.i = phi double [ %result.0.lcssa.i, %for.body37.preheader.i ], [ %add45.i, %for.body37.i ]
  %ptridx39.i = getelementptr inbounds i32, i32* %src1.addr.1.lcssa, i64 %indvars.iv.i
  %40 = load i32, i32* %ptridx39.i, align 4, !tbaa !27
  %conv40.i = sitofp i32 %40 to double
  %ptridx42.i = getelementptr inbounds i32, i32* %src2.addr.1.lcssa, i64 %indvars.iv.i
  %41 = load i32, i32* %ptridx42.i, align 4, !tbaa !27
  %conv43.i = sitofp i32 %41 to double
  %mul44.i = fmul contract double %conv40.i, %conv43.i
  %add45.i = fadd contract double %result.176.i, %mul44.i
  %indvars.iv.next.i = add nuw nsw i64 %indvars.iv.i, 1
  %exitcond.not.i = icmp eq i64 %indvars.iv.next.i, %wide.trip.count.i
  br i1 %exitcond.not.i, label %"??$dotProd_@H@opt_SSE4_1@cv@@YANPEBH0H@Z.exit.loopexit", label %for.body37.i, !llvm.loop !53

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

attributes #0 = { nofree nounwind sspstrong uwtable "frame-pointer"="none" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }

!llvm.linker.options = !{!0, !1, !2, !3, !4, !5, !6, !7, !8, !9, !10}
!llvm.module.flags = !{!11, !12}
!llvm.ident = !{!13}

!0 = !{!"/DEFAULTLIB:msvcrt.lib"}
!1 = !{!"/DEFAULTLIB:libircmt.lib"}
!2 = !{!"/DEFAULTLIB:svml_dispmd.lib"}
!3 = !{!"/DEFAULTLIB:libdecimal.lib"}
!4 = !{!"/DEFAULTLIB:libmmd.lib"}
!5 = !{!"/DEFAULTLIB:oldnames.lib"}
!6 = !{!"/FAILIFMISMATCH:\22_MSC_VER=1900\22"}
!7 = !{!"/FAILIFMISMATCH:\22_ITERATOR_DEBUG_LEVEL=0\22"}
!8 = !{!"/FAILIFMISMATCH:\22RuntimeLibrary=MT_StaticRelease\22"}
!9 = !{!"/DEFAULTLIB:libcpmt.lib"}
!10 = !{!"/FAILIFMISMATCH:\22_CRT_STDIO_ISO_WIDE_SPECIFIERS=0\22"}
!11 = !{i32 1, !"wchar_size", i32 2}
!12 = !{i32 7, !"PIC Level", i32 2}
!13 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!14 = !{!15, !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C++ TBAA"}
!17 = !{!18, !20}
!18 = distinct !{!18, !19, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z: %agg.result"}
!19 = distinct !{!19, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z"}
!20 = distinct !{!20, !21, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z: %agg.result"}
!21 = distinct !{!21, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z"}
!22 = !{!23, !25}
!23 = distinct !{!23, !24, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z: %agg.result"}
!24 = distinct !{!24, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z"}
!25 = distinct !{!25, !26, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z: %agg.result"}
!26 = distinct !{!26, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z"}
!27 = !{!28, !28, i64 0}
!28 = !{!"int", !15, i64 0}
!29 = !{!30, !32}
!30 = distinct !{!30, !31, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z: %agg.result"}
!31 = distinct !{!31, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z"}
!32 = distinct !{!32, !33, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z: %agg.result"}
!33 = distinct !{!33, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z"}
!34 = !{!35, !37}
!35 = distinct !{!35, !36, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z: %agg.result"}
!36 = distinct !{!36, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z"}
!37 = distinct !{!37, !38, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z: %agg.result"}
!38 = distinct !{!38, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z"}
!39 = distinct !{!39, !40}
!40 = !{!"llvm.loop.mustprogress"}
!41 = !{!42, !44}
!42 = distinct !{!42, !43, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z: %agg.result"}
!43 = distinct !{!43, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z"}
!44 = distinct !{!44, !45, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z: %agg.result"}
!45 = distinct !{!45, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z"}
!46 = !{!47, !49}
!47 = distinct !{!47, !48, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z: %agg.result"}
!48 = distinct !{!48, !"?v_load@hal_SSE4_1@cv@@YA?AUv_int32x4@12@PEBH@Z"}
!49 = distinct !{!49, !50, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z: %agg.result"}
!50 = distinct !{!50, !"?vx_load@simd128_cpp@hal_SSE4_1@cv@@YA?AUv_int32x4@23@PEBH@Z"}
!51 = distinct !{!51, !40}
!52 = distinct !{!52, !40}
!53 = distinct !{!53, !40}
