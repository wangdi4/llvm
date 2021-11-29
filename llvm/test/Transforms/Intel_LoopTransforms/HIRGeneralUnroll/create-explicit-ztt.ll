; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-lmm -hir-general-unroll -print-after=hir-lmm,hir-general-unroll -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-lmm,print<hir>,hir-general-unroll,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s

; Verify that an explicit-if with ztt is created after general unroll.
; Note that the original loop had postExit, but no ztt.; To be safe after GU, when the actual trip count is zero at runtime, an if with ztt is created.

; // After lmm, before GU
; CHECK: Function: eggs
;
; CHECK:         BEGIN REGION { modified }
; CHECK:                  %limm = 0;
; CHECK:               + DO i1 = 0, %tmp32 + -2, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:               |   %tmp34.out = -1 * i1 + %tmp32;
; CHECK:               |   %tmp36 = %tmp31  %  -1 * i1 + %tmp32;
; CHECK:               |   if (%tmp36 == 0)
; CHECK:               |   {
; CHECK:               |      if (i1 != 0)
; CHECK:               |      {
; CHECK:               |         (@global.10)[0] = %limm;
; CHECK:               |      }
; CHECK:               |      goto bb41;
; CHECK:               |   }
; CHECK:               |   %limm = -1 * i1 + %tmp32 + -1;
; CHECK:               + END LOOP
; CHECK:                  (@global.10)[0] = %limm;
; CHECK:         END REGION
;
; // After GU
; CHECK: Function: eggs
;
; CHECK:          BEGIN REGION { modified }
; CHECK:                if (0 <u %tmp32 + -1)
; CHECK:                {
; CHECK:                   %limm = 0;
; CHECK:                   %tgu = (%tmp32 + -1)/u2;
; CHECK:                   %tmp34.out = undef;
;
; CHECK:                   + DO i1 = 0, %tgu + -1, 1   <DO_MULTI_EXIT_LOOP> <nounroll>
; CHECK:                   |   %tmp34.out = -2 * i1 + %tmp32;
; CHECK:                   |   %tmp36 = %tmp31  %  -2 * i1 + %tmp32;
; CHECK:                   |   if (%tmp36 == 0)
; CHECK:                   |   {
; CHECK:                   |      if (2 * i1 != 0)
; CHECK:                   |      {
; CHECK:                   |         (@global.10)[0] = %limm;
; CHECK:                   |      }
; CHECK:                   |      goto bb41;
; CHECK:                   |   }
; CHECK:                   |   %limm = -2 * i1 + %tmp32 + -1;
; CHECK:                   |   %tmp34.out = -2 * i1 + %tmp32 + -1;
; CHECK:                   |   %tmp36 = %tmp31  %  -2 * i1 + %tmp32 + -1;
; CHECK:                   |   if (%tmp36 == 0)
; CHECK:                   |   {
; CHECK:                   |      if (2 * i1 + 1 != 0)
; CHECK:                   |      {
; CHECK:                   |         (@global.10)[0] = %limm;
; CHECK:                   |      }
; CHECK:                   |      goto bb41;
; CHECK:                   |   }
; CHECK:                   |   %limm = -2 * i1 + %tmp32 + -2;
; CHECK:                   + END LOOP
;
; CHECK:                   if (2 * %tgu <u %tmp32 + -1)
; CHECK:                   {
; CHECK:                      %tmp34.out = %tmp32 + -2 * %tgu;
; CHECK:                      %tmp36 = %tmp31  %  %tmp32 + -1 * (2 * %tgu);
; CHECK:                      if (%tmp36 == 0)
; CHECK:                      {
; CHECK:                         if (2 * %tgu != 0)
; CHECK:                         {
; CHECK:                            (@global.10)[0] = %limm;
; CHECK:                         }
; CHECK:                         goto bb41;
; CHECK:                      }
; CHECK:                      %limm = %tmp32 + -2 * %tgu + -1;
; CHECK:                   }
; CHECK:                   (@global.10)[0] = %limm;
; CHECK:                }
; CHECK:          END REGION


; ModuleID = 'f.ll'
source_filename = "/tmp/ifxf1m3X7.i90"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = external global i32, align 8
@global.1 = external global i32, align 8
@global.2 = external local_unnamed_addr global i32, align 8
@global.3 = external local_unnamed_addr global i32, align 8
@global.4 = external global i32, align 8
@global.5 = external global i32, align 8
@global.6 = external global i32, align 8
@global.7 = external global i32, align 8
@global.8 = external global i32, align 8
@global.9 = external global i32, align 8
@global.10 = external global i32, align 8
@global.11 = external local_unnamed_addr global i32, align 8
@global.12 = external local_unnamed_addr global float, align 8
@global.13 = external local_unnamed_addr global float, align 8
@global.14 = external local_unnamed_addr global i64, align 8
@global.15 = external global i64, align 8
@global.16 = external global i64, align 8
@global.17 = external global i64, align 8
@global.18 = external local_unnamed_addr global float, align 8
@global.19 = external global i32, align 8
@global.20 = external global i64, align 8
@global.21 = external hidden unnamed_addr constant [24 x i8]
@global.22 = external hidden unnamed_addr constant [19 x i8]
@global.23 = external hidden unnamed_addr constant [23 x i8]
@global.24 = external hidden unnamed_addr constant [16 x i8]
@global.25 = external hidden unnamed_addr constant [55 x i8]
@global.26 = external hidden unnamed_addr constant [15 x i8]
@global.27 = external hidden unnamed_addr constant [17 x i8]
@global.28 = external hidden unnamed_addr constant [55 x i8]
@global.29 = external local_unnamed_addr global float, align 8
@global.30 = external hidden unnamed_addr constant i32
@global.31 = external hidden unnamed_addr constant i32
@global.32 = external hidden unnamed_addr constant i32
@global.33 = external hidden unnamed_addr constant i32
@global.34 = external hidden unnamed_addr constant i32
@global.35 = external hidden unnamed_addr constant i32

; Function Attrs: nofree
declare i32 @for_write_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr #0

; Function Attrs: nofree
declare i32 @for_write_seq_lis_xmit(i8* nocapture readonly, i8* nocapture readonly, i8*) local_unnamed_addr #0

; Function Attrs: nounwind uwtable
define void @eggs() local_unnamed_addr #1 {
bb:
  %tmp = alloca [8 x i64], align 16
  %tmp1 = alloca i32, align 8
  %tmp2 = alloca [4 x i8], align 1
  %tmp3 = alloca <{ i64, i8* }>, align 8
  %tmp4 = alloca [4 x i8], align 1
  %tmp5 = alloca <{ i64, i8* }>, align 8
  %tmp6 = alloca [4 x i8], align 1
  %tmp7 = alloca <{ i32 }>, align 8
  %tmp8 = alloca [4 x i8], align 1
  %tmp9 = alloca <{ i64, i8* }>, align 8
  %tmp10 = alloca [4 x i8], align 1
  %tmp11 = alloca <{ i32 }>, align 8
  %tmp12 = alloca [4 x i8], align 1
  %tmp13 = alloca <{ i64, i8* }>, align 8
  %tmp14 = alloca [2 x i8], align 1
  %tmp15 = alloca [4 x i8], align 1
  %tmp16 = alloca <{ i64, i8* }>, align 8
  %tmp17 = alloca [4 x i8], align 1
  %tmp18 = alloca <{ i32 }>, align 8
  %tmp19 = alloca [4 x i8], align 1
  %tmp20 = alloca <{ i64, i8* }>, align 8
  %tmp21 = alloca [4 x i8], align 1
  %tmp22 = alloca <{ i32 }>, align 8
  %tmp23 = alloca [4 x i8], align 1
  %tmp24 = alloca <{ i64, i8* }>, align 8
  %tmp25 = alloca [4 x i8], align 1
  %tmp26 = alloca <{ i32 }>, align 8
  %tmp27 = alloca [4 x i8], align 1
  %tmp28 = alloca <{ i64, i8* }>, align 8
  %tmp29 = alloca [4 x i8], align 1
  %tmp30 = alloca <{ i32 }>, align 8
  %tmp31 = load i64, i64* @global.20, align 8, !tbaa !1
  %tmp32 = load i32, i32* @global.10, align 8, !tbaa !5
  br label %bb33

bb33:                                             ; preds = %bb38, %bb
  %tmp34 = phi i32 [ %tmp39, %bb38 ], [ %tmp32, %bb ]
  %tmp35 = sext i32 %tmp34 to i64
  %tmp36 = srem i64 %tmp31, %tmp35
  %tmp37 = icmp eq i64 %tmp36, 0
  br i1 %tmp37, label %bb41, label %bb38

bb38:                                             ; preds = %bb33
  %tmp39 = add nsw i32 %tmp34, -1
  store i32 %tmp39, i32* @global.10, align 8, !tbaa !5
  %tmp40 = icmp eq i32 %tmp39, 1
  br i1 %tmp40, label %bb41, label %bb33

bb41:                                             ; preds = %bb38, %bb33
  %tmp42 = phi i32 [ 1, %bb38 ], [ %tmp34, %bb33 ]
  %tmp43 = sext i32 %tmp42 to i64
  %tmp44 = sdiv i64 %tmp31, %tmp43
  %tmp45 = trunc i64 %tmp44 to i32
  store i32 %tmp45, i32* @global.11, align 8, !tbaa !7
  %tmp46 = load i32, i32* @global.19, align 8, !tbaa !9
  %tmp47 = icmp eq i32 %tmp46, 0
  br i1 %tmp47, label %bb48, label %bb112

bb48:                                             ; preds = %bb41
  %tmp49 = load i64, i64* @global.16, align 8, !tbaa !11
  %tmp50 = trunc i64 %tmp49 to i32
  %tmp51 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp2, i64 0, i64 0
  store i8 56, i8* %tmp51, align 1, !tbaa !13
  %tmp52 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp2, i64 0, i64 1
  store i8 4, i8* %tmp52, align 1, !tbaa !13
  %tmp53 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp2, i64 0, i64 2
  store i8 1, i8* %tmp53, align 1, !tbaa !13
  %tmp54 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp2, i64 0, i64 3
  store i8 0, i8* %tmp54, align 1, !tbaa !13
  %tmp55 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp3, i64 0, i32 0
  store i64 55, i64* %tmp55, align 8, !tbaa !14
  %tmp56 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp3, i64 0, i32 1
  store i8* getelementptr inbounds ([55 x i8], [55 x i8]* @global.28, i64 0, i64 0), i8** %tmp56, align 8, !tbaa !16
  %tmp57 = bitcast [8 x i64]* %tmp to i8*
  %tmp58 = bitcast <{ i64, i8* }>* %tmp3 to i8*
  %tmp59 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %tmp57, i32 %tmp50, i64 1239157112576, i8* nonnull %tmp51, i8* nonnull %tmp58) #2
  %tmp60 = load i64, i64* @global.16, align 8, !tbaa !11
  %tmp61 = trunc i64 %tmp60 to i32
  %tmp62 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp4, i64 0, i64 0
  store i8 56, i8* %tmp62, align 1, !tbaa !13
  %tmp63 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp4, i64 0, i64 1
  store i8 4, i8* %tmp63, align 1, !tbaa !13
  %tmp64 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp4, i64 0, i64 2
  store i8 2, i8* %tmp64, align 1, !tbaa !13
  %tmp65 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp4, i64 0, i64 3
  store i8 0, i8* %tmp65, align 1, !tbaa !13
  %tmp66 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp5, i64 0, i32 0
  store i64 17, i64* %tmp66, align 8, !tbaa !18
  %tmp67 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp5, i64 0, i32 1
  store i8* getelementptr inbounds ([17 x i8], [17 x i8]* @global.27, i64 0, i64 0), i8** %tmp67, align 8, !tbaa !20
  %tmp68 = bitcast <{ i64, i8* }>* %tmp5 to i8*
  %tmp69 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %tmp57, i32 %tmp61, i64 1239157112576, i8* nonnull %tmp62, i8* nonnull %tmp68) #2
  %tmp70 = load i32, i32* @global.10, align 8, !tbaa !5
  %tmp71 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp6, i64 0, i64 0
  store i8 9, i8* %tmp71, align 1, !tbaa !13
  %tmp72 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp6, i64 0, i64 1
  store i8 1, i8* %tmp72, align 1, !tbaa !13
  %tmp73 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp6, i64 0, i64 2
  store i8 2, i8* %tmp73, align 1, !tbaa !13
  %tmp74 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp6, i64 0, i64 3
  store i8 0, i8* %tmp74, align 1, !tbaa !13
  %tmp75 = getelementptr inbounds <{ i32 }>, <{ i32 }>* %tmp7, i64 0, i32 0
  store i32 %tmp70, i32* %tmp75, align 8, !tbaa !22
  %tmp76 = bitcast <{ i32 }>* %tmp7 to i8*
  %tmp77 = call i32 @for_write_seq_lis_xmit(i8* nonnull %tmp57, i8* nonnull %tmp71, i8* nonnull %tmp76) #2
  %tmp78 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp8, i64 0, i64 0
  store i8 56, i8* %tmp78, align 1, !tbaa !13
  %tmp79 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp8, i64 0, i64 1
  store i8 4, i8* %tmp79, align 1, !tbaa !13
  %tmp80 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp8, i64 0, i64 2
  store i8 2, i8* %tmp80, align 1, !tbaa !13
  %tmp81 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp8, i64 0, i64 3
  store i8 0, i8* %tmp81, align 1, !tbaa !13
  %tmp82 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp9, i64 0, i32 0
  store i64 15, i64* %tmp82, align 8, !tbaa !24
  %tmp83 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp9, i64 0, i32 1
  store i8* getelementptr inbounds ([15 x i8], [15 x i8]* @global.26, i64 0, i64 0), i8** %tmp83, align 8, !tbaa !26
  %tmp84 = bitcast <{ i64, i8* }>* %tmp9 to i8*
  %tmp85 = call i32 @for_write_seq_lis_xmit(i8* nonnull %tmp57, i8* nonnull %tmp78, i8* nonnull %tmp84) #2
  %tmp86 = load i32, i32* @global.11, align 8, !tbaa !7
  %tmp87 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp10, i64 0, i64 0
  store i8 9, i8* %tmp87, align 1, !tbaa !13
  %tmp88 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp10, i64 0, i64 1
  store i8 1, i8* %tmp88, align 1, !tbaa !13
  %tmp89 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp10, i64 0, i64 2
  store i8 1, i8* %tmp89, align 1, !tbaa !13
  %tmp90 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp10, i64 0, i64 3
  store i8 0, i8* %tmp90, align 1, !tbaa !13
  %tmp91 = getelementptr inbounds <{ i32 }>, <{ i32 }>* %tmp11, i64 0, i32 0
  store i32 %tmp86, i32* %tmp91, align 8, !tbaa !28
  %tmp92 = bitcast <{ i32 }>* %tmp11 to i8*
  %tmp93 = call i32 @for_write_seq_lis_xmit(i8* nonnull %tmp57, i8* nonnull %tmp87, i8* nonnull %tmp92) #2
  %tmp94 = load i64, i64* @global.16, align 8, !tbaa !11
  %tmp95 = trunc i64 %tmp94 to i32
  %tmp96 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp12, i64 0, i64 0
  store i8 56, i8* %tmp96, align 1, !tbaa !13
  %tmp97 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp12, i64 0, i64 1
  store i8 4, i8* %tmp97, align 1, !tbaa !13
  %tmp98 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp12, i64 0, i64 2
  store i8 1, i8* %tmp98, align 1, !tbaa !13
  %tmp99 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp12, i64 0, i64 3
  store i8 0, i8* %tmp99, align 1, !tbaa !13
  %tmp100 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp13, i64 0, i32 0
  store i64 55, i64* %tmp100, align 8, !tbaa !30
  %tmp101 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp13, i64 0, i32 1
  store i8* getelementptr inbounds ([55 x i8], [55 x i8]* @global.25, i64 0, i64 0), i8** %tmp101, align 8, !tbaa !32
  %tmp102 = bitcast <{ i64, i8* }>* %tmp13 to i8*
  %tmp103 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %tmp57, i32 %tmp95, i64 1239157112576, i8* nonnull %tmp96, i8* nonnull %tmp102) #2
  %tmp104 = load i64, i64* @global.16, align 8, !tbaa !11
  %tmp105 = trunc i64 %tmp104 to i32
  %tmp106 = getelementptr inbounds [2 x i8], [2 x i8]* %tmp14, i64 0, i64 0
  store i8 1, i8* %tmp106, align 1, !tbaa !13
  %tmp107 = getelementptr inbounds [2 x i8], [2 x i8]* %tmp14, i64 0, i64 1
  store i8 0, i8* %tmp107, align 1, !tbaa !13
  %tmp108 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %tmp57, i32 %tmp105, i64 1239157112576, i8* nonnull %tmp106, i8* null) #2
  %tmp109 = load i32, i32* @global.11, align 8, !tbaa !7
  %tmp110 = load i32, i32* @global.19, align 8, !tbaa !9
  %tmp111 = load i32, i32* @global.10, align 8, !tbaa !5
  br label %bb112

bb112:                                            ; preds = %bb48, %bb41
  %tmp113 = phi i32 [ %tmp42, %bb41 ], [ %tmp111, %bb48 ]
  %tmp114 = phi i32 [ %tmp46, %bb41 ], [ %tmp110, %bb48 ]
  %tmp115 = phi i32 [ %tmp45, %bb41 ], [ %tmp109, %bb48 ]
  %tmp116 = load i64, i64* @global.15, align 8, !tbaa !34
  %tmp117 = sitofp i64 %tmp116 to float
  %tmp118 = sitofp i32 %tmp115 to float
  %tmp119 = fdiv reassoc ninf nsz arcp contract afn float %tmp117, %tmp118
  %tmp120 = fadd reassoc ninf nsz arcp contract afn float %tmp119, 5.000000e-01
  %tmp121 = fptosi float %tmp120 to i32
  %tmp122 = icmp sgt i32 %tmp121, 1
  %tmp123 = select i1 %tmp122, i32 %tmp121, i32 1
  %tmp124 = mul nsw i32 %tmp123, %tmp115
  %tmp125 = sext i32 %tmp124 to i64
  store i64 %tmp125, i64* @global.15, align 8, !tbaa !34
  %tmp126 = load i64, i64* @global.17, align 8, !tbaa !36
  %tmp127 = sdiv i64 %tmp126, 2
  %tmp128 = shl nsw i64 %tmp127, 1
  store i64 %tmp128, i64* @global.17, align 8, !tbaa !36
  %tmp129 = srem i32 %tmp114, %tmp113
  store i32 %tmp129, i32* @global, align 8, !tbaa !38
  %tmp130 = sdiv i32 %tmp114, %tmp113
  store i32 %tmp130, i32* @global.1, align 8, !tbaa !40
  %tmp131 = sext i32 %tmp115 to i64
  %tmp132 = sdiv i64 %tmp125, %tmp131
  store i64 %tmp132, i64* @global.14, align 8, !tbaa !42
  %tmp133 = load float, float* @global.18, align 8, !tbaa !44
  %tmp134 = fmul reassoc ninf nsz arcp contract afn float %tmp133, 2.000000e+00
  %tmp135 = sitofp i32 %tmp130 to float
  %tmp136 = fmul reassoc ninf nsz arcp contract afn float %tmp134, %tmp135
  %tmp137 = fdiv reassoc ninf nsz arcp contract afn float %tmp136, %tmp118
  store float %tmp137, float* @global.13, align 8, !tbaa !46
  %tmp138 = add nsw i32 %tmp130, 1
  %tmp139 = sitofp i32 %tmp138 to float
  %tmp140 = fmul reassoc ninf nsz arcp contract afn float %tmp134, %tmp139
  %tmp141 = fdiv reassoc ninf nsz arcp contract afn float %tmp140, %tmp118
  store float %tmp141, float* @global.29, align 8, !tbaa !48
  %tmp142 = fsub reassoc ninf nsz arcp contract afn float %tmp141, %tmp137
  %tmp143 = sitofp i64 %tmp132 to float
  %tmp144 = fdiv reassoc ninf nsz arcp contract afn float %tmp142, %tmp143
  store float %tmp144, float* @global.12, align 8, !tbaa !50
  call void (...) @quux(i32* nonnull @global.30, i32* nonnull @global.1, i32* nonnull @global, i32* nonnull @global.9, i32* nonnull %tmp1) #2
  call void (...) @quux(i32* nonnull @global.31, i32* nonnull @global, i32* nonnull @global.1, i32* nonnull @global.6, i32* nonnull %tmp1) #2
  call void (...) @widget(i32* nonnull @global.9, i32* nonnull @global.8, i32* nonnull %tmp1) #2
  call void (...) @barney(i32* nonnull @global.9, i32* nonnull @global.7, i32* nonnull %tmp1) #2
  call void (...) @widget(i32* nonnull @global.6, i32* nonnull @global.5, i32* nonnull %tmp1) #2
  call void (...) @barney(i32* nonnull @global.6, i32* nonnull @global.4, i32* nonnull %tmp1) #2
  %tmp145 = load i32, i32* @global.8, align 8, !tbaa !52
  %tmp146 = load i32, i32* @global.10, align 8, !tbaa !5
  %tmp147 = icmp eq i32 %tmp145, %tmp146
  br i1 %tmp147, label %bb182, label %bb148

bb148:                                            ; preds = %bb112
  %tmp149 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp15, i64 0, i64 0
  store i8 56, i8* %tmp149, align 1, !tbaa !13
  %tmp150 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp15, i64 0, i64 1
  store i8 4, i8* %tmp150, align 1, !tbaa !13
  %tmp151 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp15, i64 0, i64 2
  store i8 2, i8* %tmp151, align 1, !tbaa !13
  %tmp152 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp15, i64 0, i64 3
  store i8 0, i8* %tmp152, align 1, !tbaa !13
  %tmp153 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp16, i64 0, i32 0
  store i64 16, i64* %tmp153, align 8, !tbaa !54
  %tmp154 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp16, i64 0, i32 1
  store i8* getelementptr inbounds ([16 x i8], [16 x i8]* @global.24, i64 0, i64 0), i8** %tmp154, align 8, !tbaa !56
  %tmp155 = bitcast [8 x i64]* %tmp to i8*
  %tmp156 = bitcast <{ i64, i8* }>* %tmp16 to i8*
  %tmp157 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %tmp155, i32 0, i64 1239157112576, i8* nonnull %tmp149, i8* nonnull %tmp156) #2
  %tmp158 = load i32, i32* @global.8, align 8, !tbaa !52
  %tmp159 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp17, i64 0, i64 0
  store i8 9, i8* %tmp159, align 1, !tbaa !13
  %tmp160 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp17, i64 0, i64 1
  store i8 1, i8* %tmp160, align 1, !tbaa !13
  %tmp161 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp17, i64 0, i64 2
  store i8 2, i8* %tmp161, align 1, !tbaa !13
  %tmp162 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp17, i64 0, i64 3
  store i8 0, i8* %tmp162, align 1, !tbaa !13
  %tmp163 = getelementptr inbounds <{ i32 }>, <{ i32 }>* %tmp18, i64 0, i32 0
  store i32 %tmp158, i32* %tmp163, align 8, !tbaa !58
  %tmp164 = bitcast <{ i32 }>* %tmp18 to i8*
  %tmp165 = call i32 @for_write_seq_lis_xmit(i8* nonnull %tmp155, i8* nonnull %tmp159, i8* nonnull %tmp164) #2
  %tmp166 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp19, i64 0, i64 0
  store i8 56, i8* %tmp166, align 1, !tbaa !13
  %tmp167 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp19, i64 0, i64 1
  store i8 4, i8* %tmp167, align 1, !tbaa !13
  %tmp168 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp19, i64 0, i64 2
  store i8 2, i8* %tmp168, align 1, !tbaa !13
  %tmp169 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp19, i64 0, i64 3
  store i8 0, i8* %tmp169, align 1, !tbaa !13
  %tmp170 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp20, i64 0, i32 0
  store i64 23, i64* %tmp170, align 8, !tbaa !60
  %tmp171 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp20, i64 0, i32 1
  store i8* getelementptr inbounds ([23 x i8], [23 x i8]* @global.23, i64 0, i64 0), i8** %tmp171, align 8, !tbaa !62
  %tmp172 = bitcast <{ i64, i8* }>* %tmp20 to i8*
  %tmp173 = call i32 @for_write_seq_lis_xmit(i8* nonnull %tmp155, i8* nonnull %tmp166, i8* nonnull %tmp172) #2
  %tmp174 = load i32, i32* @global.10, align 8, !tbaa !5
  %tmp175 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp21, i64 0, i64 0
  store i8 9, i8* %tmp175, align 1, !tbaa !13
  %tmp176 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp21, i64 0, i64 1
  store i8 1, i8* %tmp176, align 1, !tbaa !13
  %tmp177 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp21, i64 0, i64 2
  store i8 1, i8* %tmp177, align 1, !tbaa !13
  %tmp178 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp21, i64 0, i64 3
  store i8 0, i8* %tmp178, align 1, !tbaa !13
  %tmp179 = getelementptr inbounds <{ i32 }>, <{ i32 }>* %tmp22, i64 0, i32 0
  store i32 %tmp174, i32* %tmp179, align 8, !tbaa !64
  %tmp180 = bitcast <{ i32 }>* %tmp22 to i8*
  %tmp181 = call i32 @for_write_seq_lis_xmit(i8* nonnull %tmp155, i8* nonnull %tmp175, i8* nonnull %tmp180) #2
  call void (...) @foo(i32* nonnull @global.32, i32* nonnull @global.33, i32* nonnull %tmp1) #2
  br label %bb182

bb182:                                            ; preds = %bb148, %bb112
  %tmp183 = load i32, i32* @global.5, align 8, !tbaa !66
  %tmp184 = load i32, i32* @global.11, align 8, !tbaa !7
  %tmp185 = icmp eq i32 %tmp183, %tmp184
  br i1 %tmp185, label %bb221, label %bb186

bb186:                                            ; preds = %bb182
  %tmp187 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp23, i64 0, i64 0
  store i8 56, i8* %tmp187, align 1, !tbaa !13
  %tmp188 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp23, i64 0, i64 1
  store i8 4, i8* %tmp188, align 1, !tbaa !13
  %tmp189 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp23, i64 0, i64 2
  store i8 2, i8* %tmp189, align 1, !tbaa !13
  %tmp190 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp23, i64 0, i64 3
  store i8 0, i8* %tmp190, align 1, !tbaa !13
  %tmp191 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp24, i64 0, i32 0
  store i64 19, i64* %tmp191, align 8, !tbaa !68
  %tmp192 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp24, i64 0, i32 1
  store i8* getelementptr inbounds ([19 x i8], [19 x i8]* @global.22, i64 0, i64 0), i8** %tmp192, align 8, !tbaa !70
  %tmp193 = bitcast [8 x i64]* %tmp to i8*
  %tmp194 = bitcast <{ i64, i8* }>* %tmp24 to i8*
  %tmp195 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_write_seq_lis(i8* nonnull %tmp193, i32 0, i64 1239157112576, i8* nonnull %tmp187, i8* nonnull %tmp194) #2
  %tmp196 = load i32, i32* @global.5, align 8, !tbaa !66
  %tmp197 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp25, i64 0, i64 0
  store i8 9, i8* %tmp197, align 1, !tbaa !13
  %tmp198 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp25, i64 0, i64 1
  store i8 1, i8* %tmp198, align 1, !tbaa !13
  %tmp199 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp25, i64 0, i64 2
  store i8 2, i8* %tmp199, align 1, !tbaa !13
  %tmp200 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp25, i64 0, i64 3
  store i8 0, i8* %tmp200, align 1, !tbaa !13
  %tmp201 = getelementptr inbounds <{ i32 }>, <{ i32 }>* %tmp26, i64 0, i32 0
  store i32 %tmp196, i32* %tmp201, align 8, !tbaa !72
  %tmp202 = bitcast <{ i32 }>* %tmp26 to i8*
  %tmp203 = call i32 @for_write_seq_lis_xmit(i8* nonnull %tmp193, i8* nonnull %tmp197, i8* nonnull %tmp202) #2
  %tmp204 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp27, i64 0, i64 0
  store i8 56, i8* %tmp204, align 1, !tbaa !13
  %tmp205 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp27, i64 0, i64 1
  store i8 4, i8* %tmp205, align 1, !tbaa !13
  %tmp206 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp27, i64 0, i64 2
  store i8 2, i8* %tmp206, align 1, !tbaa !13
  %tmp207 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp27, i64 0, i64 3
  store i8 0, i8* %tmp207, align 1, !tbaa !13
  %tmp208 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp28, i64 0, i32 0
  store i64 24, i64* %tmp208, align 8, !tbaa !74
  %tmp209 = getelementptr inbounds <{ i64, i8* }>, <{ i64, i8* }>* %tmp28, i64 0, i32 1
  store i8* getelementptr inbounds ([24 x i8], [24 x i8]* @global.21, i64 0, i64 0), i8** %tmp209, align 8, !tbaa !76
  %tmp210 = bitcast <{ i64, i8* }>* %tmp28 to i8*
  %tmp211 = call i32 @for_write_seq_lis_xmit(i8* nonnull %tmp193, i8* nonnull %tmp204, i8* nonnull %tmp210) #2
  %tmp212 = load i32, i32* @global.11, align 8, !tbaa !7
  %tmp213 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp29, i64 0, i64 0
  store i8 9, i8* %tmp213, align 1, !tbaa !13
  %tmp214 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp29, i64 0, i64 1
  store i8 1, i8* %tmp214, align 1, !tbaa !13
  %tmp215 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp29, i64 0, i64 2
  store i8 1, i8* %tmp215, align 1, !tbaa !13
  %tmp216 = getelementptr inbounds [4 x i8], [4 x i8]* %tmp29, i64 0, i64 3
  store i8 0, i8* %tmp216, align 1, !tbaa !13
  %tmp217 = getelementptr inbounds <{ i32 }>, <{ i32 }>* %tmp30, i64 0, i32 0
  store i32 %tmp212, i32* %tmp217, align 8, !tbaa !78
  %tmp218 = bitcast <{ i32 }>* %tmp30 to i8*
  %tmp219 = call i32 @for_write_seq_lis_xmit(i8* nonnull %tmp193, i8* nonnull %tmp213, i8* nonnull %tmp218) #2
  call void (...) @foo(i32* nonnull @global.34, i32* nonnull @global.35, i32* nonnull %tmp1) #2
  %tmp220 = load i32, i32* @global.11, align 8, !tbaa !7
  br label %bb221

bb221:                                            ; preds = %bb186, %bb182
  %tmp222 = phi i32 [ %tmp183, %bb182 ], [ %tmp220, %bb186 ]
  %tmp223 = load i32, i32* @global.4, align 8, !tbaa !80
  %tmp224 = add nsw i32 %tmp223, -1
  %tmp225 = add nsw i32 %tmp224, %tmp222
  %tmp226 = srem i32 %tmp225, %tmp222
  store i32 %tmp226, i32* @global.3, align 8, !tbaa !82
  %tmp227 = add nsw i32 %tmp223, 1
  %tmp228 = srem i32 %tmp227, %tmp222
  store i32 %tmp228, i32* @global.2, align 8, !tbaa !84
  ret void
}

declare void @foo(...) local_unnamed_addr

declare void @barney(...) local_unnamed_addr

declare void @widget(...) local_unnamed_addr

declare void @quux(...) local_unnamed_addr

attributes #0 = { nofree "intel-lang"="fortran" }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve_sign,preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" }
attributes #2 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!0}
!nvvm.annotations = !{}

!0 = !{i32 7, !"openmp", i32 50}
!1 = !{!2, !2, i64 0}
!2 = !{!"ifx$unique_sym$683", !3, i64 0}
!3 = !{!"Generic Fortran Symbol", !4, i64 0}
!4 = !{!"ifx$root$4$set_particle_decomp_"}
!5 = !{!6, !6, i64 0}
!6 = !{!"ifx$unique_sym$684", !3, i64 0}
!7 = !{!8, !8, i64 0}
!8 = !{!"ifx$unique_sym$685", !3, i64 0}
!9 = !{!10, !10, i64 0}
!10 = !{!"ifx$unique_sym$686", !3, i64 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"ifx$unique_sym$688", !3, i64 0}
!13 = !{!3, !3, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"ifx$unique_sym$689", !3, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"ifx$unique_sym$690", !3, i64 0}
!18 = !{!19, !19, i64 0}
!19 = !{!"ifx$unique_sym$692", !3, i64 0}
!20 = !{!21, !21, i64 0}
!21 = !{!"ifx$unique_sym$693", !3, i64 0}
!22 = !{!23, !23, i64 0}
!23 = !{!"ifx$unique_sym$694", !3, i64 0}
!24 = !{!25, !25, i64 0}
!25 = !{!"ifx$unique_sym$696", !3, i64 0}
!26 = !{!27, !27, i64 0}
!27 = !{!"ifx$unique_sym$697", !3, i64 0}
!28 = !{!29, !29, i64 0}
!29 = !{!"ifx$unique_sym$698", !3, i64 0}
!30 = !{!31, !31, i64 0}
!31 = !{!"ifx$unique_sym$700", !3, i64 0}
!32 = !{!33, !33, i64 0}
!33 = !{!"ifx$unique_sym$701", !3, i64 0}
!34 = !{!35, !35, i64 0}
!35 = !{!"ifx$unique_sym$702", !3, i64 0}
!36 = !{!37, !37, i64 0}
!37 = !{!"ifx$unique_sym$703", !3, i64 0}
!38 = !{!39, !39, i64 0}
!39 = !{!"ifx$unique_sym$704", !3, i64 0}
!40 = !{!41, !41, i64 0}
!41 = !{!"ifx$unique_sym$705", !3, i64 0}
!42 = !{!43, !43, i64 0}
!43 = !{!"ifx$unique_sym$706", !3, i64 0}
!44 = !{!45, !45, i64 0}
!45 = !{!"ifx$unique_sym$707", !3, i64 0}
!46 = !{!47, !47, i64 0}
!47 = !{!"ifx$unique_sym$708", !3, i64 0}
!48 = !{!49, !49, i64 0}
!49 = !{!"ifx$unique_sym$709", !3, i64 0}
!50 = !{!51, !51, i64 0}
!51 = !{!"ifx$unique_sym$710", !3, i64 0}
!52 = !{!53, !53, i64 0}
!53 = !{!"ifx$unique_sym$711", !3, i64 0}
!54 = !{!55, !55, i64 0}
!55 = !{!"ifx$unique_sym$713", !3, i64 0}
!56 = !{!57, !57, i64 0}
!57 = !{!"ifx$unique_sym$714", !3, i64 0}
!58 = !{!59, !59, i64 0}
!59 = !{!"ifx$unique_sym$715", !3, i64 0}
!60 = !{!61, !61, i64 0}
!61 = !{!"ifx$unique_sym$717", !3, i64 0}
!62 = !{!63, !63, i64 0}
!63 = !{!"ifx$unique_sym$718", !3, i64 0}
!64 = !{!65, !65, i64 0}
!65 = !{!"ifx$unique_sym$719", !3, i64 0}
!66 = !{!67, !67, i64 0}
!67 = !{!"ifx$unique_sym$720", !3, i64 0}
!68 = !{!69, !69, i64 0}
!69 = !{!"ifx$unique_sym$722", !3, i64 0}
!70 = !{!71, !71, i64 0}
!71 = !{!"ifx$unique_sym$723", !3, i64 0}
!72 = !{!73, !73, i64 0}
!73 = !{!"ifx$unique_sym$724", !3, i64 0}
!74 = !{!75, !75, i64 0}
!75 = !{!"ifx$unique_sym$726", !3, i64 0}
!76 = !{!77, !77, i64 0}
!77 = !{!"ifx$unique_sym$727", !3, i64 0}
!78 = !{!79, !79, i64 0}
!79 = !{!"ifx$unique_sym$728", !3, i64 0}
!80 = !{!81, !81, i64 0}
!81 = !{!"ifx$unique_sym$729", !3, i64 0}
!82 = !{!83, !83, i64 0}
!83 = !{!"ifx$unique_sym$730", !3, i64 0}
!84 = !{!85, !85, i64 0}
!85 = !{!"ifx$unique_sym$731", !3, i64 0}
