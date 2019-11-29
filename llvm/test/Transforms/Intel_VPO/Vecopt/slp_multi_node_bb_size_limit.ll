; RUN: opt < %s -slp-vectorizer -enable-intel-advanced-opts -mtriple=x86_64-unknown-linux-gnu -pslp -mcpu=skylake-avx512 -tti -max-bb-size-for-multi-node-slp=250 -S | FileCheck %s -check-prefix=CHECK_LO_LIMIT
; RUN: opt < %s -slp-vectorizer -enable-intel-advanced-opts -mtriple=x86_64-unknown-linux-gnu -pslp -mcpu=skylake-avx512 -tti -max-bb-size-for-multi-node-slp=400 -S | FileCheck %s -check-prefix=CHECK_HI_LIMIT

; This test tries to limit building MultiNodes via lowering maximum BB
; size allowed for the optimization. Basic block labeled lp.1248 is processed first and
; after it has been processed ~20 instructions added to list of "to be deleted" instructions.
; When we process block lp.1247 these deleted instructions should not affect current
; block size calculation. The BB initial size is 258 instructions thus with the limit
; set to 250 this test expects to hit the maximum. If we erroneously reduce number of
; instructions by those deleted in the other block the optimization kicks in unexpectedly.

; CHECK_LO_LIMIT-NOT: [[L1:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_LO_LIMIT-NOT: [[L2:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_LO_LIMIT-NOT: [[L3:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_LO_LIMIT-NOT: [[L4:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_LO_LIMIT-NOT: [[L5:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_LO_LIMIT-NOT: [[L6:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_LO_LIMIT-NOT: [[L7:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_LO_LIMIT-NOT: [[L8:%.*]] = load <4 x i8>, <4 x i8>*

; Check that we vectorize Multi-Node with vector length 8
; CHECK_HI_LIMIT: [[L1:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_HI_LIMIT: [[L2:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_HI_LIMIT: [[L3:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_HI_LIMIT: [[L4:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_HI_LIMIT: [[L5:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_HI_LIMIT: [[L6:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_HI_LIMIT: [[L7:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_HI_LIMIT: [[L8:%.*]] = load <4 x i8>, <4 x i8>*
; CHECK_HI_LIMIT: store <8 x i32> [[SEL1:%.*]]


%struct.S = type { [8 x i32], [8 x i32] }

define dso_local i32 @foo(i8* nocapture readonly %pix1, i32 %i_pix1, i8* nocapture readonly %pix2, i32 %i_pix2, i32 %x, %struct.S* nocapture %p) {
entry:
  %idx.ext.i = sext i32 %i_pix1 to i64
  %idx.ext63.i = sext i32 %i_pix2 to i64
  %alloca = alloca [16 x [8 x i32]], align 4
  %alloca933 = alloca [8 x i32], align 16
  %alloca934 = alloca [8 x i32], align 16
  %alloca935 = alloca [8 x i32], align 16
  %alloca936 = alloca [8 x i32], align 16

  br label %lp.1247

lp.1247:

  %i1.i64.0 = sext i32 %x to i64
  %0 = mul i64 %i1.i64.0, %idx.ext.i
  %arrayIdx = getelementptr inbounds i8, i8* %pix1, i64 %0
  %gepload = load i8, i8* %arrayIdx, align 1, !tbaa !2
  %1 = mul i64 %i1.i64.0, %idx.ext63.i
  %arrayIdx1013 = getelementptr inbounds i8, i8* %pix2, i64 %1
  %gepload1014 = load i8, i8* %arrayIdx1013, align 1, !tbaa !2
  %2 = add i64 %0, 4
  %arrayIdx1015 = getelementptr inbounds i8, i8* %pix1, i64 %2
  %gepload1016 = load i8, i8* %arrayIdx1015, align 1, !tbaa !2
  %3 = add i64 %1, 4
  %arrayIdx1017 = getelementptr inbounds i8, i8* %pix2, i64 %3
  %gepload1018 = load i8, i8* %arrayIdx1017, align 1, !tbaa !2
  %4 = add i64 %0, 1
  %arrayIdx1019 = getelementptr inbounds i8, i8* %pix1, i64 %4
  %gepload1020 = load i8, i8* %arrayIdx1019, align 1, !tbaa !2
  %5 = add i64 %1, 1
  %arrayIdx1021 = getelementptr inbounds i8, i8* %pix2, i64 %5
  %gepload1022 = load i8, i8* %arrayIdx1021, align 1, !tbaa !2
  %6 = add i64 %0, 5
  %arrayIdx1023 = getelementptr inbounds i8, i8* %pix1, i64 %6
  %gepload1024 = load i8, i8* %arrayIdx1023, align 1, !tbaa !2
  %7 = add i64 %1, 5
  %arrayIdx1025 = getelementptr inbounds i8, i8* %pix2, i64 %7
  %gepload1026 = load i8, i8* %arrayIdx1025, align 1, !tbaa !2
  %8 = add i64 %0, 2
  %arrayIdx1027 = getelementptr inbounds i8, i8* %pix1, i64 %8
  %gepload1028 = load i8, i8* %arrayIdx1027, align 1, !tbaa !2
  %9 = add i64 %1, 2
  %arrayIdx1029 = getelementptr inbounds i8, i8* %pix2, i64 %9
  %gepload1030 = load i8, i8* %arrayIdx1029, align 1, !tbaa !2
  %10 = add i64 %0, 6
  %arrayIdx1031 = getelementptr inbounds i8, i8* %pix1, i64 %10
  %gepload1032 = load i8, i8* %arrayIdx1031, align 1, !tbaa !2
  %11 = add i64 %1, 6
  %arrayIdx1033 = getelementptr inbounds i8, i8* %pix2, i64 %11
  %gepload1034 = load i8, i8* %arrayIdx1033, align 1, !tbaa !2
  %12 = add i64 %0, 3
  %arrayIdx1035 = getelementptr inbounds i8, i8* %pix1, i64 %12
  %gepload1036 = load i8, i8* %arrayIdx1035, align 1, !tbaa !2
  %13 = add i64 %1, 3
  %arrayIdx1037 = getelementptr inbounds i8, i8* %pix2, i64 %13
  %gepload1038 = load i8, i8* %arrayIdx1037, align 1, !tbaa !2
  %14 = add i64 %0, 7
  %arrayIdx1039 = getelementptr inbounds i8, i8* %pix1, i64 %14
  %gepload1040 = load i8, i8* %arrayIdx1039, align 1, !tbaa !2
  %15 = add i64 %1, 7
  %arrayIdx1041 = getelementptr inbounds i8, i8* %pix2, i64 %15
  %gepload1042 = load i8, i8* %arrayIdx1041, align 1, !tbaa !2
  %arrayIdx1043 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 0
  %16 = zext i8 %gepload1036 to i32
  %17 = zext i8 %gepload1028 to i32
  %18 = zext i8 %gepload1020 to i32
  %19 = zext i8 %gepload to i32
  %20 = zext i8 %gepload1042 to i32
  %21 = zext i8 %gepload1034 to i32
  %22 = zext i8 %gepload1026 to i32
  %23 = zext i8 %gepload1018 to i32
  %24 = zext i8 %gepload1038 to i32
  %25 = zext i8 %gepload1030 to i32
  %26 = zext i8 %gepload1022 to i32
  %27 = zext i8 %gepload1014 to i32
  %28 = zext i8 %gepload1040 to i32
  %29 = zext i8 %gepload1032 to i32
  %30 = zext i8 %gepload1024 to i32
  %31 = zext i8 %gepload1016 to i32
  %32 = shl i32 %20, 16
  %33 = shl i32 %21, 16
  %34 = shl i32 %22, 16
  %35 = shl i32 %23, 16
  %36 = shl i32 %28, 16
  %37 = shl i32 %29, 16
  %38 = shl i32 %30, 16
  %39 = shl i32 %31, 16
  %Chain_T24_168 = sub i32 %36, %24
  %Chain_T24_171 = sub i32 %Chain_T24_168, %32
  %Chain_T24_169 = add i32 %Chain_T24_171, %16
  %Chain_T24_160 = sub i32 %37, %25
  %Chain_T24_163 = sub i32 %Chain_T24_160, %33
  %Chain_T24_161 = add i32 %Chain_T24_163, %17
  %Chain_T24_152 = sub i32 %38, %26
  %Chain_T24_155 = sub i32 %Chain_T24_152, %34
  %Chain_T24_153 = add i32 %Chain_T24_155, %18
  %Chain_T24_ = sub i32 %39, %27
  %Chain_T24_150 = sub i32 %Chain_T24_, %35
  %Chain_T24_148 = add i32 %Chain_T24_150, %19
  %Bridge_T24_151 = add i32 %Chain_T24_148, %Chain_T24_153
  %Bridge_T24_159 = add i32 %Bridge_T24_151, %Chain_T24_161
  %Bridge_T24_167 = add i32 %Bridge_T24_159, %Chain_T24_169
  store i32 %Bridge_T24_167, i32* %arrayIdx1043, align 4
  %arrayIdx1045 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 2
  %40 = shl i32 %20, 16
  %41 = shl i32 %21, 16
  %42 = shl i32 %22, 16
  %43 = shl i32 %23, 16
  %44 = shl i32 %28, 16
  %45 = shl i32 %29, 16
  %46 = shl i32 %30, 16
  %47 = shl i32 %31, 16
  %Bridge_T23_156 = add i32 %Chain_T24_148, %Chain_T24_153
  %Bridge_T23_164 = sub i32 %Bridge_T23_156, %Chain_T24_161
  %Bridge_T23_172 = sub i32 %Bridge_T23_164, %Chain_T24_169
  store i32 %Bridge_T23_172, i32* %arrayIdx1045, align 4
  %arrayIdx1063 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 1
  %48 = shl i32 %20, 16
  %49 = shl i32 %21, 16
  %50 = shl i32 %22, 16
  %51 = shl i32 %23, 16
  %52 = shl i32 %28, 16
  %53 = shl i32 %29, 16
  %54 = shl i32 %30, 16
  %55 = shl i32 %31, 16
  %Bridge_T22_157 = sub i32 %Chain_T24_148, %Chain_T24_153
  %Bridge_T22_165 = add i32 %Bridge_T22_157, %Chain_T24_161
  %Bridge_T22_173 = sub i32 %Bridge_T22_165, %Chain_T24_169
  store i32 %Bridge_T22_173, i32* %arrayIdx1063, align 4
  %arrayIdx1081 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 3
  %56 = shl i32 %20, 16
  %57 = shl i32 %21, 16
  %58 = shl i32 %22, 16
  %59 = shl i32 %23, 16
  %60 = shl i32 %28, 16
  %61 = shl i32 %29, 16
  %62 = shl i32 %30, 16
  %63 = shl i32 %31, 16
  %Bridge_T21_158 = sub i32 %Chain_T24_148, %Chain_T24_153
  %Bridge_T21_166 = sub i32 %Bridge_T21_158, %Chain_T24_161
  %Bridge_T21_174 = add i32 %Bridge_T21_166, %Chain_T24_169
  store i32 %Bridge_T21_174, i32* %arrayIdx1081, align 4
  %64 = add i64 %0, 8
  %arrayIdx1098 = getelementptr inbounds i8, i8* %pix1, i64 %64
  %gepload1099 = load i8, i8* %arrayIdx1098, align 1, !tbaa !2
  %65 = add i64 %1, 8
  %arrayIdx1100 = getelementptr inbounds i8, i8* %pix2, i64 %65
  %gepload1101 = load i8, i8* %arrayIdx1100, align 1, !tbaa !2
  %66 = add i64 %0, 12
  %arrayIdx1102 = getelementptr inbounds i8, i8* %pix1, i64 %66
  %gepload1103 = load i8, i8* %arrayIdx1102, align 1, !tbaa !2
  %67 = add i64 %1, 12
  %arrayIdx1104 = getelementptr inbounds i8, i8* %pix2, i64 %67
  %gepload1105 = load i8, i8* %arrayIdx1104, align 1, !tbaa !2
  %68 = add i64 %0, 9
  %arrayIdx1106 = getelementptr inbounds i8, i8* %pix1, i64 %68
  %gepload1107 = load i8, i8* %arrayIdx1106, align 1, !tbaa !2
  %69 = add i64 %1, 9
  %arrayIdx1108 = getelementptr inbounds i8, i8* %pix2, i64 %69
  %gepload1109 = load i8, i8* %arrayIdx1108, align 1, !tbaa !2
  %70 = add i64 %0, 13
  %arrayIdx1110 = getelementptr inbounds i8, i8* %pix1, i64 %70
  %gepload1111 = load i8, i8* %arrayIdx1110, align 1, !tbaa !2
  %71 = add i64 %1, 13
  %arrayIdx1112 = getelementptr inbounds i8, i8* %pix2, i64 %71
  %gepload1113 = load i8, i8* %arrayIdx1112, align 1, !tbaa !2
  %72 = add i64 %0, 10
  %arrayIdx1114 = getelementptr inbounds i8, i8* %pix1, i64 %72
  %gepload1115 = load i8, i8* %arrayIdx1114, align 1, !tbaa !2
  %73 = add i64 %1, 10
  %arrayIdx1116 = getelementptr inbounds i8, i8* %pix2, i64 %73
  %gepload1117 = load i8, i8* %arrayIdx1116, align 1, !tbaa !2
  %74 = add i64 %0, 14
  %arrayIdx1118 = getelementptr inbounds i8, i8* %pix1, i64 %74
  %gepload1119 = load i8, i8* %arrayIdx1118, align 1, !tbaa !2
  %75 = add i64 %1, 14
  %arrayIdx1120 = getelementptr inbounds i8, i8* %pix2, i64 %75
  %gepload1121 = load i8, i8* %arrayIdx1120, align 1, !tbaa !2
  %76 = add i64 %0, 11
  %arrayIdx1122 = getelementptr inbounds i8, i8* %pix1, i64 %76
  %gepload1123 = load i8, i8* %arrayIdx1122, align 1, !tbaa !2
  %77 = add i64 %1, 11
  %arrayIdx1124 = getelementptr inbounds i8, i8* %pix2, i64 %77
  %gepload1125 = load i8, i8* %arrayIdx1124, align 1, !tbaa !2
  %78 = add i64 %0, 15
  %arrayIdx1126 = getelementptr inbounds i8, i8* %pix1, i64 %78
  %gepload1127 = load i8, i8* %arrayIdx1126, align 1, !tbaa !2
  %79 = add i64 %1, 15
  %arrayIdx1128 = getelementptr inbounds i8, i8* %pix2, i64 %79
  %gepload1129 = load i8, i8* %arrayIdx1128, align 1, !tbaa !2
  %arrayIdx1131 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 4
  %80 = zext i8 %gepload1123 to i32
  %81 = zext i8 %gepload1115 to i32
  %82 = zext i8 %gepload1107 to i32
  %83 = zext i8 %gepload1099 to i32
  %84 = zext i8 %gepload1129 to i32
  %85 = zext i8 %gepload1121 to i32
  %86 = zext i8 %gepload1113 to i32
  %87 = zext i8 %gepload1105 to i32
  %88 = zext i8 %gepload1125 to i32
  %89 = zext i8 %gepload1117 to i32
  %90 = zext i8 %gepload1109 to i32
  %91 = zext i8 %gepload1101 to i32
  %92 = zext i8 %gepload1127 to i32
  %93 = zext i8 %gepload1119 to i32
  %94 = zext i8 %gepload1111 to i32
  %95 = zext i8 %gepload1103 to i32
  %96 = shl i32 %84, 16
  %97 = shl i32 %85, 16
  %98 = shl i32 %86, 16
  %99 = shl i32 %87, 16
  %100 = shl i32 %92, 16
  %101 = shl i32 %93, 16
  %102 = shl i32 %94, 16
  %103 = shl i32 %95, 16
  %Chain_T4_81 = sub i32 %100, %88
  %Chain_T4_84 = sub i32 %Chain_T4_81, %96
  %Chain_T4_82 = add i32 %Chain_T4_84, %80
  %Chain_T4_73 = sub i32 %101, %89
  %Chain_T4_76 = sub i32 %Chain_T4_73, %97
  %Chain_T4_74 = add i32 %Chain_T4_76, %81
  %Chain_T4_65 = sub i32 %102, %90
  %Chain_T4_68 = sub i32 %Chain_T4_65, %98
  %Chain_T4_66 = add i32 %Chain_T4_68, %82
  %Chain_T4_ = sub i32 %103, %91
  %Chain_T4_63 = sub i32 %Chain_T4_, %99
  %Chain_T4_61 = add i32 %Chain_T4_63, %83
  %Bridge_T4_64 = add i32 %Chain_T4_61, %Chain_T4_66
  %Bridge_T4_72 = add i32 %Bridge_T4_64, %Chain_T4_74
  %Bridge_T4_80 = add i32 %Bridge_T4_72, %Chain_T4_82
  store i32 %Bridge_T4_80, i32* %arrayIdx1131, align 4
  %arrayIdx1133 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 6
  %104 = shl i32 %84, 16
  %105 = shl i32 %85, 16
  %106 = shl i32 %86, 16
  %107 = shl i32 %87, 16
  %108 = shl i32 %92, 16
  %109 = shl i32 %93, 16
  %110 = shl i32 %94, 16
  %111 = shl i32 %95, 16
  %Bridge_T3_69 = add i32 %Chain_T4_61, %Chain_T4_66
  %Bridge_T3_77 = sub i32 %Bridge_T3_69, %Chain_T4_74
  %Bridge_T3_85 = sub i32 %Bridge_T3_77, %Chain_T4_82
  store i32 %Bridge_T3_85, i32* %arrayIdx1133, align 4
  %arrayIdx1151 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 5
  %112 = shl i32 %84, 16
  %113 = shl i32 %85, 16
  %114 = shl i32 %86, 16
  %115 = shl i32 %87, 16
  %116 = shl i32 %92, 16
  %117 = shl i32 %93, 16
  %118 = shl i32 %94, 16
  %119 = shl i32 %95, 16
  %Bridge_T2_70 = sub i32 %Chain_T4_61, %Chain_T4_66
  %Bridge_T2_78 = add i32 %Bridge_T2_70, %Chain_T4_74
  %Bridge_T2_86 = sub i32 %Bridge_T2_78, %Chain_T4_82
  store i32 %Bridge_T2_86, i32* %arrayIdx1151, align 4
  %arrayIdx1169 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 7
  %120 = shl i32 %84, 16
  %121 = shl i32 %85, 16
  %122 = shl i32 %86, 16
  %123 = shl i32 %87, 16
  %124 = shl i32 %92, 16
  %125 = shl i32 %93, 16
  %126 = shl i32 %94, 16
  %127 = shl i32 %95, 16
  %Bridge_T1_71 = sub i32 %Chain_T4_61, %Chain_T4_66
  %Bridge_T1_79 = sub i32 %Bridge_T1_71, %Chain_T4_74
  %Bridge_T1_87 = add i32 %Bridge_T1_79, %Chain_T4_82
  store i32 %Bridge_T1_87, i32* %arrayIdx1169, align 4

  br label %lp.1248

lp.1248:
  %arrayidx = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 1, i64 0
  %n0 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %arrayidx1 = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 0, i64 0
  %n1 = load i32, i32* %arrayidx1, align 4, !tbaa !2
  %sub = add i32 %n0, 1
  %add = sub i32 %sub, %n1
  store i32 %add, i32* %arrayidx1, align 4, !tbaa !2
  %arrayidx5 = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 1, i64 1
  %n2 = load i32, i32* %arrayidx5, align 4, !tbaa !2
  %arrayidx7 = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 0, i64 1
  %n3 = load i32, i32* %arrayidx7, align 4, !tbaa !2
  %sub8 = add i32 %n2, 2
  %add9 = sub i32 %sub8, %n3
  store i32 %add9, i32* %arrayidx7, align 4, !tbaa !2
  %arrayidx13 = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 1, i64 2
  %n4 = load i32, i32* %arrayidx13, align 4, !tbaa !2
  %arrayidx15 = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 0, i64 2
  %n5 = load i32, i32* %arrayidx15, align 4, !tbaa !2
  %sub16 = add i32 %n4, 3
  %add17 = sub i32 %sub16, %n5
  store i32 %add17, i32* %arrayidx15, align 4, !tbaa !2
  %arrayidx21 = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 1, i64 3
  %n6 = load i32, i32* %arrayidx21, align 4, !tbaa !2
  %arrayidx23 = getelementptr inbounds %struct.S, %struct.S* %p, i64 0, i32 0, i64 3
  %n7 = load i32, i32* %arrayidx23, align 4, !tbaa !2
  %sub24 = add i32 %n6, 4
  %add25 = sub i32 %sub24, %n7
  store i32 %add25, i32* %arrayidx23, align 4, !tbaa !2

  ret i32 0
}

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
