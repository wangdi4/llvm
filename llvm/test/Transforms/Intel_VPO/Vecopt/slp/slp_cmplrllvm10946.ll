; RUN: opt < %s -slp-vectorizer -enable-intel-advanced-opts -slp-multinode -mtriple=x86_64 -mcpu=skylake-avx512 -max-bb-size-for-multi-node-slp=100 -S | FileCheck %s

; Test case reproduces situation when Multi-Node is being built and
; an instruction operands defs turned out in different basic block.
; Due to incorrectly placed basic block size check we end up with assertion (or
; empty output if assertions not enabled) that execution took unexpected path.
; Originally the issue revealed on denbench test but this one modeled
; using existing slp_x264_16x16_unary test case as a base.

; CHECK: store <8 x i32>

define dso_local i32 @foo(i8* nocapture readonly %pix1, i32 %i_pix1, i8* nocapture readonly %pix2, i32 %i_pix2) {
entry:
  %alloca = alloca [16 x [8 x i32]], align 4
  %alloca933 = alloca [8 x i32], align 16
  %alloca934 = alloca [8 x i32], align 16
  %alloca935 = alloca [8 x i32], align 16
  %alloca936 = alloca [8 x i32], align 16
  br label %loop.1247

loop.1247:                                        ; preds = %inner_bl, %entry
  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.1247, %inner_bl ]
  %0 = sext i32 %i_pix1 to i64
  %1 = mul i64 %i1.i64.0, %0
  %arrayIdx = getelementptr inbounds i8, i8* %pix1, i64 %1
  %gepload = load i8, i8* %arrayIdx, align 1
  %2 = sext i32 %i_pix2 to i64
  %3 = mul i64 %i1.i64.0, %2
  %arrayIdx1013 = getelementptr inbounds i8, i8* %pix2, i64 %3
  %gepload1014 = load i8, i8* %arrayIdx1013, align 1
  %4 = add i64 %1, 4
  %arrayIdx1015 = getelementptr inbounds i8, i8* %pix1, i64 %4
  %gepload1016 = load i8, i8* %arrayIdx1015, align 1
  %5 = add i64 %3, 4
  %arrayIdx1017 = getelementptr inbounds i8, i8* %pix2, i64 %5
  %gepload1018 = load i8, i8* %arrayIdx1017, align 1
  %6 = add i64 %1, 1
  %arrayIdx1019 = getelementptr inbounds i8, i8* %pix1, i64 %6
  %gepload1020 = load i8, i8* %arrayIdx1019, align 1
  %7 = add i64 %3, 1
  %arrayIdx1021 = getelementptr inbounds i8, i8* %pix2, i64 %7
  %gepload1022 = load i8, i8* %arrayIdx1021, align 1
  %8 = add i64 %1, 5
  %arrayIdx1023 = getelementptr inbounds i8, i8* %pix1, i64 %8
  %gepload1024 = load i8, i8* %arrayIdx1023, align 1
  %9 = add i64 %3, 5
  %arrayIdx1025 = getelementptr inbounds i8, i8* %pix2, i64 %9
  %gepload1026 = load i8, i8* %arrayIdx1025, align 1
  %10 = add i64 %1, 2
  %arrayIdx1027 = getelementptr inbounds i8, i8* %pix1, i64 %10
  %gepload1028 = load i8, i8* %arrayIdx1027, align 1
  %11 = add i64 %3, 2
  %arrayIdx1029 = getelementptr inbounds i8, i8* %pix2, i64 %11
  %gepload1030 = load i8, i8* %arrayIdx1029, align 1
  %12 = add i64 %1, 6
  %arrayIdx1031 = getelementptr inbounds i8, i8* %pix1, i64 %12
  %gepload1032 = load i8, i8* %arrayIdx1031, align 1
  %13 = add i64 %3, 6
  %arrayIdx1033 = getelementptr inbounds i8, i8* %pix2, i64 %13
  %gepload1034 = load i8, i8* %arrayIdx1033, align 1
  %14 = add i64 %1, 3
  %arrayIdx1035 = getelementptr inbounds i8, i8* %pix1, i64 %14
  %gepload1036 = load i8, i8* %arrayIdx1035, align 1
  %15 = add i64 %3, 3
  %arrayIdx1037 = getelementptr inbounds i8, i8* %pix2, i64 %15
  %gepload1038 = load i8, i8* %arrayIdx1037, align 1
  %16 = add i64 %1, 7
  %arrayIdx1039 = getelementptr inbounds i8, i8* %pix1, i64 %16
  %gepload1040 = load i8, i8* %arrayIdx1039, align 1
  %17 = add i64 %3, 7
  %arrayIdx1041 = getelementptr inbounds i8, i8* %pix2, i64 %17
  %gepload1042 = load i8, i8* %arrayIdx1041, align 1
  %18 = zext i8 %gepload1036 to i32
  %19 = zext i8 %gepload1028 to i32
  %20 = zext i8 %gepload1020 to i32
  %21 = zext i8 %gepload to i32
  %22 = zext i8 %gepload1042 to i32
  %23 = zext i8 %gepload1034 to i32
  %24 = zext i8 %gepload1026 to i32
  %25 = zext i8 %gepload1018 to i32
  %26 = zext i8 %gepload1038 to i32
  %27 = zext i8 %gepload1030 to i32
  %28 = zext i8 %gepload1022 to i32
  %29 = zext i8 %gepload1014 to i32
  %30 = zext i8 %gepload1040 to i32
  %31 = zext i8 %gepload1032 to i32
  %32 = zext i8 %gepload1024 to i32
  %33 = zext i8 %gepload1016 to i32
  %34 = shl i32 %25, 16
  %35 = shl i32 %33, 16
  %Chain_T24_168 = sub i32 %35, %29
  %Chain_T24_171 = sub i32 %Chain_T24_168, %34
  %Chain_T24_169 = add i32 %Chain_T24_171, %21
  %36 = shl i32 %32, 16
  %37 = shl i32 %24, 16
  %Chain_T24_162 = sub i32 %36, %28
  %Chain_T24_160 = sub i32 %Chain_T24_162, %37
  %Chain_T24_163 = add i32 %Chain_T24_160, %20
  %38 = shl i32 %31, 16
  %39 = shl i32 %23, 16
  %Chain_T24_154 = sub i32 %38, %27
  %Chain_T24_152 = sub i32 %Chain_T24_154, %39
  %Chain_T24_155 = add i32 %Chain_T24_152, %19
  %40 = shl i32 %30, 16
  %41 = shl i32 %22, 16
  %Chain_T24_149 = sub i32 %40, %26
  %Chain_T24_ = sub i32 %Chain_T24_149, %41
  %Chain_T24_150 = add i32 %Chain_T24_, %18
  %42 = shl i32 %25, 16
  %43 = shl i32 %33, 16
  %44 = shl i32 %32, 16
  %45 = shl i32 %24, 16
  %46 = shl i32 %31, 16
  %47 = shl i32 %23, 16
  %48 = shl i32 %30, 16
  %49 = shl i32 %22, 16
  %50 = shl i32 %25, 16
  %51 = shl i32 %33, 16
  %52 = shl i32 %32, 16
  %53 = shl i32 %24, 16
  %54 = shl i32 %31, 16
  %55 = shl i32 %23, 16
  %56 = shl i32 %30, 16
  %57 = shl i32 %22, 16
  %58 = shl i32 %25, 16
  %59 = shl i32 %33, 16
  %60 = shl i32 %32, 16
  %61 = shl i32 %24, 16
  %62 = shl i32 %31, 16
  %63 = shl i32 %23, 16
  %64 = shl i32 %30, 16
  %65 = shl i32 %22, 16

  %66 = add i64 %1, 8
  %arrayIdx1098 = getelementptr inbounds i8, i8* %pix1, i64 %66
  %gepload1099 = load i8, i8* %arrayIdx1098, align 1
  %67 = add i64 %3, 8
  %arrayIdx1100 = getelementptr inbounds i8, i8* %pix2, i64 %67
  %gepload1101 = load i8, i8* %arrayIdx1100, align 1
  %68 = add i64 %1, 12
  %arrayIdx1102 = getelementptr inbounds i8, i8* %pix1, i64 %68
  %gepload1103 = load i8, i8* %arrayIdx1102, align 1
  %69 = add i64 %3, 12
  %arrayIdx1104 = getelementptr inbounds i8, i8* %pix2, i64 %69
  %gepload1105 = load i8, i8* %arrayIdx1104, align 1
  %70 = add i64 %1, 9
  %arrayIdx1106 = getelementptr inbounds i8, i8* %pix1, i64 %70
  %gepload1107 = load i8, i8* %arrayIdx1106, align 1
  %71 = add i64 %3, 9
  %arrayIdx1108 = getelementptr inbounds i8, i8* %pix2, i64 %71
  %gepload1109 = load i8, i8* %arrayIdx1108, align 1
  %72 = add i64 %1, 13
  %arrayIdx1110 = getelementptr inbounds i8, i8* %pix1, i64 %72
  %gepload1111 = load i8, i8* %arrayIdx1110, align 1
  %73 = add i64 %3, 13
  %arrayIdx1112 = getelementptr inbounds i8, i8* %pix2, i64 %73
  %gepload1113 = load i8, i8* %arrayIdx1112, align 1
  %74 = add i64 %1, 10
  %arrayIdx1114 = getelementptr inbounds i8, i8* %pix1, i64 %74
  %gepload1115 = load i8, i8* %arrayIdx1114, align 1
  %75 = add i64 %3, 10
  %arrayIdx1116 = getelementptr inbounds i8, i8* %pix2, i64 %75
  %gepload1117 = load i8, i8* %arrayIdx1116, align 1
  %76 = add i64 %1, 14
  %arrayIdx1118 = getelementptr inbounds i8, i8* %pix1, i64 %76
  %gepload1119 = load i8, i8* %arrayIdx1118, align 1
  %77 = add i64 %3, 14
  %arrayIdx1120 = getelementptr inbounds i8, i8* %pix2, i64 %77
  %gepload1121 = load i8, i8* %arrayIdx1120, align 1
  %78 = add i64 %1, 11
  %arrayIdx1122 = getelementptr inbounds i8, i8* %pix1, i64 %78
  %gepload1123 = load i8, i8* %arrayIdx1122, align 1
  %79 = add i64 %3, 11
  %arrayIdx1124 = getelementptr inbounds i8, i8* %pix2, i64 %79
  %gepload1125 = load i8, i8* %arrayIdx1124, align 1
  %80 = add i64 %1, 15
  %arrayIdx1126 = getelementptr inbounds i8, i8* %pix1, i64 %80
  %gepload1127 = load i8, i8* %arrayIdx1126, align 1
  %81 = add i64 %3, 15
  %arrayIdx1128 = getelementptr inbounds i8, i8* %pix2, i64 %81
  %gepload1129 = load i8, i8* %arrayIdx1128, align 1

  %82 = zext i8 %gepload1123 to i32
  %83 = zext i8 %gepload1115 to i32
  %84 = zext i8 %gepload1107 to i32
  %85 = zext i8 %gepload1099 to i32
  %86 = zext i8 %gepload1129 to i32
  %87 = zext i8 %gepload1121 to i32
  %88 = zext i8 %gepload1113 to i32
  %89 = zext i8 %gepload1105 to i32
  %90 = zext i8 %gepload1125 to i32
  %91 = zext i8 %gepload1117 to i32
  %92 = zext i8 %gepload1109 to i32
  %93 = zext i8 %gepload1101 to i32
  %94 = zext i8 %gepload1127 to i32
  %95 = zext i8 %gepload1119 to i32
  %96 = zext i8 %gepload1111 to i32
  %97 = zext i8 %gepload1103 to i32
  %98 = shl i32 %89, 16
  %99 = shl i32 %97, 16
  %Chain_T4_81 = sub i32 %99, %93
  %Chain_T4_84 = sub i32 %Chain_T4_81, %98
  %Chain_T4_82 = add i32 %Chain_T4_84, %85
  %100 = shl i32 %96, 16
  %101 = shl i32 %88, 16
  %Chain_T4_75 = sub i32 %100, %92
  %Chain_T4_73 = sub i32 %Chain_T4_75, %101
  %Chain_T4_76 = add i32 %Chain_T4_73, %84
  %102 = shl i32 %95, 16
  %103 = shl i32 %87, 16
  %Chain_T4_67 = sub i32 %102, %91
  %Chain_T4_65 = sub i32 %Chain_T4_67, %103
  %Chain_T4_68 = add i32 %Chain_T4_65, %83
  %104 = shl i32 %94, 16
  %105 = shl i32 %86, 16
  %Chain_T4_62 = sub i32 %104, %90
  %Chain_T4_ = sub i32 %Chain_T4_62, %105
  %Chain_T4_63 = add i32 %Chain_T4_, %82
  br label %inner_bl

inner_bl:
  %106 = shl i32 %89, 16
  %107 = shl i32 %97, 16
  %108 = shl i32 %96, 16
  %109 = shl i32 %88, 16
  %110 = shl i32 %95, 16
  %111 = shl i32 %87, 16
  %112 = shl i32 %94, 16
  %113 = shl i32 %86, 16
  %114 = shl i32 %89, 16
  %115 = shl i32 %97, 16
  %116 = shl i32 %96, 16
  %117 = shl i32 %88, 16
  %118 = shl i32 %95, 16
  %119 = shl i32 %87, 16
  %120 = shl i32 %94, 16
  %121 = shl i32 %86, 16
  %122 = shl i32 %89, 16
  %123 = shl i32 %97, 16
  %124 = shl i32 %96, 16
  %125 = shl i32 %88, 16
  %126 = shl i32 %95, 16
  %127 = shl i32 %87, 16
  %128 = shl i32 %94, 16
  %129 = shl i32 %86, 16

  %Bridge_T24_151 = add i32 %Chain_T24_150, %Chain_T24_155
  %Bridge_T24_159 = add i32 %Bridge_T24_151, %Chain_T24_163
  %Bridge_T24_167 = add i32 %Bridge_T24_159, %Chain_T24_169


  %Bridge_T23_156 = sub i32 %Chain_T24_163, %Chain_T24_155
  %Bridge_T23_ = sub i32 %Bridge_T23_156, %Chain_T24_150
  %Bridge_T23_172 = add i32 %Bridge_T23_, %Chain_T24_169

  %Bridge_T22_ = sub i32 %Chain_T24_155, %Chain_T24_150
  %Bridge_T22_165 = sub i32 %Bridge_T22_, %Chain_T24_163
  %Bridge_T22_173 = add i32 %Bridge_T22_165, %Chain_T24_169


  %Bridge_T21_158 = sub i32 %Chain_T24_150, %Chain_T24_155
  %Bridge_T21_166 = sub i32 %Bridge_T21_158, %Chain_T24_163
  %Bridge_T21_174 = add i32 %Bridge_T21_166, %Chain_T24_169


  %Bridge_T4_64 = add i32 %Chain_T4_63, %Chain_T4_68
  %Bridge_T4_72 = add i32 %Bridge_T4_64, %Chain_T4_76
  %Bridge_T4_80 = add i32 %Bridge_T4_72, %Chain_T4_82


  %Bridge_T3_69 = sub i32 %Chain_T4_76, %Chain_T4_68
  %Bridge_T3_ = sub i32 %Bridge_T3_69, %Chain_T4_63
  %Bridge_T3_85 = add i32 %Bridge_T3_, %Chain_T4_82

  %Bridge_T2_ = sub i32 %Chain_T4_68, %Chain_T4_63
  %Bridge_T2_78 = sub i32 %Bridge_T2_, %Chain_T4_76
  %Bridge_T2_86 = add i32 %Bridge_T2_78, %Chain_T4_82

  %Bridge_T1_71 = sub i32 %Chain_T4_63, %Chain_T4_68
  %Bridge_T1_79 = sub i32 %Bridge_T1_71, %Chain_T4_76
  %Bridge_T1_87 = add i32 %Bridge_T1_79, %Chain_T4_82


  %arrayIdx1043 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 0
  %arrayIdx1045 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 2
  %arrayIdx1063 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 1
  %arrayIdx1081 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 3
  %arrayIdx1131 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 4
  %arrayIdx1133 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 6
  %arrayIdx1151 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 5
  %arrayIdx1169 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 7

  store i32 %Bridge_T24_167, i32* %arrayIdx1043, align 4
  store i32 %Bridge_T23_172, i32* %arrayIdx1045, align 4
  store i32 %Bridge_T22_173, i32* %arrayIdx1063, align 4
  store i32 %Bridge_T21_174, i32* %arrayIdx1081, align 4

  store i32 %Bridge_T4_80, i32* %arrayIdx1131, align 4
  store i32 %Bridge_T3_85, i32* %arrayIdx1133, align 4
  store i32 %Bridge_T2_86, i32* %arrayIdx1151, align 4
  store i32 %Bridge_T1_87, i32* %arrayIdx1169, align 4
  %nextivloop.1247 = add nuw nsw i64 %i1.i64.0, 1
  %condloop.1247 = icmp ult i64 %nextivloop.1247, 16
  br i1 %condloop.1247, label %loop.1247, label %afterloop.1247

afterloop.1247:                                   ; preds = %loop.1247
  ret i32 0
}
