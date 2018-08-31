; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=true -addsub-reassoc-simplify-chains=true -addsub-reassoc-reuse-chain=true -addsub-reassoc-memcan-enable-unary-associations=true -addsub-reassoc-unshare-leaves=true -addsub-reassoc-canonicalize-group=true -S | FileCheck %s -check-prefix=CHECK_CANON_GROUP
; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=true -addsub-reassoc-simplify-chains=true -addsub-reassoc-reuse-chain=true -addsub-reassoc-memcan-enable-unary-associations=true -addsub-reassoc-unshare-leaves=true -addsub-reassoc-canonicalize-group=false -S | FileCheck %s -check-prefix=CHECK_UNSHARE
; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=true -addsub-reassoc-simplify-chains=true -addsub-reassoc-reuse-chain=true -addsub-reassoc-memcan-enable-unary-associations=true -addsub-reassoc-unshare-leaves=false -addsub-reassoc-canonicalize-group=false -S | FileCheck %s -check-prefix=CHECK_UNARY_ASSOC
; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=true -addsub-reassoc-simplify-chains=true -addsub-reassoc-reuse-chain=true -addsub-reassoc-memcan-enable-unary-associations=false -addsub-reassoc-unshare-leaves=false -addsub-reassoc-canonicalize-group=false -S | FileCheck %s -check-prefix=CHECK_CHAIN_REUSE
; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=true -addsub-reassoc-simplify-chains=true -addsub-reassoc-reuse-chain=false -addsub-reassoc-memcan-enable-unary-associations=false -addsub-reassoc-unshare-leaves=false -addsub-reassoc-canonicalize-group=false -S | FileCheck %s -check-prefix=CHECK_SIMP
; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=false -addsub-reassoc-simplify-chains=false -addsub-reassoc-reuse-chain=false -addsub-reassoc-memcan-enable-unary-associations=false -addsub-reassoc-unshare-leaves=false -addsub-reassoc-canonicalize-group=false -S | FileCheck %s -check-prefix=CHECK

; This is a test for AddSubReassoc pass to check that it kicks in for satd_16x16 like pattern.

; CHECK_CANON_GROUP: [[Chain_T24_187:%.*]] = sub i32 %44, %28
; CHECK_CANON_GROUP: [[Chain_T24_185:%.*]] = sub i32 [[Chain_T24_187]], %40
; CHECK_CANON_GROUP: [[Chain1_3:%.*]] = add i32 [[Chain_T24_185]], %16
; CHECK_CANON_GROUP: [[Chain_T24_174178:%.*]] = sub i32 %45, %29
; CHECK_CANON_GROUP: [[Chain_T24_173177:%.*]] = sub i32 [[Chain_T24_174178]], %41
; CHECK_CANON_GROUP: [[Chain2_3:%.*]] = add i32 [[Chain_T24_173177]], %17
; CHECK_CANON_GROUP: [[Chain_T24_162166:%.*]] = sub i32 %46, %30
; CHECK_CANON_GROUP: [[Chain_T24_161165:%.*]] = sub i32 [[Chain_T24_162166]], %42
; CHECK_CANON_GROUP: [[Chain3_3:%.*]] = add i32 [[Chain_T24_161165]], %18
; CHECK_CANON_GROUP: [[Chain_T24_158:%.*]] = sub i32 %47, %31
; CHECK_CANON_GROUP: [[Chain_T24_156:%.*]] = sub i32 [[Chain_T24_158]], %43
; CHECK_CANON_GROUP: [[Chain4_3:%.*]] = add i32 [[Chain_T24_156]], %19
; CHECK_CANON_GROUP: [[Bridge1_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_CANON_GROUP: [[Bridge1_2:%.*]] = add i32 [[Bridge1_1]], [[Chain2_3]]
; CHECK_CANON_GROUP: [[Bridge1_3:%.*]] = add i32 [[Bridge1_2]], [[Chain1_3]]
; CHECK_CANON_GROUP: store i32 [[Bridge1_3]]
; CHECK_CANON_GROUP: [[Bridge2_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_CANON_GROUP: [[Bridge2_2:%.*]] = sub i32 [[Bridge2_1]], [[Chain2_3]]
; CHECK_CANON_GROUP: [[Bridge2_3:%.*]] = sub i32 [[Bridge2_2]], [[Chain1_3]]
; CHECK_CANON_GROUP: store i32 [[Bridge2_3]]
; CHECK_CANON_GROUP: [[Bridge3_1:%.*]] = sub i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_CANON_GROUP: [[Bridge3_2:%.*]] = add i32 [[Bridge3_1]], [[Chain2_3]]
; CHECK_CANON_GROUP: [[Bridge3_3:%.*]] = sub i32 [[Bridge3_2]], [[Chain1_3]]
; CHECK_CANON_GROUP: store i32 [[Bridge3_3]]
; CHECK_CANON_GROUP: [[Bridge4_1:%.*]] = sub i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_CANON_GROUP: [[Bridge4_2:%.*]] = sub i32 [[Bridge4_1]], [[Chain2_3]]
; CHECK_CANON_GROUP: [[Bridge4_3:%.*]] = add i32 [[Bridge4_2]], [[Chain1_3]]
; CHECK_CANON_GROUP: store i32 [[Bridge4_3]]

; CHECK_UNSHARE: [[Chain_T24_187:%.*]] = sub i32 [[l44:%.*]], [[l40:%.*]]
; CHECK_UNSHARE: [[Chain_T24_185:%.*]] = add i32 [[Chain_T24_187]], [[l16:%.*]]
; CHECK_UNSHARE: [[Chain_T1_3:%.*]] = sub i32 [[Chain_T24_185]], [[l28:%.*]]
; CHECK_UNSHARE: [[Chain_T24_174178:%.*]] = sub i32 [[l41:%.*]], [[l45:%.*]]
; CHECK_UNSHARE: [[Chain_T24_173177:%.*]] = sub i32 [[Chain_T24_174178]], [[l17:%.*]]
; CHECK_UNSHARE: [[Chain_T2_3:%.*]] = add i32 [[Chain_T24_173177]], [[l29:%.*]]
; CHECK_UNSHARE: [[Chain_T24_162166:%.*]] = sub i32 [[l42:%.*]], [[l46:%.*]]
; CHECK_UNSHARE: [[Chain_T24_161165:%.*]] = sub i32 [[Chain_T24_162166]], [[l18:%.*]]
; CHECK_UNSHARE: [[Chain_T3_3:%.*]] = add i32 [[Chain_T24_161165]], [[l30:%.*]]
; CHECK_UNSHARE: [[Chain_T24_158:%.*]] = sub i32 [[l47:%.*]], [[l43:%.*]]
; CHECK_UNSHARE: [[Chain_T24_156:%.*]] = add i32 [[Chain_T24_158]], [[l19:%.*]]
; CHECK_UNSHARE: [[Chain_T4_3_:%.*]] = sub i32 [[Chain_T24_156]], [[l31:%.*]]
; CHECK_UNSHARE: [[Bridge1_1:%.*]] = sub i32 [[Chain4_3:%.*]], [[Chain3_3:%.*]]
; CHECK_UNSHARE: [[Bridge1_2:%.*]] = sub i32 [[Bridge1_1]], [[Chain2_3:%.*]]
; CHECK_UNSHARE: [[Bridge1_3:%.*]] = add i32 [[Bridge1_2]], [[Chain1_3:%.*]]
; CHECK_UNSHARE: store i32 [[Bridge1_3]]
; CHECK_UNSHARE: [[Bridge2_1:%.*]] = sub i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_UNSHARE: [[Bridge2_2:%.*]] = add i32 [[Bridge2_1]], [[Chain2_3]]
; CHECK_UNSHARE: [[Bridge2_3:%.*]] = sub i32 [[Bridge2_2]], [[Chain1_3]]
; CHECK_UNSHARE: store i32 [[Bridge2_3]]
; CHECK_UNSHARE: [[Bridge3_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_UNSHARE: [[Bridge3_2:%.*]] = sub i32 [[Bridge3_1]], [[Chain2_3]]
; CHECK_UNSHARE: [[Bridge3_3:%.*]] = sub i32 [[Bridge3_2]], [[Chain1_3]]
; CHECK_UNSHARE: store i32 [[Bridge3_3]]
; CHECK_UNSHARE: [[Bridge4_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_UNSHARE: [[Bridge4_2:%.*]] = add i32 [[Bridge4_1]], [[Chain2_3]]
; CHECK_UNSHARE: [[Bridge4_3:%.*]] = add i32 [[Bridge4_2]], [[Chain1_3]]
; CHECK_UNSHARE: store i32 [[Bridge4_3]]

; CHECK_UNARY_ASSOC: [[Chain1_1:%.*]] = sub i32 [[l16:%.*]], [[l24:%.*]]
; CHECK_UNARY_ASSOC: [[Chain1_2:%.*]] = sub i32 [[Chain1_1]], [[l35:%.*]]
; CHECK_UNARY_ASSOC: [[Chain1_3:%.*]] = add i32 [[Chain1_2]], [[l44:%.*]]
; CHECK_UNARY_ASSOC: [[Chain2_1:%.*]] = sub i32 [[l27:%.*]], [[l17:%.*]]
; CHECK_UNARY_ASSOC: [[Chain2_2:%.*]] = add i32 [[Chain2_1]], [[l29:%.*]]
; CHECK_UNARY_ASSOC: [[Chain2_3:%.*]] = sub i32 [[Chain2_2]], [[l35:%.*]]
; CHECK_UNARY_ASSOC: [[Chain3_1:%.*]] = sub i32 [[l30:%.*]], [[l19:%.*]]
; CHECK_UNARY_ASSOC: [[Chain3_2:%.*]] = add i32 [[Chain3_1]], [[l30:%.*]]
; CHECK_UNARY_ASSOC: [[Chain3_3:%.*]] = sub i32 [[Chain3_2]], [[l37:%.*]]
; CHECK_UNARY_ASSOC: [[Chain4_1:%.*]] = sub i32 [[l21:%.*]], [[l33:%.*]]
; CHECK_UNARY_ASSOC: [[Chain4_2:%.*]] = sub i32 [[Chain4_1]], [[l31:%.*]]
; CHECK_UNARY_ASSOC: [[Chain4_3:%.*]] = add i32 [[Chain4_2]], [[l39:%.*]]
; CHECK_UNARY_ASSOC: [[Bridge1_1:%.*]] = sub i32 [[Chain4_3:%.*]], [[Chain3_3:%.*]]
; CHECK_UNARY_ASSOC: [[Bridge1_2:%.*]] = sub i32 [[Bridge1_1]], [[Chain2_3:%.*]]
; CHECK_UNARY_ASSOC: [[Bridge1_3:%.*]] = add i32 [[Bridge1_2]], [[Chain1_3:%.*]]
; CHECK_UNARY_ASSOC: store i32 [[Bridge1_3]]
; CHECK_UNARY_ASSOC: [[Bridge2_1:%.*]] = sub i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_UNARY_ASSOC: [[Bridge2_2:%.*]] = add i32 [[Bridge2_1]], [[Chain2_3]]
; CHECK_UNARY_ASSOC: [[Bridge2_3:%.*]] = sub i32 [[Bridge2_2]], [[Chain1_3]]
; CHECK_UNARY_ASSOC: store i32 [[Bridge2_3]]
; CHECK_UNARY_ASSOC: [[Bridge3_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_UNARY_ASSOC: [[Bridge3_2:%.*]] = sub i32 [[Bridge3_1]], [[Chain2_3]]
; CHECK_UNARY_ASSOC: [[Bridge3_3:%.*]] = sub i32 [[Bridge3_2]], [[Chain1_3]]
; CHECK_UNARY_ASSOC: store i32 [[Bridge3_3]]
; CHECK_UNARY_ASSOC: [[Bridge4_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_UNARY_ASSOC: [[Bridge4_2:%.*]] = add i32 [[Bridge4_1]], [[Chain2_3]]
; CHECK_UNARY_ASSOC: [[Bridge4_3:%.*]] = add i32 [[Bridge4_2]], [[Chain1_3]]
; CHECK_UNARY_ASSOC: store i32 [[Bridge4_3]]

; CHECK_CHAIN_REUSE: [[Chain1_1:%.*]] = sub i32 [[l16:%.*]], [[l24:%.*]]
; CHECK_CHAIN_REUSE: [[Chain1_2:%.*]] = sub i32 [[Chain1_1]], [[l35:%.*]]
; CHECK_CHAIN_REUSE: [[Chain1_3:%.*]] = add i32 [[Chain1_2]], [[l44:%.*]]
; CHECK_CHAIN_REUSE: [[Chain2_1:%.*]] = sub i32 [[l27:%.*]], [[l17:%.*]]
; CHECK_CHAIN_REUSE: [[Chain2_2:%.*]] = add i32 [[Chain2_1]], [[l29:%.*]]
; CHECK_CHAIN_REUSE: [[Chain2_3:%.*]] = sub i32 [[Chain2_2]], [[l35:%.*]]
; CHECK_CHAIN_REUSE: [[Chain3_1:%.*]] = sub i32 [[l30:%.*]], [[l19:%.*]]
; CHECK_CHAIN_REUSE: [[Chain3_2:%.*]] = add i32 [[Chain3_1]], [[l30:%.*]]
; CHECK_CHAIN_REUSE: [[Chain3_3:%.*]] = sub i32 [[Chain3_2]], [[l37:%.*]]
; CHECK_CHAIN_REUSE: [[Chain4_1:%.*]] = sub i32 [[l21:%.*]], [[l33:%.*]]
; CHECK_CHAIN_REUSE: [[Chain4_2:%.*]] = sub i32 [[Chain4_1]], [[l31:%.*]]
; CHECK_CHAIN_REUSE: [[Chain4_3:%.*]] = add i32 [[Chain4_2]], [[l39:%.*]]
; CHECK_CHAIN_REUSE: [[Bridge1_1:%.*]] = sub i32 [[Chain4_3:%.*]], [[Chain3_3:%.*]]
; CHECK_CHAIN_REUSE: [[Bridge1_2:%.*]] = sub i32 [[Bridge1_1]], [[Chain2_3:%.*]]
; CHECK_CHAIN_REUSE: [[Bridge1_3:%.*]] = add i32 [[Bridge1_2]], [[Chain1_3:%.*]]
; CHECK_CHAIN_REUSE: store i32 [[Bridge1_3]]
; CHECK_CHAIN_REUSE: [[Bridge2_1:%.*]] = sub i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_CHAIN_REUSE: [[Bridge2_2:%.*]] = add i32 [[Bridge2_1]], [[Chain2_3]]
; CHECK_CHAIN_REUSE: [[Bridge2_3:%.*]] = sub i32 [[Bridge2_2]], [[Chain1_3]]
; CHECK_CHAIN_REUSE: store i32 [[Bridge2_3]]
; CHECK_CHAIN_REUSE: [[Bridge3_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_CHAIN_REUSE: [[Bridge3_2:%.*]] = sub i32 [[Bridge3_1]], [[Chain2_3]]
; CHECK_CHAIN_REUSE: [[Bridge3_3:%.*]] = sub i32 [[Bridge3_2]], [[Chain1_3]]
; CHECK_CHAIN_REUSE: store i32 [[Bridge3_3]]
; CHECK_CHAIN_REUSE: [[Bridge4_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_CHAIN_REUSE: [[Bridge4_2:%.*]] = add i32 [[Bridge4_1]], [[Chain2_3]]
; CHECK_CHAIN_REUSE: [[Bridge4_3:%.*]] = add i32 [[Bridge4_2]], [[Chain1_3]]
; CHECK_CHAIN_REUSE: store i32 [[Bridge4_3]]

; CHECK_SIMP: [[Chain1_1:%.*]] = sub i32 [[l16:%.*]], [[l24:%.*]]
; CHECK_SIMP: [[Chain1_2:%.*]] = sub i32 [[Chain1_1]], [[l35:%.*]]
; CHECK_SIMP: [[Chain1_3:%.*]] = add i32 [[Chain1_2]], [[l44:%.*]]
; CHECK_SIMP: [[Chain2_1:%.*]] = sub i32 [[l27:%.*]], [[l17:%.*]]
; CHECK_SIMP: [[Chain2_2:%.*]] = add i32 [[Chain2_1]], [[l29:%.*]]
; CHECK_SIMP: [[Chain2_3:%.*]] = sub i32 [[Chain2_2]], [[l35:%.*]]
; CHECK_SIMP: [[Chain3_1:%.*]] = sub i32 [[l30:%.*]], [[l19:%.*]]
; CHECK_SIMP: [[Chain3_2:%.*]] = add i32 [[Chain3_1]], [[l30:%.*]]
; CHECK_SIMP: [[Chain3_3:%.*]] = sub i32 [[Chain3_2]], [[l37:%.*]]
; CHECK_SIMP: [[Chain4_1:%.*]] = sub i32 [[l21:%.*]], [[l33:%.*]]
; CHECK_SIMP: [[Chain4_2:%.*]] = sub i32 [[Chain4_1]], [[l31:%.*]]
; CHECK_SIMP: [[Chain4_3:%.*]] = add i32 [[Chain4_2]], [[l39:%.*]]
; CHECK_SIMP: [[Bridge1:%.*]] = sub i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_SIMP: [[Bridge2:%.*]] = sub i32 [[Bridge1]], [[Chain2_3]]
; CHECK_SIMP: [[Bridge3:%.*]] = add i32 [[Bridge2]], [[Chain1_3]]
; CHECK_SIMP: store i32 [[Bridge3]]

; CHECK: [[Chain1_0:%.*]] = add i32 0, [[l16:%.*]]
; CHECK: [[Chain1_1:%.*]] = sub i32 [[Chain1_0]], [[l21:%.*]]
; CHECK: [[Chain1_2:%.*]] = sub i32 [[Chain1_1]], [[l28:%.*]]
; CHECK: [[Chain1_3:%.*]] = add i32 [[Chain1_2]], [[l33:%.*]]
; CHECK: [[Chain2_0:%.*]] = sub i32 0, [[l17:%.*]]
; CHECK: [[Chain2_1:%.*]] = add i32 [[Chain2_0]], [[l23:%.*]]
; CHECK: [[Chain2_2:%.*]] = add i32 [[Chain2_1]], [[l29:%.*]]
; CHECK: [[Chain2_3:%.*]] = sub i32 [[Chain2_2]], [[l35:%.*]]
; CHECK: [[Chain3_0:%.*]] = sub i32 0, [[l18:%.*]]
; CHECK: [[Chain3_1:%.*]] = add i32 [[Chain3_0]], [[l25:%.*]]
; CHECK: [[Chain3_2:%.*]] = add i32 [[Chain3_1]], [[l30:%.*]]
; CHECK: [[Chain3_3:%.*]] = sub i32 [[Chain3_2]], [[l37:%.*]]
; CHECK: [[Chain4_0:%.*]] = add i32 0, [[l19:%.*]]
; CHECK: [[Chain4_1:%.*]] = sub i32 [[Chain4_0]], [[l27:%.*]]
; CHECK: [[Chain4_2:%.*]] = sub i32 [[Chain4_1]], [[l31:%.*]]
; CHECK: [[Chain4_3:%.*]] = add i32 [[Chain4_2]], [[l39:%.*]]
; CHECK: [[Bridge0:%.*]] = add i32 0, [[Chain4_3]]
; CHECK: [[Bridge1:%.*]] = sub i32 [[Bridge0]], [[Chain3_3]]
; CHECK: [[Bridge2:%.*]] = sub i32 [[Bridge1]], [[Chain2_3]]
; CHECK: [[Bridge3:%.*]] = add i32 [[Bridge2]], [[Chain1_3]]
; CHECK: store i32 [[Bridge3]]

define dso_local i32 @x264_pixel_satd_16x16(i8* nocapture readonly %pix1, i32 %i_pix1, i8* nocapture readonly %pix2, i32 %i_pix2) #2 {
entry:
  %idx.ext.i = sext i32 %i_pix1 to i64
  %idx.ext63.i = sext i32 %i_pix2 to i64
  %alloca = alloca [16 x [8 x i32]], align 4
  %alloca933 = alloca [8 x i32], align 16
  %alloca934 = alloca [8 x i32], align 16
  %alloca935 = alloca [8 x i32], align 16
  %alloca936 = alloca [8 x i32], align 16
  br label %loop.1247

loop.1247:                                        ; preds = %loop.1247, %entry
  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.1247, %loop.1247 ]
  %0 = mul i64 %i1.i64.0, %idx.ext.i
  %arrayIdx = getelementptr inbounds i8, i8* %pix1, i64 %0
  %gepload = load i8, i8* %arrayIdx, align 1, !tbaa !21
  %1 = mul i64 %i1.i64.0, %idx.ext63.i
  %arrayIdx1013 = getelementptr inbounds i8, i8* %pix2, i64 %1
  %gepload1014 = load i8, i8* %arrayIdx1013, align 1, !tbaa !21
  %2 = add i64 %0, 4
  %arrayIdx1015 = getelementptr inbounds i8, i8* %pix1, i64 %2
  %gepload1016 = load i8, i8* %arrayIdx1015, align 1, !tbaa !21
  %3 = add i64 %1, 4
  %arrayIdx1017 = getelementptr inbounds i8, i8* %pix2, i64 %3
  %gepload1018 = load i8, i8* %arrayIdx1017, align 1, !tbaa !21
  %4 = add i64 %0, 1
  %arrayIdx1019 = getelementptr inbounds i8, i8* %pix1, i64 %4
  %gepload1020 = load i8, i8* %arrayIdx1019, align 1, !tbaa !21
  %5 = add i64 %1, 1
  %arrayIdx1021 = getelementptr inbounds i8, i8* %pix2, i64 %5
  %gepload1022 = load i8, i8* %arrayIdx1021, align 1, !tbaa !21
  %6 = add i64 %0, 5
  %arrayIdx1023 = getelementptr inbounds i8, i8* %pix1, i64 %6
  %gepload1024 = load i8, i8* %arrayIdx1023, align 1, !tbaa !21
  %7 = add i64 %1, 5
  %arrayIdx1025 = getelementptr inbounds i8, i8* %pix2, i64 %7
  %gepload1026 = load i8, i8* %arrayIdx1025, align 1, !tbaa !21
  %8 = add i64 %0, 2
  %arrayIdx1027 = getelementptr inbounds i8, i8* %pix1, i64 %8
  %gepload1028 = load i8, i8* %arrayIdx1027, align 1, !tbaa !21
  %9 = add i64 %1, 2
  %arrayIdx1029 = getelementptr inbounds i8, i8* %pix2, i64 %9
  %gepload1030 = load i8, i8* %arrayIdx1029, align 1, !tbaa !21
  %10 = add i64 %0, 6
  %arrayIdx1031 = getelementptr inbounds i8, i8* %pix1, i64 %10
  %gepload1032 = load i8, i8* %arrayIdx1031, align 1, !tbaa !21
  %11 = add i64 %1, 6
  %arrayIdx1033 = getelementptr inbounds i8, i8* %pix2, i64 %11
  %gepload1034 = load i8, i8* %arrayIdx1033, align 1, !tbaa !21
  %12 = add i64 %0, 3
  %arrayIdx1035 = getelementptr inbounds i8, i8* %pix1, i64 %12
  %gepload1036 = load i8, i8* %arrayIdx1035, align 1, !tbaa !21
  %13 = add i64 %1, 3
  %arrayIdx1037 = getelementptr inbounds i8, i8* %pix2, i64 %13
  %gepload1038 = load i8, i8* %arrayIdx1037, align 1, !tbaa !21
  %14 = add i64 %0, 7
  %arrayIdx1039 = getelementptr inbounds i8, i8* %pix1, i64 %14
  %gepload1040 = load i8, i8* %arrayIdx1039, align 1, !tbaa !21
  %15 = add i64 %1, 7
  %arrayIdx1041 = getelementptr inbounds i8, i8* %pix2, i64 %15
  %gepload1042 = load i8, i8* %arrayIdx1041, align 1, !tbaa !21
  %arrayIdx1043 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 0
  %16 = zext i8 %gepload1036 to i32
  %17 = zext i8 %gepload1028 to i32
  %18 = add nuw nsw i32 %16, %17
  %19 = zext i8 %gepload1020 to i32
  %20 = add nuw nsw i32 %18, %19
  %21 = zext i8 %gepload to i32
  %22 = add nuw nsw i32 %20, %21
  %23 = zext i8 %gepload1042 to i32
  %24 = shl nuw nsw i32 %23, 16
  %25 = sub nsw i32 %22, %24
  %26 = zext i8 %gepload1034 to i32
  %27 = shl nuw nsw i32 %26, 16
  %28 = sub nsw i32 %25, %27
  %29 = zext i8 %gepload1026 to i32
  %30 = shl nuw nsw i32 %29, 16
  %31 = sub nsw i32 %28, %30
  %32 = zext i8 %gepload1018 to i32
  %33 = shl nuw nsw i32 %32, 16
  %34 = sub i32 %31, %33
  %35 = zext i8 %gepload1038 to i32
  %36 = sub i32 %34, %35
  %37 = zext i8 %gepload1030 to i32
  %38 = sub i32 %36, %37
  %39 = zext i8 %gepload1022 to i32
  %40 = sub i32 %38, %39
  %41 = zext i8 %gepload1014 to i32
  %42 = sub i32 %40, %41
  %43 = zext i8 %gepload1040 to i32
  %44 = shl nuw nsw i32 %43, 16
  %45 = add i32 %42, %44
  %46 = zext i8 %gepload1032 to i32
  %47 = shl nuw nsw i32 %46, 16
  %48 = add i32 %45, %47
  %49 = zext i8 %gepload1024 to i32
  %50 = shl nuw nsw i32 %49, 16
  %51 = add i32 %48, %50
  %52 = zext i8 %gepload1016 to i32
  %53 = shl nuw nsw i32 %52, 16
  %54 = add i32 %51, %53
  store i32 %54, i32* %arrayIdx1043, align 4
  %arrayIdx1045 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 2
  %55 = add nuw nsw i32 %16, %17
  %56 = sub nsw i32 %19, %55
  %57 = add nsw i32 %56, %21
  %58 = add nsw i32 %57, %24
  %59 = add nsw i32 %58, %27
  %60 = sub nsw i32 %59, %30
  %61 = sub i32 %60, %33
  %62 = add i32 %61, %35
  %63 = add i32 %62, %37
  %64 = sub i32 %63, %39
  %65 = sub i32 %64, %41
  %66 = sub i32 %65, %44
  %67 = sub i32 %66, %47
  %68 = add i32 %67, %50
  %69 = add i32 %68, %53
  store i32 %69, i32* %arrayIdx1045, align 4
  %arrayIdx1063 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 1
  %70 = sub nsw i32 %17, %16
  %71 = sub nsw i32 %70, %19
  %72 = add nsw i32 %71, %21
  %73 = add nsw i32 %72, %24
  %74 = sub nsw i32 %73, %27
  %75 = add nsw i32 %74, %30
  %76 = sub i32 %75, %33
  %77 = add i32 %76, %35
  %78 = sub i32 %77, %37
  %79 = add i32 %78, %39
  %80 = sub i32 %79, %41
  %81 = sub i32 %80, %44
  %82 = add i32 %81, %47
  %83 = sub i32 %82, %50
  %84 = add i32 %83, %53
  store i32 %84, i32* %arrayIdx1063, align 4
  %arrayIdx1081 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 3
  %85 = sub nsw i32 %16, %17
  %86 = sub nsw i32 %85, %19
  %87 = add nsw i32 %86, %21
  %88 = sub nsw i32 %87, %24
  %89 = add nsw i32 %88, %27
  %90 = add nsw i32 %89, %30
  %91 = sub i32 %90, %33
  %92 = sub i32 %91, %35
  %93 = add i32 %92, %37
  %94 = add i32 %93, %39
  %95 = sub i32 %94, %41
  %96 = add i32 %95, %44
  %97 = sub i32 %96, %47
  %98 = sub i32 %97, %50
  %99 = add i32 %98, %53
  store i32 %99, i32* %arrayIdx1081, align 4
  %100 = add i64 %0, 8
  %arrayIdx1098 = getelementptr inbounds i8, i8* %pix1, i64 %100
  %gepload1099 = load i8, i8* %arrayIdx1098, align 1, !tbaa !21
  %101 = add i64 %1, 8
  %arrayIdx1100 = getelementptr inbounds i8, i8* %pix2, i64 %101
  %gepload1101 = load i8, i8* %arrayIdx1100, align 1, !tbaa !21
  %102 = add i64 %0, 12
  %arrayIdx1102 = getelementptr inbounds i8, i8* %pix1, i64 %102
  %gepload1103 = load i8, i8* %arrayIdx1102, align 1, !tbaa !21
  %103 = add i64 %1, 12
  %arrayIdx1104 = getelementptr inbounds i8, i8* %pix2, i64 %103
  %gepload1105 = load i8, i8* %arrayIdx1104, align 1, !tbaa !21
  %104 = add i64 %0, 9
  %arrayIdx1106 = getelementptr inbounds i8, i8* %pix1, i64 %104
  %gepload1107 = load i8, i8* %arrayIdx1106, align 1, !tbaa !21
  %105 = add i64 %1, 9
  %arrayIdx1108 = getelementptr inbounds i8, i8* %pix2, i64 %105
  %gepload1109 = load i8, i8* %arrayIdx1108, align 1, !tbaa !21
  %106 = add i64 %0, 13
  %arrayIdx1110 = getelementptr inbounds i8, i8* %pix1, i64 %106
  %gepload1111 = load i8, i8* %arrayIdx1110, align 1, !tbaa !21
  %107 = add i64 %1, 13
  %arrayIdx1112 = getelementptr inbounds i8, i8* %pix2, i64 %107
  %gepload1113 = load i8, i8* %arrayIdx1112, align 1, !tbaa !21
  %108 = add i64 %0, 10
  %arrayIdx1114 = getelementptr inbounds i8, i8* %pix1, i64 %108
  %gepload1115 = load i8, i8* %arrayIdx1114, align 1, !tbaa !21
  %109 = add i64 %1, 10
  %arrayIdx1116 = getelementptr inbounds i8, i8* %pix2, i64 %109
  %gepload1117 = load i8, i8* %arrayIdx1116, align 1, !tbaa !21
  %110 = add i64 %0, 14
  %arrayIdx1118 = getelementptr inbounds i8, i8* %pix1, i64 %110
  %gepload1119 = load i8, i8* %arrayIdx1118, align 1, !tbaa !21
  %111 = add i64 %1, 14
  %arrayIdx1120 = getelementptr inbounds i8, i8* %pix2, i64 %111
  %gepload1121 = load i8, i8* %arrayIdx1120, align 1, !tbaa !21
  %112 = add i64 %0, 11
  %arrayIdx1122 = getelementptr inbounds i8, i8* %pix1, i64 %112
  %gepload1123 = load i8, i8* %arrayIdx1122, align 1, !tbaa !21
  %113 = add i64 %1, 11
  %arrayIdx1124 = getelementptr inbounds i8, i8* %pix2, i64 %113
  %gepload1125 = load i8, i8* %arrayIdx1124, align 1, !tbaa !21
  %114 = add i64 %0, 15
  %arrayIdx1126 = getelementptr inbounds i8, i8* %pix1, i64 %114
  %gepload1127 = load i8, i8* %arrayIdx1126, align 1, !tbaa !21
  %115 = add i64 %1, 15
  %arrayIdx1128 = getelementptr inbounds i8, i8* %pix2, i64 %115
  %gepload1129 = load i8, i8* %arrayIdx1128, align 1, !tbaa !21
  %arrayIdx1131 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 4
  %116 = zext i8 %gepload1123 to i32
  %117 = zext i8 %gepload1115 to i32
  %118 = add nuw nsw i32 %116, %117
  %119 = zext i8 %gepload1107 to i32
  %120 = add nuw nsw i32 %118, %119
  %121 = zext i8 %gepload1099 to i32
  %122 = add nuw nsw i32 %120, %121
  %123 = zext i8 %gepload1129 to i32
  %124 = shl nuw nsw i32 %123, 16
  %125 = sub nsw i32 %122, %124
  %126 = zext i8 %gepload1121 to i32
  %127 = shl nuw nsw i32 %126, 16
  %128 = sub nsw i32 %125, %127
  %129 = zext i8 %gepload1113 to i32
  %130 = shl nuw nsw i32 %129, 16
  %131 = sub nsw i32 %128, %130
  %132 = zext i8 %gepload1105 to i32
  %133 = shl nuw nsw i32 %132, 16
  %134 = sub i32 %131, %133
  %135 = zext i8 %gepload1125 to i32
  %136 = sub i32 %134, %135
  %137 = zext i8 %gepload1117 to i32
  %138 = sub i32 %136, %137
  %139 = zext i8 %gepload1109 to i32
  %140 = sub i32 %138, %139
  %141 = zext i8 %gepload1101 to i32
  %142 = sub i32 %140, %141
  %143 = zext i8 %gepload1127 to i32
  %144 = shl nuw nsw i32 %143, 16
  %145 = add i32 %142, %144
  %146 = zext i8 %gepload1119 to i32
  %147 = shl nuw nsw i32 %146, 16
  %148 = add i32 %145, %147
  %149 = zext i8 %gepload1111 to i32
  %150 = shl nuw nsw i32 %149, 16
  %151 = add i32 %148, %150
  %152 = zext i8 %gepload1103 to i32
  %153 = shl nuw nsw i32 %152, 16
  %154 = add i32 %151, %153
  store i32 %154, i32* %arrayIdx1131, align 4
  %arrayIdx1133 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 6
  %155 = add nuw nsw i32 %116, %117
  %156 = sub nsw i32 %119, %155
  %157 = add nsw i32 %156, %121
  %158 = add nsw i32 %157, %124
  %159 = add nsw i32 %158, %127
  %160 = sub nsw i32 %159, %130
  %161 = sub i32 %160, %133
  %162 = add i32 %161, %135
  %163 = add i32 %162, %137
  %164 = sub i32 %163, %139
  %165 = sub i32 %164, %141
  %166 = sub i32 %165, %144
  %167 = sub i32 %166, %147
  %168 = add i32 %167, %150
  %169 = add i32 %168, %153
  store i32 %169, i32* %arrayIdx1133, align 4
  %arrayIdx1151 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 5
  %170 = sub nsw i32 %117, %116
  %171 = sub nsw i32 %170, %119
  %172 = add nsw i32 %171, %121
  %173 = add nsw i32 %172, %124
  %174 = sub nsw i32 %173, %127
  %175 = add nsw i32 %174, %130
  %176 = sub i32 %175, %133
  %177 = add i32 %176, %135
  %178 = sub i32 %177, %137
  %179 = add i32 %178, %139
  %180 = sub i32 %179, %141
  %181 = sub i32 %180, %144
  %182 = add i32 %181, %147
  %183 = sub i32 %182, %150
  %184 = add i32 %183, %153
  store i32 %184, i32* %arrayIdx1151, align 4
  %arrayIdx1169 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 7
  %185 = sub nsw i32 %116, %117
  %186 = sub nsw i32 %185, %119
  %187 = add nsw i32 %186, %121
  %188 = sub nsw i32 %187, %124
  %189 = add nsw i32 %188, %127
  %190 = add nsw i32 %189, %130
  %191 = sub i32 %190, %133
  %192 = sub i32 %191, %135
  %193 = add i32 %192, %137
  %194 = add i32 %193, %139
  %195 = sub i32 %194, %141
  %196 = add i32 %195, %144
  %197 = sub i32 %196, %147
  %198 = sub i32 %197, %150
  %199 = add i32 %198, %153
  store i32 %199, i32* %arrayIdx1169, align 4
  %nextivloop.1247 = add nuw nsw i64 %i1.i64.0, 1
  %condloop.1247 = icmp ult i64 %nextivloop.1247, 16
  br i1 %condloop.1247, label %loop.1247, label %afterloop.1247

afterloop.1247:                                   ; preds = %loop.1247
  ret i32 0
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang cd5be4ea094f77df25a78b0ab0ead6e58960206c) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm f515ae58d78783d4a202c33471bf5e00ce0fc49b)"}
!2 = !{!3, !5, i64 56}
!3 = !{!"struct@", !4, i64 0, !4, i64 56, !4, i64 112, !4, i64 168, !8, i64 224, !4, i64 256, !4, i64 312, !4, i64 368, !9, i64 424, !11, i64 480, !4, i64 536, !13, i64 592, !14, i64 600, !14, i64 632, !16, i64 664, !17, i64 672, !9, i64 680, !11, i64 736, !9, i64 792, !11, i64 848, !18, i64 904, !20, i64 960, !20, i64 968, !20, i64 976, !20, i64 984, !20, i64 992, !20, i64 1000, !20, i64 1008, !20, i64 1016, !20, i64 1024, !20, i64 1032, !20, i64 1040, !20, i64 1048}
!4 = !{!"array@_ZTSA7_PFiPhiS_iE", !5, i64 0}
!5 = !{!"pointer@_ZTSPFiPhiS_iE", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!"array@_ZTSA4_PFiPhiS_iE", !5, i64 0}
!9 = !{!"array@_ZTSA7_PFvPhS_S_S_iPiE", !10, i64 0}
!10 = !{!"pointer@_ZTSPFvPhS_S_S_iPiE", !6, i64 0}
!11 = !{!"array@_ZTSA7_PFvPhS_S_S_S_iPiE", !12, i64 0}
!12 = !{!"pointer@_ZTSPFvPhS_S_S_S_iPiE", !6, i64 0}
!13 = !{!"pointer@_ZTSPFiPhiS_iPiE", !6, i64 0}
!14 = !{!"array@_ZTSA4_PFmPhiE", !15, i64 0}
!15 = !{!"pointer@_ZTSPFmPhiE", !6, i64 0}
!16 = !{!"pointer@_ZTSPFvPKhiS0_iPA4_iE", !6, i64 0}
!17 = !{!"pointer@_ZTSPFfPA4_iS0_iE", !6, i64 0}
!18 = !{!"array@_ZTSA7_PFiPiPtiS0_PsiiE", !19, i64 0}
!19 = !{!"pointer@_ZTSPFiPiPtiS0_PsiiE", !6, i64 0}
!20 = !{!"pointer@_ZTSPFvPhS_PiE", !6, i64 0}
!21 = !{!6, !6, i64 0}
