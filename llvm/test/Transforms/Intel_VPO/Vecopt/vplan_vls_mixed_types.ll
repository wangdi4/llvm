; RUN: opt -S -VPlanDriver < %s | FileCheck %s
; REQUIRES: asserts

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test_00 = type { i32, float }

define void @test_00(%struct.test_00* nocapture %src, %struct.test_00* nocapture %dst) {
;  for (i = 0; i < 1024; i += 1) {
;    dst[i] = src[i];
;  }
;
; CHECK-LABEL: @test_00(
; CHECK:         [[MM_VECTORGEP:%.*]] = getelementptr inbounds [[STRUCT_TEST_00:%.*]], <4 x %struct.test_00*> [[BROADCAST_SPLAT:%.*]], <4 x i64> [[VEC_PHI:%.*]], <4 x i32> zeroinitializer
; CHECK-NEXT:    [[MM_VECTORGEP1:%.*]] = getelementptr inbounds [[STRUCT_TEST_00]], <4 x %struct.test_00*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI]], <4 x i32> <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[MM_VECTORGEP_0:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPPTR:%.*]] = bitcast i32* [[MM_VECTORGEP_0]] to <8 x i32>*
; CHECK-NEXT:    [[GROUPLOAD:%.*]] = load <8 x i32>, <8 x i32>* [[GROUPPTR]], align 4
; CHECK-NEXT:    [[GROUPSHUFFLE:%.*]] = shufflevector <8 x i32> [[GROUPLOAD]], <8 x i32> undef, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
; CHECK-NEXT:    [[GROUPSHUFFLE2:%.*]] = shufflevector <8 x i32> [[GROUPLOAD]], <8 x i32> undef, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
; CHECK-NEXT:    [[GROUPCAST:%.*]] = bitcast <4 x i32> [[GROUPSHUFFLE2]] to <4 x float>
; CHECK-NEXT:    [[MM_VECTORGEP5:%.*]] = getelementptr inbounds [[STRUCT_TEST_00]], <4 x %struct.test_00*> [[BROADCAST_SPLAT4:%.*]], <4 x i64> [[VEC_PHI]], <4 x i32> zeroinitializer
; CHECK-NEXT:    [[MM_VECTORGEP6:%.*]] = getelementptr inbounds [[STRUCT_TEST_00]], <4 x %struct.test_00*> [[BROADCAST_SPLAT4]], <4 x i64> [[VEC_PHI]], <4 x i32> <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[GROUPCAST7:%.*]] = bitcast <4 x float> [[GROUPCAST]] to <4 x i32>
; CHECK-NEXT:    [[TMP0:%.*]] = shufflevector <4 x i32> [[GROUPSHUFFLE]], <4 x i32> [[GROUPCAST7]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[GROUPSHUFFLE8:%.*]] = shufflevector <8 x i32> [[TMP0]], <8 x i32> undef, <8 x i32> <i32 0, i32 4, i32 1, i32 5, i32 2, i32 6, i32 3, i32 7>
; CHECK-NEXT:    [[MM_VECTORGEP5_0:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP5]], i64 0
; CHECK-NEXT:    [[GROUPPTR9:%.*]] = bitcast i32* [[MM_VECTORGEP5_0]] to <8 x i32>*
; CHECK-NEXT:    store <8 x i32> [[GROUPSHUFFLE8]], <8 x i32>* [[GROUPPTR9]], align 4
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %ptr.src.fst = getelementptr inbounds %struct.test_00, %struct.test_00* %src, i64 %indvars.iv, i32 0
  %ptr.src.snd = getelementptr inbounds %struct.test_00, %struct.test_00* %src, i64 %indvars.iv, i32 1

  %val.fst = load i32, i32* %ptr.src.fst, align 4
  %val.snd = load float, float* %ptr.src.snd, align 4

  %ptr.dst.fst = getelementptr inbounds %struct.test_00, %struct.test_00* %dst, i64 %indvars.iv, i32 0
  %ptr.dst.snd = getelementptr inbounds %struct.test_00, %struct.test_00* %dst, i64 %indvars.iv, i32 1

  store i32 %val.fst, i32* %ptr.dst.fst, align 4
  store float %val.snd, float* %ptr.dst.snd, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

%struct.test_01 = type { i32, float }
define void @test_01(%struct.test_01* nocapture %src, %struct.test_01* nocapture %dst) {
;  for (i = 0; i < 1024; i += 1) {
;    { t0, t1 } = src[i];
;    dst[i] = { t0 + 42, t1 * t1 };
;  }
;
; CHECK-LABEL: @test_01(
; CHECK:         [[MM_VECTORGEP:%.*]] = getelementptr inbounds [[STRUCT_TEST_01:%.*]], <4 x %struct.test_01*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI]], <4 x i32> zeroinitializer
; CHECK-NEXT:    [[MM_VECTORGEP1:%.*]] = getelementptr inbounds [[STRUCT_TEST_01]], <4 x %struct.test_01*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI]], <4 x i32> <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[MM_VECTORGEP_0:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPPTR:%.*]] = bitcast i32* [[MM_VECTORGEP_0]] to <8 x i32>*
; CHECK-NEXT:    [[GROUPLOAD:%.*]] = load <8 x i32>, <8 x i32>* [[GROUPPTR]], align 4
; CHECK-NEXT:    [[GROUPSHUFFLE:%.*]] = shufflevector <8 x i32> [[GROUPLOAD]], <8 x i32> undef, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
; CHECK-NEXT:    [[GROUPSHUFFLE2:%.*]] = shufflevector <8 x i32> [[GROUPLOAD]], <8 x i32> undef, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
; CHECK-NEXT:    [[GROUPCAST:%.*]] = bitcast <4 x i32> [[GROUPSHUFFLE2]] to <4 x float>
; CHECK-NEXT:    [[TMP0:%.*]] = add nuw nsw <4 x i32> [[GROUPSHUFFLE]], <i32 42, i32 42, i32 42, i32 42>
; CHECK-NEXT:    [[TMP1:%.*]] = fmul fast <4 x float> [[GROUPCAST]], [[GROUPCAST]]
; CHECK-NEXT:    [[MM_VECTORGEP5:%.*]] = getelementptr inbounds [[STRUCT_TEST_01]], <4 x %struct.test_01*> [[BROADCAST_SPLAT4]], <4 x i64> [[VEC_PHI]], <4 x i32> zeroinitializer
; CHECK-NEXT:    [[MM_VECTORGEP6:%.*]] = getelementptr inbounds [[STRUCT_TEST_01]], <4 x %struct.test_01*> [[BROADCAST_SPLAT4]], <4 x i64> [[VEC_PHI]], <4 x i32> <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[GROUPCAST7:%.*]] = bitcast <4 x float> [[TMP1]] to <4 x i32>
; CHECK-NEXT:    [[TMP2:%.*]] = shufflevector <4 x i32> [[TMP0]], <4 x i32> [[GROUPCAST7]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[GROUPSHUFFLE8:%.*]] = shufflevector <8 x i32> [[TMP2]], <8 x i32> undef, <8 x i32> <i32 0, i32 4, i32 1, i32 5, i32 2, i32 6, i32 3, i32 7>
; CHECK-NEXT:    [[MM_VECTORGEP5_0:%.*]] = extractelement <4 x i32*> [[MM_VECTORGEP5]], i64 0
; CHECK-NEXT:    [[GROUPPTR9:%.*]] = bitcast i32* [[MM_VECTORGEP5_0]] to <8 x i32>*
; CHECK-NEXT:    store <8 x i32> [[GROUPSHUFFLE8]], <8 x i32>* [[GROUPPTR9]], align 4
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %ptr.src.fst = getelementptr inbounds %struct.test_01, %struct.test_01* %src, i64 %indvars.iv, i32 0
  %ptr.src.snd = getelementptr inbounds %struct.test_01, %struct.test_01* %src, i64 %indvars.iv, i32 1

  %val.src.fst = load i32, i32* %ptr.src.fst, align 4
  %val.src.snd = load float, float* %ptr.src.snd, align 4

  %val.dst.fst = add nuw nsw i32 %val.src.fst, 42
  %val.dst.snd = fmul fast float %val.src.snd, %val.src.snd

  %ptr.dst.fst = getelementptr inbounds %struct.test_01, %struct.test_01* %dst, i64 %indvars.iv, i32 0
  %ptr.dst.snd = getelementptr inbounds %struct.test_01, %struct.test_01* %dst, i64 %indvars.iv, i32 1

  store i32 %val.dst.fst, i32* %ptr.dst.fst, align 4
  store float %val.dst.snd, float* %ptr.dst.snd, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}


%struct.test_02 = type { <2 x i32>, double }
define void @test_02(%struct.test_02* nocapture %src, %struct.test_02* nocapture %dst) {
;  for (i = 0; i < 1024; i += 1) {
;    { t0, t1 } = src[i];
;    dst[i] = { t0 + <11, 17>, t1 * t1 };
;  }
;
; CHECK-LABEL: @test_02(
; CHECK:         [[MM_VECTORGEP:%.*]] = getelementptr inbounds [[STRUCT_TEST_02:%.*]], <4 x %struct.test_02*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI]], <4 x i32> zeroinitializer
; CHECK-NEXT:    [[MM_VECTORGEP1:%.*]] = getelementptr inbounds [[STRUCT_TEST_02]], <4 x %struct.test_02*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI]], <4 x i32> <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[MM_VECTORGEP_0:%.*]] = extractelement <4 x <2 x i32>*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPPTR:%.*]] = bitcast <2 x i32>* [[MM_VECTORGEP_0]] to <16 x i32>*
; CHECK-NEXT:    [[GROUPLOAD:%.*]] = load <16 x i32>, <16 x i32>* [[GROUPPTR]], align 4
; CHECK-NEXT:    [[GROUPSHUFFLE:%.*]] = shufflevector <16 x i32> [[GROUPLOAD]], <16 x i32> undef, <8 x i32> <i32 0, i32 1, i32 4, i32 5, i32 8, i32 9, i32 12, i32 13>
; CHECK-NEXT:    [[GROUPSHUFFLE2:%.*]] = shufflevector <16 x i32> [[GROUPLOAD]], <16 x i32> undef, <8 x i32> <i32 2, i32 3, i32 6, i32 7, i32 10, i32 11, i32 14, i32 15>
; CHECK-NEXT:    [[GROUPCAST:%.*]] = bitcast <8 x i32> [[GROUPSHUFFLE2]] to <4 x double>
; CHECK-NEXT:    [[TMP0:%.*]] = add nuw nsw <8 x i32> [[GROUPSHUFFLE]], <i32 11, i32 17, i32 11, i32 17, i32 11, i32 17, i32 11, i32 17>
; CHECK-NEXT:    [[TMP1:%.*]] = fmul fast <4 x double> [[GROUPCAST]], [[GROUPCAST]]
; CHECK-NEXT:    [[MM_VECTORGEP5:%.*]] = getelementptr inbounds [[STRUCT_TEST_02]], <4 x %struct.test_02*> [[BROADCAST_SPLAT4]], <4 x i64> [[VEC_PHI]], <4 x i32> zeroinitializer
; CHECK-NEXT:    [[MM_VECTORGEP6:%.*]] = getelementptr inbounds [[STRUCT_TEST_02]], <4 x %struct.test_02*> [[BROADCAST_SPLAT4]], <4 x i64> [[VEC_PHI]], <4 x i32> <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[GROUPCAST7:%.*]] = bitcast <4 x double> [[TMP1]] to <8 x i32>
; CHECK-NEXT:    [[TMP2:%.*]] = shufflevector <8 x i32> [[TMP0]], <8 x i32> [[GROUPCAST7]], <16 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15>
; CHECK-NEXT:    [[GROUPSHUFFLE8:%.*]] = shufflevector <16 x i32> [[TMP2]], <16 x i32> undef, <16 x i32> <i32 0, i32 1, i32 8, i32 9, i32 2, i32 3, i32 10, i32 11, i32 4, i32 5, i32 12, i32 13, i32 6, i32 7, i32 14, i32 15>
; CHECK-NEXT:    [[MM_VECTORGEP5_0:%.*]] = extractelement <4 x <2 x i32>*> [[MM_VECTORGEP5]], i64 0
; CHECK-NEXT:    [[GROUPPTR9:%.*]] = bitcast <2 x i32>* [[MM_VECTORGEP5_0]] to <16 x i32>*
; CHECK-NEXT:    store <16 x i32> [[GROUPSHUFFLE8]], <16 x i32>* [[GROUPPTR9]], align 4
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %ptr.src.fst = getelementptr inbounds %struct.test_02, %struct.test_02* %src, i64 %indvars.iv, i32 0
  %ptr.src.snd = getelementptr inbounds %struct.test_02, %struct.test_02* %src, i64 %indvars.iv, i32 1

  %val.src.fst = load <2 x i32>, <2 x i32>* %ptr.src.fst, align 4
  %val.src.snd = load double, double* %ptr.src.snd, align 4

  %val.dst.fst = add nuw nsw <2 x i32> %val.src.fst, <i32 11, i32 17>
  %val.dst.snd = fmul fast double %val.src.snd, %val.src.snd

  %ptr.dst.fst = getelementptr inbounds %struct.test_02, %struct.test_02* %dst, i64 %indvars.iv, i32 0
  %ptr.dst.snd = getelementptr inbounds %struct.test_02, %struct.test_02* %dst, i64 %indvars.iv, i32 1

  store <2 x i32> %val.dst.fst, <2 x i32>* %ptr.dst.fst, align 4
  store double %val.dst.snd, double* %ptr.dst.snd, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}


%struct.test_03 = type { double, <2 x i32> }
define void @test_03(%struct.test_03* nocapture %src, %struct.test_03* nocapture %dst) {
;  for (i = 0; i < 1024; i += 1) {
;    { t0, t1 } = src[i];
;    dst[i] = { t0 * t0, t1 + <11, 17> };
;  }
;
; CHECK-LABEL: @test_03(
; CHECK:         [[MM_VECTORGEP:%.*]] = getelementptr inbounds [[STRUCT_TEST_03:%.*]], <4 x %struct.test_03*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI]], <4 x i32> zeroinitializer
; CHECK-NEXT:    [[MM_VECTORGEP1:%.*]] = getelementptr inbounds [[STRUCT_TEST_03]], <4 x %struct.test_03*> [[BROADCAST_SPLAT]], <4 x i64> [[VEC_PHI]], <4 x i32> <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[MM_VECTORGEP_0:%.*]] = extractelement <4 x double*> [[MM_VECTORGEP]], i64 0
; CHECK-NEXT:    [[GROUPPTR:%.*]] = bitcast double* [[MM_VECTORGEP_0]] to <8 x double>*
; CHECK-NEXT:    [[GROUPLOAD:%.*]] = load <8 x double>, <8 x double>* [[GROUPPTR]], align 4
; CHECK-NEXT:    [[GROUPSHUFFLE:%.*]] = shufflevector <8 x double> [[GROUPLOAD]], <8 x double> undef, <4 x i32> <i32 0, i32 2, i32 4, i32 6>
; CHECK-NEXT:    [[GROUPSHUFFLE2:%.*]] = shufflevector <8 x double> [[GROUPLOAD]], <8 x double> undef, <4 x i32> <i32 1, i32 3, i32 5, i32 7>
; CHECK-NEXT:    [[GROUPCAST:%.*]] = bitcast <4 x double> [[GROUPSHUFFLE2]] to <8 x i32>
; CHECK-NEXT:    [[TMP0:%.*]] = fmul fast <4 x double> [[GROUPSHUFFLE]], [[GROUPSHUFFLE]]
; CHECK-NEXT:    [[TMP1:%.*]] = add nuw nsw <8 x i32> [[GROUPCAST]], <i32 11, i32 17, i32 11, i32 17, i32 11, i32 17, i32 11, i32 17>
; CHECK-NEXT:    [[MM_VECTORGEP5:%.*]] = getelementptr inbounds [[STRUCT_TEST_03]], <4 x %struct.test_03*> [[BROADCAST_SPLAT4]], <4 x i64> [[VEC_PHI]], <4 x i32> zeroinitializer
; CHECK-NEXT:    [[MM_VECTORGEP6:%.*]] = getelementptr inbounds [[STRUCT_TEST_03]], <4 x %struct.test_03*> [[BROADCAST_SPLAT4]], <4 x i64> [[VEC_PHI]], <4 x i32> <i32 1, i32 1, i32 1, i32 1>
; CHECK-NEXT:    [[GROUPCAST7:%.*]] = bitcast <8 x i32> [[TMP1]] to <4 x double>
; CHECK-NEXT:    [[TMP2:%.*]] = shufflevector <4 x double> [[TMP0]], <4 x double> [[GROUPCAST7]], <8 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7>
; CHECK-NEXT:    [[GROUPSHUFFLE8:%.*]] = shufflevector <8 x double> [[TMP2]], <8 x double> undef, <8 x i32> <i32 0, i32 4, i32 1, i32 5, i32 2, i32 6, i32 3, i32 7>
; CHECK-NEXT:    [[MM_VECTORGEP5_0:%.*]] = extractelement <4 x double*> [[MM_VECTORGEP5]], i64 0
; CHECK-NEXT:    [[GROUPPTR9:%.*]] = bitcast double* [[MM_VECTORGEP5_0]] to <8 x double>*
; CHECK-NEXT:    store <8 x double> [[GROUPSHUFFLE8]], <8 x double>* [[GROUPPTR9]], align 4
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]

  %ptr.src.fst = getelementptr inbounds %struct.test_03, %struct.test_03* %src, i64 %indvars.iv, i32 0
  %ptr.src.snd = getelementptr inbounds %struct.test_03, %struct.test_03* %src, i64 %indvars.iv, i32 1

  %val.src.fst = load double, double* %ptr.src.fst, align 4
  %val.src.snd = load <2 x i32>, <2 x i32>* %ptr.src.snd, align 4

  %val.dst.fst = fmul fast double %val.src.fst, %val.src.fst
  %val.dst.snd = add nuw nsw <2 x i32> %val.src.snd, <i32 11, i32 17>

  %ptr.dst.fst = getelementptr inbounds %struct.test_03, %struct.test_03* %dst, i64 %indvars.iv, i32 0
  %ptr.dst.snd = getelementptr inbounds %struct.test_03, %struct.test_03* %dst, i64 %indvars.iv, i32 1

  store double %val.dst.fst, double* %ptr.dst.fst, align 4
  store <2 x i32> %val.dst.snd, <2 x i32>* %ptr.dst.snd, align 4

  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1024
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
