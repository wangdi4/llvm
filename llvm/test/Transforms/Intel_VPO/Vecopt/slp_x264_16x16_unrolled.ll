; RUN: opt < %s -slp-vectorizer -mtriple=x86_64-unknown-linux-gnu -pslp -mcpu=skylake-avx512 -tti -S | FileCheck %s -check-prefix=16WIDE
; This test checks that we don't crash because of not unique values and still vectorize part of the tree.
; Full support for this case requires "head duplication".

source_filename = "x264_pixel_satd_16x16.ll"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readonly uwtable
define internal i32 @x264_pixel_satd_16x16(i8* nocapture readonly %pix1, i32 %i_pix1, i8* nocapture readonly %pix2, i32 %i_pix2) #4 {
entry:
  %alloca = alloca [16 x [8 x i32]], align 4
  %alloca933 = alloca [8 x i32], align 16
  %alloca934 = alloca [8 x i32], align 16
  %alloca935 = alloca [8 x i32], align 16
  %alloca936 = alloca [8 x i32], align 16
  br label %loop.1452

loop.1452:                                        ; preds = %loop.1452, %entry
  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.1452, %loop.1452 ]
  %0 = sext i32 %i_pix1 to i64
  %1 = shl nuw i64 %i1.i64.0, 1
  %2 = mul i64 %1, %0
  %arrayIdx = getelementptr inbounds i8, i8* %pix1, i64 %2
  %gepload = load i8, i8* %arrayIdx, align 1, !tbaa !21
  %3 = sext i32 %i_pix2 to i64
  %4 = mul i64 %1, %3
  %arrayIdx1020 = getelementptr inbounds i8, i8* %pix2, i64 %4
  %gepload1021 = load i8, i8* %arrayIdx1020, align 1, !tbaa !21
  %5 = add i64 %2, 4
  %arrayIdx1022 = getelementptr inbounds i8, i8* %pix1, i64 %5
  %gepload1023 = load i8, i8* %arrayIdx1022, align 1, !tbaa !21
  %6 = add i64 %4, 4
  %arrayIdx1024 = getelementptr inbounds i8, i8* %pix2, i64 %6
  %gepload1025 = load i8, i8* %arrayIdx1024, align 1, !tbaa !21
  %7 = or i64 %2, 1
  %arrayIdx1026 = getelementptr inbounds i8, i8* %pix1, i64 %7
  %gepload1027 = load i8, i8* %arrayIdx1026, align 1, !tbaa !21
  %8 = or i64 %4, 1
  %arrayIdx1028 = getelementptr inbounds i8, i8* %pix2, i64 %8
  %gepload1029 = load i8, i8* %arrayIdx1028, align 1, !tbaa !21
  %9 = add i64 %2, 5
  %arrayIdx1030 = getelementptr inbounds i8, i8* %pix1, i64 %9
  %gepload1031 = load i8, i8* %arrayIdx1030, align 1, !tbaa !21
  %10 = add i64 %4, 5
  %arrayIdx1032 = getelementptr inbounds i8, i8* %pix2, i64 %10
  %gepload1033 = load i8, i8* %arrayIdx1032, align 1, !tbaa !21
  %11 = add i64 %2, 2
  %arrayIdx1034 = getelementptr inbounds i8, i8* %pix1, i64 %11
  %gepload1035 = load i8, i8* %arrayIdx1034, align 1, !tbaa !21
  %12 = add i64 %4, 2
  %arrayIdx1036 = getelementptr inbounds i8, i8* %pix2, i64 %12
  %gepload1037 = load i8, i8* %arrayIdx1036, align 1, !tbaa !21
  %13 = add i64 %2, 6
  %arrayIdx1038 = getelementptr inbounds i8, i8* %pix1, i64 %13
  %gepload1039 = load i8, i8* %arrayIdx1038, align 1, !tbaa !21
  %14 = add i64 %4, 6
  %arrayIdx1040 = getelementptr inbounds i8, i8* %pix2, i64 %14
  %gepload1041 = load i8, i8* %arrayIdx1040, align 1, !tbaa !21
  %15 = add i64 %2, 3
  %arrayIdx1042 = getelementptr inbounds i8, i8* %pix1, i64 %15
  %gepload1043 = load i8, i8* %arrayIdx1042, align 1, !tbaa !21
  %16 = add i64 %4, 3
  %arrayIdx1044 = getelementptr inbounds i8, i8* %pix2, i64 %16
  %gepload1045 = load i8, i8* %arrayIdx1044, align 1, !tbaa !21
  %17 = add i64 %2, 7
  %arrayIdx1046 = getelementptr inbounds i8, i8* %pix1, i64 %17
  %gepload1047 = load i8, i8* %arrayIdx1046, align 1, !tbaa !21
  %18 = add i64 %4, 7
  %arrayIdx1048 = getelementptr inbounds i8, i8* %pix2, i64 %18
  %gepload1049 = load i8, i8* %arrayIdx1048, align 1, !tbaa !21
  %arrayIdx1050 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %1, i64 0
  %19 = zext i8 %gepload1043 to i32
  %20 = zext i8 %gepload1035 to i32
  %21 = zext i8 %gepload1027 to i32
  %22 = zext i8 %gepload to i32
  %23 = zext i8 %gepload1047 to i32
  %24 = zext i8 %gepload1039 to i32
  %25 = zext i8 %gepload1031 to i32
  %26 = zext i8 %gepload1023 to i32
  %27 = zext i8 %gepload1049 to i32
  %28 = zext i8 %gepload1041 to i32
  %29 = zext i8 %gepload1033 to i32
  %30 = zext i8 %gepload1025 to i32
  %31 = zext i8 %gepload1045 to i32
  %32 = zext i8 %gepload1037 to i32
  %33 = zext i8 %gepload1029 to i32
  %34 = zext i8 %gepload1021 to i32
  %35 = shl nsw i32 %26, 16
  %36 = shl nsw i32 %30, 16
  %37 = shl nsw i32 %25, 16
  %38 = shl nsw i32 %29, 16
  %Chain_T856_3327 = sub nsw i32 %37, %38
  %Chain_T856_3326 = sub nsw i32 %Chain_T856_3327, %33
  %Chain_T856_3329 = add nsw i32 %Chain_T856_3326, %21
  %Chain_T856_3323 = sub nsw i32 %35, %36
  %Chain_T856_3321 = sub nsw i32 %Chain_T856_3323, %34
  %Chain_T856_3322 = add nsw i32 %Chain_T856_3321, %22
  %39 = shl nsw i32 %24, 16
  %40 = shl nsw i32 %28, 16
  %Chain_T856_3316 = sub nsw i32 %39, %40
  %Chain_T856_3318 = sub nsw i32 %Chain_T856_3316, %32
  %Chain_T856_3319 = add nsw i32 %Chain_T856_3318, %20
  %41 = shl nsw i32 %23, 16
  %42 = shl nsw i32 %27, 16
  %Chain_T856_ = sub nsw i32 %41, %42
  %Chain_T856_3313 = sub nsw i32 %Chain_T856_, %31
  %Chain_T856_3314 = add nsw i32 %Chain_T856_3313, %19
  %Bridge_T856_3315 = add nsw i32 %Chain_T856_3314, %Chain_T856_3319
  %Bridge_T856_3320 = add nsw i32 %Bridge_T856_3315, %Chain_T856_3322
  %Bridge_T856_3325 = add nsw i32 %Bridge_T856_3320, %Chain_T856_3329
  store i32 %Bridge_T856_3325, i32* %arrayIdx1050, align 4
  %arrayIdx1052 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %1, i64 2
  %Bridge_T855_3295 = sub nsw i32 %Chain_T856_3322, %Chain_T856_3319
  %Bridge_T855_ = sub nsw i32 %Bridge_T855_3295, %Chain_T856_3314
  %Bridge_T855_3307 = add nsw i32 %Bridge_T855_, %Chain_T856_3329
  store i32 %Bridge_T855_3307, i32* %arrayIdx1052, align 4
  %arrayIdx1070 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %1, i64 1
  %Bridge_T851_ = sub nsw i32 %Chain_T856_3319, %Chain_T856_3314
  %Bridge_T851_3274 = add nsw i32 %Bridge_T851_, %Chain_T856_3322
  %Bridge_T851_3279 = sub nsw i32 %Bridge_T851_3274, %Chain_T856_3329
  store i32 %Bridge_T851_3279, i32* %arrayIdx1070, align 4
  %arrayIdx1088 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %1, i64 3
  %Bridge_T850_3240 = sub nsw i32 %Chain_T856_3314, %Chain_T856_3319
  %Bridge_T850_3249 = add nsw i32 %Bridge_T850_3240, %Chain_T856_3322
  %Bridge_T850_3254 = sub nsw i32 %Bridge_T850_3249, %Chain_T856_3329
  store i32 %Bridge_T850_3254, i32* %arrayIdx1088, align 4
  %43 = add i64 %2, 8
  %arrayIdx1105 = getelementptr inbounds i8, i8* %pix1, i64 %43
  %gepload1106 = load i8, i8* %arrayIdx1105, align 1, !tbaa !21
  %44 = add i64 %4, 8
  %arrayIdx1107 = getelementptr inbounds i8, i8* %pix2, i64 %44
  %gepload1108 = load i8, i8* %arrayIdx1107, align 1, !tbaa !21
  %45 = add i64 %2, 12
  %arrayIdx1109 = getelementptr inbounds i8, i8* %pix1, i64 %45
  %gepload1110 = load i8, i8* %arrayIdx1109, align 1, !tbaa !21
  %46 = add i64 %4, 12
  %arrayIdx1111 = getelementptr inbounds i8, i8* %pix2, i64 %46
  %gepload1112 = load i8, i8* %arrayIdx1111, align 1, !tbaa !21
  %47 = add i64 %2, 9
  %arrayIdx1113 = getelementptr inbounds i8, i8* %pix1, i64 %47
  %gepload1114 = load i8, i8* %arrayIdx1113, align 1, !tbaa !21
  %48 = add i64 %4, 9
  %arrayIdx1115 = getelementptr inbounds i8, i8* %pix2, i64 %48
  %gepload1116 = load i8, i8* %arrayIdx1115, align 1, !tbaa !21
  %49 = add i64 %2, 13
  %arrayIdx1117 = getelementptr inbounds i8, i8* %pix1, i64 %49
  %gepload1118 = load i8, i8* %arrayIdx1117, align 1, !tbaa !21
  %50 = add i64 %4, 13
  %arrayIdx1119 = getelementptr inbounds i8, i8* %pix2, i64 %50
  %gepload1120 = load i8, i8* %arrayIdx1119, align 1, !tbaa !21
  %51 = add i64 %2, 10
  %arrayIdx1121 = getelementptr inbounds i8, i8* %pix1, i64 %51
  %gepload1122 = load i8, i8* %arrayIdx1121, align 1, !tbaa !21
  %52 = add i64 %4, 10
  %arrayIdx1123 = getelementptr inbounds i8, i8* %pix2, i64 %52
  %gepload1124 = load i8, i8* %arrayIdx1123, align 1, !tbaa !21
  %53 = add i64 %2, 14
  %arrayIdx1125 = getelementptr inbounds i8, i8* %pix1, i64 %53
  %gepload1126 = load i8, i8* %arrayIdx1125, align 1, !tbaa !21
  %54 = add i64 %4, 14
  %arrayIdx1127 = getelementptr inbounds i8, i8* %pix2, i64 %54
  %gepload1128 = load i8, i8* %arrayIdx1127, align 1, !tbaa !21
  %55 = add i64 %2, 11
  %arrayIdx1129 = getelementptr inbounds i8, i8* %pix1, i64 %55
  %gepload1130 = load i8, i8* %arrayIdx1129, align 1, !tbaa !21
  %56 = add i64 %4, 11
  %arrayIdx1131 = getelementptr inbounds i8, i8* %pix2, i64 %56
  %gepload1132 = load i8, i8* %arrayIdx1131, align 1, !tbaa !21
  %57 = add i64 %2, 15
  %arrayIdx1133 = getelementptr inbounds i8, i8* %pix1, i64 %57
  %gepload1134 = load i8, i8* %arrayIdx1133, align 1, !tbaa !21
  %58 = add i64 %4, 15
  %arrayIdx1135 = getelementptr inbounds i8, i8* %pix2, i64 %58
  %gepload1136 = load i8, i8* %arrayIdx1135, align 1, !tbaa !21
  %arrayIdx1138 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %1, i64 4
  %59 = zext i8 %gepload1130 to i32
  %60 = zext i8 %gepload1122 to i32
  %61 = zext i8 %gepload1114 to i32
  %62 = zext i8 %gepload1106 to i32
  %63 = zext i8 %gepload1134 to i32
  %64 = zext i8 %gepload1126 to i32
  %65 = zext i8 %gepload1118 to i32
  %66 = zext i8 %gepload1110 to i32
  %67 = zext i8 %gepload1136 to i32
  %68 = zext i8 %gepload1128 to i32
  %69 = zext i8 %gepload1120 to i32
  %70 = zext i8 %gepload1112 to i32
  %71 = zext i8 %gepload1132 to i32
  %72 = zext i8 %gepload1124 to i32
  %73 = zext i8 %gepload1116 to i32
  %74 = zext i8 %gepload1108 to i32
  %75 = shl nsw i32 %66, 16
  %76 = shl nsw i32 %70, 16
  %77 = shl nsw i32 %65, 16
  %78 = shl nsw i32 %69, 16
  %Chain_T829_3174 = sub nsw i32 %77, %78
  %Chain_T829_3173 = sub nsw i32 %Chain_T829_3174, %73
  %Chain_T829_3176 = add nsw i32 %Chain_T829_3173, %61
  %Chain_T829_3170 = sub nsw i32 %75, %76
  %Chain_T829_3168 = sub nsw i32 %Chain_T829_3170, %74
  %Chain_T829_3169 = add nsw i32 %Chain_T829_3168, %62
  %79 = shl nsw i32 %64, 16
  %80 = shl nsw i32 %68, 16
  %Chain_T829_3163 = sub nsw i32 %79, %80
  %Chain_T829_3165 = sub nsw i32 %Chain_T829_3163, %72
  %Chain_T829_3166 = add nsw i32 %Chain_T829_3165, %60
  %81 = shl nsw i32 %63, 16
  %82 = shl nsw i32 %67, 16
  %Chain_T829_ = sub nsw i32 %81, %82
  %Chain_T829_3160 = sub nsw i32 %Chain_T829_, %71
  %Chain_T829_3161 = add nsw i32 %Chain_T829_3160, %59
  %Bridge_T829_3162 = add nsw i32 %Chain_T829_3161, %Chain_T829_3166
  %Bridge_T829_3167 = add nsw i32 %Bridge_T829_3162, %Chain_T829_3169
  %Bridge_T829_3172 = add nsw i32 %Bridge_T829_3167, %Chain_T829_3176
  store i32 %Bridge_T829_3172, i32* %arrayIdx1138, align 4
  %arrayIdx1140 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %1, i64 6
  %Bridge_T828_3142 = sub nsw i32 %Chain_T829_3169, %Chain_T829_3166
  %Bridge_T828_ = sub nsw i32 %Bridge_T828_3142, %Chain_T829_3161
  %Bridge_T828_3154 = add nsw i32 %Bridge_T828_, %Chain_T829_3176
  store i32 %Bridge_T828_3154, i32* %arrayIdx1140, align 4
  %arrayIdx1158 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %1, i64 5
  %Bridge_T824_ = sub nsw i32 %Chain_T829_3166, %Chain_T829_3161
  %Bridge_T824_3121 = add nsw i32 %Bridge_T824_, %Chain_T829_3169
  %Bridge_T824_3126 = sub nsw i32 %Bridge_T824_3121, %Chain_T829_3176
  store i32 %Bridge_T824_3126, i32* %arrayIdx1158, align 4
  %arrayIdx1176 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %1, i64 7
  %Bridge_T823_3087 = sub nsw i32 %Chain_T829_3161, %Chain_T829_3166
  %Bridge_T823_3096 = add nsw i32 %Bridge_T823_3087, %Chain_T829_3169
  %Bridge_T823_3101 = sub nsw i32 %Bridge_T823_3096, %Chain_T829_3176
  store i32 %Bridge_T823_3101, i32* %arrayIdx1176, align 4
  %83 = add i64 %2, %0
  %arrayIdx1193 = getelementptr inbounds i8, i8* %pix1, i64 %83
  %gepload1194 = load i8, i8* %arrayIdx1193, align 1, !tbaa !21
  %84 = add i64 %4, %3
  %arrayIdx1195 = getelementptr inbounds i8, i8* %pix2, i64 %84
  %gepload1196 = load i8, i8* %arrayIdx1195, align 1, !tbaa !21
  %Bridge_T820_ = add nsw i64 4, %83
  %arrayIdx1197 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T820_
  %gepload1198 = load i8, i8* %arrayIdx1197, align 1, !tbaa !21
  %Bridge_T819_ = add nsw i64 4, %84
  %arrayIdx1199 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T819_
  %gepload1200 = load i8, i8* %arrayIdx1199, align 1, !tbaa !21
  %Bridge_T818_ = add nsw i64 1, %83
  %arrayIdx1201 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T818_
  %gepload1202 = load i8, i8* %arrayIdx1201, align 1, !tbaa !21
  %Bridge_T817_ = add nsw i64 1, %84
  %arrayIdx1203 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T817_
  %gepload1204 = load i8, i8* %arrayIdx1203, align 1, !tbaa !21
  %Bridge_T816_ = add nsw i64 5, %83
  %arrayIdx1205 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T816_
  %gepload1206 = load i8, i8* %arrayIdx1205, align 1, !tbaa !21
  %Bridge_T815_ = add nsw i64 5, %84
  %arrayIdx1207 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T815_
  %gepload1208 = load i8, i8* %arrayIdx1207, align 1, !tbaa !21
  %Bridge_T814_ = add nsw i64 2, %83
  %arrayIdx1209 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T814_
  %gepload1210 = load i8, i8* %arrayIdx1209, align 1, !tbaa !21
  %Bridge_T813_ = add nsw i64 2, %84
  %arrayIdx1211 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T813_
  %gepload1212 = load i8, i8* %arrayIdx1211, align 1, !tbaa !21
  %Bridge_T812_ = add nsw i64 6, %83
  %arrayIdx1213 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T812_
  %gepload1214 = load i8, i8* %arrayIdx1213, align 1, !tbaa !21
  %Bridge_T811_ = add nsw i64 6, %84
  %arrayIdx1215 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T811_
  %gepload1216 = load i8, i8* %arrayIdx1215, align 1, !tbaa !21
  %Bridge_T810_ = add nsw i64 3, %83
  %arrayIdx1217 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T810_
  %gepload1218 = load i8, i8* %arrayIdx1217, align 1, !tbaa !21
  %Bridge_T809_ = add nsw i64 3, %84
  %arrayIdx1219 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T809_
  %gepload1220 = load i8, i8* %arrayIdx1219, align 1, !tbaa !21
  %Bridge_T808_ = add nsw i64 7, %83
  %arrayIdx1221 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T808_
  %gepload1222 = load i8, i8* %arrayIdx1221, align 1, !tbaa !21
  %Bridge_T807_ = add nsw i64 7, %84
  %arrayIdx1223 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T807_
  %gepload1224 = load i8, i8* %arrayIdx1223, align 1, !tbaa !21
  %85 = or i64 %1, 1
  %arrayIdx1226 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %85, i64 0
  %86 = zext i8 %gepload1218 to i32
  %87 = zext i8 %gepload1210 to i32
  %88 = zext i8 %gepload1202 to i32
  %89 = zext i8 %gepload1194 to i32
  %90 = zext i8 %gepload1222 to i32
  %91 = zext i8 %gepload1214 to i32
  %92 = zext i8 %gepload1206 to i32
  %93 = zext i8 %gepload1198 to i32
  %94 = zext i8 %gepload1224 to i32
  %95 = zext i8 %gepload1216 to i32
  %96 = zext i8 %gepload1208 to i32
  %97 = zext i8 %gepload1200 to i32
  %98 = zext i8 %gepload1220 to i32
  %99 = zext i8 %gepload1212 to i32
  %100 = zext i8 %gepload1204 to i32
  %101 = zext i8 %gepload1196 to i32
  %102 = shl nsw i32 %93, 16
  %103 = shl nsw i32 %97, 16
  %104 = shl nsw i32 %92, 16
  %105 = shl nsw i32 %96, 16
  %Chain_T802_29342939 = sub nsw i32 %105, %104
  %Chain_T802_29352937 = sub nsw i32 %Chain_T802_29342939, %88
  %Chain_T802_2936 = add nsw i32 %Chain_T802_29352937, %100
  %Chain_T802_29272931 = sub nsw i32 %103, %102
  %Chain_T802_29252929 = sub nsw i32 %Chain_T802_29272931, %89
  %Chain_T802_2928 = add nsw i32 %Chain_T802_29252929, %101
  %106 = shl nsw i32 %91, 16
  %107 = shl nsw i32 %95, 16
  %Chain_T802_29172923 = sub nsw i32 %107, %106
  %Chain_T802_29192921 = sub nsw i32 %Chain_T802_29172923, %87
  %Chain_T802_29182920 = add nsw i32 %Chain_T802_29192921, %99
  %108 = shl nsw i32 %90, 16
  %109 = shl nsw i32 %94, 16
  %Chain_T802_29092915 = sub nsw i32 %109, %108
  %Chain_T802_29112913 = sub nsw i32 %Chain_T802_29092915, %86
  %Chain_T802_29102912 = add nsw i32 %Chain_T802_29112913, %98
  %Bridge_T802_29162940 = add nsw i32 %Chain_T802_29102912, %Chain_T802_29182920
  %Bridge_T802_29242941 = add nsw i32 %Bridge_T802_29162940, %Chain_T802_2928
  %Bridge_T802_2932 = sub nsw i32 %Bridge_T802_29242941, %Chain_T802_2936
  store i32 %Bridge_T802_2932, i32* %arrayIdx1226, align 4
  %arrayIdx1244 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %85, i64 2
  %Bridge_T801_2893 = sub nsw i32 %Bridge_T802_29162940, %Chain_T802_2928
  %Bridge_T801_2901 = sub nsw i32 %Bridge_T801_2893, %Chain_T802_2936
  store i32 %Bridge_T801_2901, i32* %arrayIdx1244, align 4
  %arrayIdx1262 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %85, i64 1
  %Bridge_T797_2968 = sub nsw i32 %Chain_T802_29102912, %Chain_T802_29182920
  %Bridge_T797_2976 = sub nsw i32 %Bridge_T797_2968, %Chain_T802_2928
  %Bridge_T797_2984 = add nsw i32 %Bridge_T797_2976, %Chain_T802_2936
  store i32 %Bridge_T797_2984, i32* %arrayIdx1262, align 4
  %arrayIdx1280 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %85, i64 3
  %Bridge_T796_ = sub nsw i32 %Chain_T802_29182920, %Chain_T802_29102912
  %Bridge_T796_2953 = sub nsw i32 %Bridge_T796_, %Chain_T802_2928
  %Bridge_T796_2961 = add nsw i32 %Bridge_T796_2953, %Chain_T802_2936
  store i32 %Bridge_T796_2961, i32* %arrayIdx1280, align 4
  %Bridge_T795_ = add nsw i64 8, %83
  %arrayIdx1297 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T795_
  %gepload1298 = load i8, i8* %arrayIdx1297, align 1, !tbaa !21
  %Bridge_T794_ = add nsw i64 8, %84
  %arrayIdx1299 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T794_
  %gepload1300 = load i8, i8* %arrayIdx1299, align 1, !tbaa !21
  %Bridge_T793_ = add nsw i64 12, %83
  %arrayIdx1301 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T793_
  %gepload1302 = load i8, i8* %arrayIdx1301, align 1, !tbaa !21
  %Bridge_T792_ = add nsw i64 12, %84
  %arrayIdx1303 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T792_
  %gepload1304 = load i8, i8* %arrayIdx1303, align 1, !tbaa !21
  %Bridge_T791_ = add nsw i64 9, %83
  %arrayIdx1305 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T791_
  %gepload1306 = load i8, i8* %arrayIdx1305, align 1, !tbaa !21
  %Bridge_T790_ = add nsw i64 9, %84
  %arrayIdx1307 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T790_
  %gepload1308 = load i8, i8* %arrayIdx1307, align 1, !tbaa !21
  %Bridge_T789_ = add nsw i64 13, %83
  %arrayIdx1309 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T789_
  %gepload1310 = load i8, i8* %arrayIdx1309, align 1, !tbaa !21
  %Bridge_T788_ = add nsw i64 13, %84
  %arrayIdx1311 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T788_
  %gepload1312 = load i8, i8* %arrayIdx1311, align 1, !tbaa !21
  %Bridge_T787_ = add nsw i64 10, %83
  %arrayIdx1313 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T787_
  %gepload1314 = load i8, i8* %arrayIdx1313, align 1, !tbaa !21
  %Bridge_T786_ = add nsw i64 10, %84
  %arrayIdx1315 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T786_
  %gepload1316 = load i8, i8* %arrayIdx1315, align 1, !tbaa !21
  %Bridge_T785_ = add nsw i64 14, %83
  %arrayIdx1317 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T785_
  %gepload1318 = load i8, i8* %arrayIdx1317, align 1, !tbaa !21
  %Bridge_T784_ = add nsw i64 14, %84
  %arrayIdx1319 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T784_
  %gepload1320 = load i8, i8* %arrayIdx1319, align 1, !tbaa !21
  %Bridge_T783_ = add nsw i64 11, %83
  %arrayIdx1321 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T783_
  %gepload1322 = load i8, i8* %arrayIdx1321, align 1, !tbaa !21
  %Bridge_T782_ = add nsw i64 11, %84
  %arrayIdx1323 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T782_
  %gepload1324 = load i8, i8* %arrayIdx1323, align 1, !tbaa !21
  %Bridge_T781_ = add nsw i64 15, %83
  %arrayIdx1325 = getelementptr inbounds i8, i8* %pix1, i64 %Bridge_T781_
  %gepload1326 = load i8, i8* %arrayIdx1325, align 1, !tbaa !21
  %Bridge_T780_ = add nsw i64 15, %84
  %arrayIdx1327 = getelementptr inbounds i8, i8* %pix2, i64 %Bridge_T780_
  %gepload1328 = load i8, i8* %arrayIdx1327, align 1, !tbaa !21
  %arrayIdx1330 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %85, i64 4
  %110 = zext i8 %gepload1322 to i32
  %111 = zext i8 %gepload1314 to i32
  %112 = zext i8 %gepload1306 to i32
  %113 = zext i8 %gepload1298 to i32
  %114 = zext i8 %gepload1326 to i32
  %115 = zext i8 %gepload1318 to i32
  %116 = zext i8 %gepload1310 to i32
  %117 = zext i8 %gepload1302 to i32
  %118 = zext i8 %gepload1328 to i32
  %119 = zext i8 %gepload1320 to i32
  %120 = zext i8 %gepload1312 to i32
  %121 = zext i8 %gepload1304 to i32
  %122 = zext i8 %gepload1324 to i32
  %123 = zext i8 %gepload1316 to i32
  %124 = zext i8 %gepload1308 to i32
  %125 = zext i8 %gepload1300 to i32
  %126 = shl nsw i32 %117, 16
  %127 = shl nsw i32 %121, 16
  %128 = shl nsw i32 %116, 16
  %129 = shl nsw i32 %120, 16
  %Chain_T775_2769 = sub nsw i32 %128, %129
  %Chain_T775_2768 = sub nsw i32 %Chain_T775_2769, %124
  %Chain_T775_2771 = add nsw i32 %Chain_T775_2768, %112
  %Chain_T775_2765 = sub nsw i32 %126, %127
  %Chain_T775_2763 = sub nsw i32 %Chain_T775_2765, %125
  %Chain_T775_2764 = add nsw i32 %Chain_T775_2763, %113
  %130 = shl nsw i32 %115, 16
  %131 = shl nsw i32 %119, 16
  %Chain_T775_2758 = sub nsw i32 %130, %131
  %Chain_T775_2760 = sub nsw i32 %Chain_T775_2758, %123
  %Chain_T775_2761 = add nsw i32 %Chain_T775_2760, %111
  %132 = shl nsw i32 %114, 16
  %133 = shl nsw i32 %118, 16
  %Chain_T775_ = sub nsw i32 %132, %133
  %Chain_T775_2755 = sub nsw i32 %Chain_T775_, %122
  %Chain_T775_2756 = add nsw i32 %Chain_T775_2755, %110
  %Bridge_T775_2757 = add nsw i32 %Chain_T775_2756, %Chain_T775_2761
  %Bridge_T775_2762 = add nsw i32 %Bridge_T775_2757, %Chain_T775_2764
  %Bridge_T775_2767 = add nsw i32 %Bridge_T775_2762, %Chain_T775_2771
  store i32 %Bridge_T775_2767, i32* %arrayIdx1330, align 4
  %arrayIdx1348 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %85, i64 6
  %Bridge_T774_2737 = sub nsw i32 %Chain_T775_2764, %Chain_T775_2761
  %Bridge_T774_ = sub nsw i32 %Bridge_T774_2737, %Chain_T775_2756
  %Bridge_T774_2749 = add nsw i32 %Bridge_T774_, %Chain_T775_2771
  store i32 %Bridge_T774_2749, i32* %arrayIdx1348, align 4
  %arrayIdx1366 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %85, i64 5
  %Bridge_T770_ = sub nsw i32 %Chain_T775_2761, %Chain_T775_2756
  %Bridge_T770_2716 = add nsw i32 %Bridge_T770_, %Chain_T775_2764
  %Bridge_T770_2721 = sub nsw i32 %Bridge_T770_2716, %Chain_T775_2771
  store i32 %Bridge_T770_2721, i32* %arrayIdx1366, align 4
  %arrayIdx1384 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %85, i64 7
  %Bridge_T769_2682 = sub nsw i32 %Chain_T775_2756, %Chain_T775_2761
  %Bridge_T769_2691 = add nsw i32 %Bridge_T769_2682, %Chain_T775_2764
  %Bridge_T769_2696 = sub nsw i32 %Bridge_T769_2691, %Chain_T775_2771
  store i32 %Bridge_T769_2696, i32* %arrayIdx1384, align 4
  %nextivloop.1452 = add nuw nsw i64 %i1.i64.0, 1
  %condloop.1452 = icmp ult i64 %nextivloop.1452, 8
  br i1 %condloop.1452, label %loop.1452, label %afterloop.1452

afterloop.1452:                                   ; preds = %loop.1452
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #4 = { nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #5 = { nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #7 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #8 = { norecurse nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #9 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!21 = !{!6, !6, i64 0}


; There should be 16 in total 4-wide loads (without load coalescing).
; Just check for a few of them.


; 16WIDE: sub <16 x i32>
; 16WIDE: add <16 x i32>
; 16WIDE: add <16 x i32>
; 16WIDE: sub <16 x i32>
; 16WIDE: store <16 x i32> [[SEL1:%.*]], <16 x i32>*

