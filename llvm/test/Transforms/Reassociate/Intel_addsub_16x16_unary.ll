; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=true -addsub-reassoc-simplify-chains=true -addsub-reassoc-reuse-chain=true -addsub-reassoc-memcan-enable-unary-associations=true -addsub-reassoc-unshare-leaves=true -S | FileCheck %s -check-prefix=CHECK_UNSHARE
; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=true -addsub-reassoc-simplify-chains=true -addsub-reassoc-reuse-chain=true -addsub-reassoc-memcan-enable-unary-associations=true -addsub-reassoc-unshare-leaves=false -S | FileCheck %s -check-prefix=CHECK_UNARY_ASSOC
; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=true -addsub-reassoc-simplify-chains=true -addsub-reassoc-reuse-chain=true -addsub-reassoc-memcan-enable-unary-associations=false -addsub-reassoc-unshare-leaves=false -S | FileCheck %s -check-prefix=CHECK_CHAIN_REUSE
; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=true -addsub-reassoc-simplify-chains=true -addsub-reassoc-reuse-chain=false -addsub-reassoc-memcan-enable-unary-associations=false -addsub-reassoc-unshare-leaves=false -S | FileCheck %s -check-prefix=CHECK_SIMP
; RUN: opt < %s -addsub-reassoc -addsub-reassoc-simplify-trunks=false -addsub-reassoc-simplify-chains=false -addsub-reassoc-reuse-chain=false -addsub-reassoc-memcan-enable-unary-associations=false -addsub-reassoc-unshare-leaves=false -S | FileCheck %s -check-prefix=CHECK

; This is a test for AddSubReassoc pass to check that it kicks in for satd_16x16 like pattern.

; CHECK_UNSHARE: [[Chain_T24_187:%.*]] = sub i32 [[l44:%.*]], [[l40:%.*]]
; CHECK_UNSHARE: [[Chain_T24_185:%.*]] = add i32 [[Chain_T24_187]], [[l16:%.*]]
; CHECK_UNSHARE: [[Chain1_3:%.*]] = sub i32 [[Chain_T24_185]], [[l28:%.*]]
; CHECK_UNSHARE: [[Chain_T24_174178:%.*]] = sub i32 [[l41:%.*]], [[l45:%.*]]
; CHECK_UNSHARE: [[Chain_T24_173177:%.*]] = sub i32 [[Chain_T24_174178]], [[l17:%.*]]
; CHECK_UNSHARE: [[Chain2_3:%.*]] = add i32 [[Chain_T24_173177]], [[l29:%.*]]
; CHECK_UNSHARE: [[Chain_T24_162166:%.*]] = sub i32 [[l42:%.*]], [[l46:%.*]]
; CHECK_UNSHARE: [[Chain_T24_161165:%.*]] = sub i32 [[Chain_T24_162166]], [[l18:%.*]]
; CHECK_UNSHARE: [[Chain3_3:%.*]] = add i32 [[Chain_T24_161165]], [[l30:%.*]]
; CHECK_UNSHARE: [[Chain_T24_158:%.*]] = sub i32 [[l47:%.*]], [[l43:%.*]]
; CHECK_UNSHARE: [[Chain_T24_156:%.*]] = add i32 [[Chain_T24_158]], [[l19:%.*]]
; CHECK_UNSHARE: [[Chain4_3:%.*]] = sub i32 [[Chain_T24_156]], [[l31:%.*]]
; CHECK_UNSHARE: [[Bridge1_1:%.*]] = sub i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_UNSHARE: [[Bridge1_2:%.*]] = sub i32 [[Bridge1_1]], [[Chain2_3]]
; CHECK_UNSHARE: [[Bridge1_3:%.*]] = add i32 [[Bridge1_2]], [[Chain1_3]]
; CHECK_UNSHARE: store i32 [[Bridge1_3]]
; CHECK_UNSHARE: [[Bridge2_1:%.*]] = sub i32 [[Chain3_3]], [[Chain4_3]]
; CHECK_UNSHARE: [[Bridge2_2:%.*]] = sub i32 [[Bridge2_1]], [[Chain2_3]]
; CHECK_UNSHARE: [[Bridge2_3:%.*]] = add i32 [[Bridge2_2]], [[Chain1_3]]
; CHECK_UNSHARE: store i32 [[Bridge2_3]]
; CHECK_UNSHARE: [[Bridge3_1:%.*]] = sub i32 [[Chain2_3]], [[Chain3_3]]
; CHECK_UNSHARE: [[Bridge3_2:%.*]] = sub i32 [[Bridge3_1]], [[Chain4_3]]
; CHECK_UNSHARE: [[Bridge3_3:%.*]] = add i32 [[Bridge3_2]], [[Chain1_3]]
; CHECK_UNSHARE: store i32 [[Bridge3_3]]
; CHECK_UNSHARE: [[Bridge4_1:%.*]] = add i32 [[Chain4_3]], [[Chain3_3]]
; CHECK_UNSHARE: [[Bridge4_2:%.*]] = add i32 [[Bridge4_1]], [[Chain2_3]]
; CHECK_UNSHARE: [[Bridge4_3:%.*]] = add i32 [[Bridge4_2]], [[Chain1_3]]
; CHECK_UNSHARE: store i32 [[Bridge4_3]]

; CHECK_UNARY_ASSOC:  [[Chain0_1:%.*]] = sub i32 [[l27:%27]], [[l19:%.*]]
; CHECK_UNARY_ASSOC:  [[Chain0_2:%.*]] = sub i32 [[Chain0_1]], [[l35:%.*]]
; CHECK_UNARY_ASSOC:  [[Chain0_3:%.*]] = add i32 [[Chain0_2]], [[l37:%.*]]
; CHECK_UNARY_ASSOC:  [[Chain1_1:%.*]] = sub i32 [[l18:%.*]], [[l26:%.*]]
; CHECK_UNARY_ASSOC:  [[Chain1_2:%.*]] = add i32 [[Chain1_1]], [[l38:%.*]]
; CHECK_UNARY_ASSOC:  [[Chain1_3:%.*]] = sub i32 [[Chain1_2]], [[l39:%.*]]
; CHECK_UNARY_ASSOC:  [[Bridge0_0:%.*]] = add i32 [[Trunk0_1:%.*]], [[Chain1_3]]
; CHECK_UNARY_ASSOC:  [[Bridge0_1:%.*]] = sub i32 [[Bridge0_0]], [[Chain0_3]]
; CHECK_UNARY_ASSOC: store i32 [[Bridge0_1]]

; CHECK_CHAIN_REUSE:  [[Chain0_1:%.*]] = sub i32 [[l57:%.*]], [[l65:%.*]]
; CHECK_CHAIN_REUSE:  [[Chain1_1:%.*]] = sub i32 [[l64:%.*]], [[l56:%.*]]
; CHECK_CHAIN_REUSE:  [[Bridge0_0:%.*]] = add i32 [[reass_add1995:%.*]], [[Chain1_1]]
; CHECK_CHAIN_REUSE:  [[Bridge0_1:%.*]] = sub i32 [[Bridge0_0]], [[Chain0_1]]

; CHECK_SIMP:  [[Chain0_1:%.*]] = sub i32 [[l57:%.*]], [[l65:%.*]]
; CHECK_SIMP:  [[Chain1_1:%.*]] = sub i32 [[l64:%.*]], [[l56:%.*]]
; CHECK_SIMP:  [[Bridge0_0:%.*]] = add i32 [[reass_add1995:%.*]], [[Chain1_1]]
; CHECK_SIMP:  [[Bridge0_1:%.*]] = sub i32 [[Bridge0_0]], [[Chain0_1]]

; CHECK:  [[Trunk0_0:%.*]] = add i32 0, [[reass_add1995:%.*]]
; CHECK:  [[Chain0_0:%.*]] = sub i32 0, [[l65:%.*]]
; CHECK:  [[Chain0_1:%.*]] = add i32 [[Chain0_0]], [[l57:%.*]]
; CHECK:  [[Chain1_0:%.*]] = add i32 0, [[l64:%.*]]
; CHECK:  [[Chain1_1:%.*]] = sub i32 [[Chain1_0]], [[l56:%.*]]
; CHECK:  [[Bridge0_0:%.*]] = add i32 [[Trunk0_0]], [[Chain1_1]]
; CHECK:  [[Bridge0_1:%.*]] = sub i32 [[Bridge0_0]], [[Chain0_1]]

define dso_local i32 @x264_pixel_satd_16x16(i8* nocapture readonly %pix1, i32 %i_pix1, i8* nocapture readonly %pix2, i32 %i_pix2) #2 {
entry:
  %alloca = alloca [16 x [8 x i32]], align 4
  %alloca933 = alloca [8 x i32], align 16
  %alloca934 = alloca [8 x i32], align 16
  %alloca935 = alloca [8 x i32], align 16
  %alloca936 = alloca [8 x i32], align 16
  br label %loop.1247

loop.1247:                                        ; preds = %loop.1247, %entry
  %i1.i64.0 = phi i64 [ 0, %entry ], [ %nextivloop.1247, %loop.1247 ]
  %0 = sext i32 %i_pix1 to i64
  %1 = mul i64 %i1.i64.0, %0
  %arrayIdx = getelementptr inbounds i8, i8* %pix1, i64 %1
  %gepload = load i8, i8* %arrayIdx, align 1, !tbaa !21
  %2 = sext i32 %i_pix2 to i64
  %3 = mul i64 %i1.i64.0, %2
  %arrayIdx1013 = getelementptr inbounds i8, i8* %pix2, i64 %3
  %gepload1014 = load i8, i8* %arrayIdx1013, align 1, !tbaa !21
  %4 = add i64 %1, 4
  %arrayIdx1015 = getelementptr inbounds i8, i8* %pix1, i64 %4
  %gepload1016 = load i8, i8* %arrayIdx1015, align 1, !tbaa !21
  %5 = add i64 %3, 4
  %arrayIdx1017 = getelementptr inbounds i8, i8* %pix2, i64 %5
  %gepload1018 = load i8, i8* %arrayIdx1017, align 1, !tbaa !21
  %6 = add i64 %1, 1
  %arrayIdx1019 = getelementptr inbounds i8, i8* %pix1, i64 %6
  %gepload1020 = load i8, i8* %arrayIdx1019, align 1, !tbaa !21
  %7 = add i64 %3, 1
  %arrayIdx1021 = getelementptr inbounds i8, i8* %pix2, i64 %7
  %gepload1022 = load i8, i8* %arrayIdx1021, align 1, !tbaa !21
  %8 = add i64 %1, 5
  %arrayIdx1023 = getelementptr inbounds i8, i8* %pix1, i64 %8
  %gepload1024 = load i8, i8* %arrayIdx1023, align 1, !tbaa !21
  %9 = add i64 %3, 5
  %arrayIdx1025 = getelementptr inbounds i8, i8* %pix2, i64 %9
  %gepload1026 = load i8, i8* %arrayIdx1025, align 1, !tbaa !21
  %10 = add i64 %1, 2
  %arrayIdx1027 = getelementptr inbounds i8, i8* %pix1, i64 %10
  %gepload1028 = load i8, i8* %arrayIdx1027, align 1, !tbaa !21
  %11 = add i64 %3, 2
  %arrayIdx1029 = getelementptr inbounds i8, i8* %pix2, i64 %11
  %gepload1030 = load i8, i8* %arrayIdx1029, align 1, !tbaa !21
  %12 = add i64 %1, 6
  %arrayIdx1031 = getelementptr inbounds i8, i8* %pix1, i64 %12
  %gepload1032 = load i8, i8* %arrayIdx1031, align 1, !tbaa !21
  %13 = add i64 %3, 6
  %arrayIdx1033 = getelementptr inbounds i8, i8* %pix2, i64 %13
  %gepload1034 = load i8, i8* %arrayIdx1033, align 1, !tbaa !21
  %14 = add i64 %1, 3
  %arrayIdx1035 = getelementptr inbounds i8, i8* %pix1, i64 %14
  %gepload1036 = load i8, i8* %arrayIdx1035, align 1, !tbaa !21
  %15 = add i64 %3, 3
  %arrayIdx1037 = getelementptr inbounds i8, i8* %pix2, i64 %15
  %gepload1038 = load i8, i8* %arrayIdx1037, align 1, !tbaa !21
  %16 = add i64 %1, 7
  %arrayIdx1039 = getelementptr inbounds i8, i8* %pix1, i64 %16
  %gepload1040 = load i8, i8* %arrayIdx1039, align 1, !tbaa !21
  %17 = add i64 %3, 7
  %arrayIdx1041 = getelementptr inbounds i8, i8* %pix2, i64 %17
  %gepload1042 = load i8, i8* %arrayIdx1041, align 1, !tbaa !21
  %arrayIdx1043 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 0
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
  %reass.add = sub nsw i32 %33, %25
  %reass.add1931 = add nsw i32 %reass.add, %32
  %reass.add1932 = sub nsw i32 %reass.add1931, %24
  %reass.add1933 = add nsw i32 %reass.add1932, %31
  %reass.add1934 = sub nsw i32 %reass.add1933, %23
  %reass.add1935 = add nsw i32 %reass.add1934, %30
  %reass.add1936 = sub i32 %reass.add1935, %22
  %reass.mul = shl i32 %reass.add1936, 16
  %reass.add1937 = add nuw nsw i32 %28, %29
  %reass.add1938 = add nuw nsw i32 %reass.add1937, %27
  %reass.add1939 = add nuw nsw i32 %reass.add1938, %26
  %34 = add nuw nsw i32 %20, %21
  %35 = add nuw nsw i32 %34, %19
  %36 = add nuw nsw i32 %35, %18
  %37 = sub nsw i32 %36, %reass.add1939
  %38 = add i32 %37, %reass.mul
  store i32 %38, i32* %arrayIdx1043, align 4
  %arrayIdx1045 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 2
  %reass.add1948 = sub nsw i32 %reass.add1932, %31
  %reass.add1949 = add nsw i32 %reass.add1948, %23
  %reass.add1950 = sub nsw i32 %reass.add1949, %30
  %reass.add1951 = add i32 %reass.add1950, %22
  %reass.mul1952 = shl i32 %reass.add1951, 16
  %reass.add1954 = add nuw nsw i32 %reass.add1937, %19
  %reass.add1955 = add nuw nsw i32 %reass.add1954, %18
  %39 = add nuw nsw i32 %34, %27
  %40 = add nuw nsw i32 %39, %26
  %41 = sub nsw i32 %40, %reass.add1955
  %42 = add i32 %41, %reass.mul1952
  store i32 %42, i32* %arrayIdx1045, align 4
  %arrayIdx1063 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 1
  %reass.add1962 = sub nsw i32 %reass.add, %32
  %reass.add1963 = add nsw i32 %reass.add1962, %24
  %reass.add1964 = add nsw i32 %reass.add1963, %31
  %reass.add1965 = sub nsw i32 %reass.add1964, %23
  %reass.add1966 = sub nsw i32 %reass.add1965, %30
  %reass.add1967 = add i32 %reass.add1966, %22
  %reass.mul1968 = shl i32 %reass.add1967, 16
  %reass.add1969 = add nuw nsw i32 %20, %29
  %reass.add1970 = add nuw nsw i32 %reass.add1969, %27
  %reass.add1971 = add nuw nsw i32 %reass.add1970, %18
  %43 = add nuw nsw i32 %28, %21
  %44 = add nuw nsw i32 %43, %19
  %45 = add nuw nsw i32 %44, %26
  %46 = sub nsw i32 %45, %reass.add1971
  %47 = add i32 %46, %reass.mul1968
  store i32 %47, i32* %arrayIdx1063, align 4
  %arrayIdx1081 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 3
  %reass.add1980 = sub nsw i32 %reass.add1963, %31
  %reass.add1981 = add nsw i32 %reass.add1980, %23
  %reass.add1982 = add nsw i32 %reass.add1981, %30
  %reass.add1983 = sub i32 %reass.add1982, %22
  %reass.mul1984 = shl i32 %reass.add1983, 16
  %reass.add1986 = add nuw nsw i32 %reass.add1969, %19
  %reass.add1987 = add nuw nsw i32 %reass.add1986, %26
  %48 = add nuw nsw i32 %43, %27
  %49 = add nuw nsw i32 %48, %18
  %50 = sub nsw i32 %49, %reass.add1987
  %51 = add i32 %50, %reass.mul1984
  store i32 %51, i32* %arrayIdx1081, align 4
  %52 = add i64 %1, 8
  %arrayIdx1098 = getelementptr inbounds i8, i8* %pix1, i64 %52
  %gepload1099 = load i8, i8* %arrayIdx1098, align 1, !tbaa !21
  %53 = add i64 %3, 8
  %arrayIdx1100 = getelementptr inbounds i8, i8* %pix2, i64 %53
  %gepload1101 = load i8, i8* %arrayIdx1100, align 1, !tbaa !21
  %54 = add i64 %1, 12
  %arrayIdx1102 = getelementptr inbounds i8, i8* %pix1, i64 %54
  %gepload1103 = load i8, i8* %arrayIdx1102, align 1, !tbaa !21
  %55 = add i64 %3, 12
  %arrayIdx1104 = getelementptr inbounds i8, i8* %pix2, i64 %55
  %gepload1105 = load i8, i8* %arrayIdx1104, align 1, !tbaa !21
  %56 = add i64 %1, 9
  %arrayIdx1106 = getelementptr inbounds i8, i8* %pix1, i64 %56
  %gepload1107 = load i8, i8* %arrayIdx1106, align 1, !tbaa !21
  %57 = add i64 %3, 9
  %arrayIdx1108 = getelementptr inbounds i8, i8* %pix2, i64 %57
  %gepload1109 = load i8, i8* %arrayIdx1108, align 1, !tbaa !21
  %58 = add i64 %1, 13
  %arrayIdx1110 = getelementptr inbounds i8, i8* %pix1, i64 %58
  %gepload1111 = load i8, i8* %arrayIdx1110, align 1, !tbaa !21
  %59 = add i64 %3, 13
  %arrayIdx1112 = getelementptr inbounds i8, i8* %pix2, i64 %59
  %gepload1113 = load i8, i8* %arrayIdx1112, align 1, !tbaa !21
  %60 = add i64 %1, 10
  %arrayIdx1114 = getelementptr inbounds i8, i8* %pix1, i64 %60
  %gepload1115 = load i8, i8* %arrayIdx1114, align 1, !tbaa !21
  %61 = add i64 %3, 10
  %arrayIdx1116 = getelementptr inbounds i8, i8* %pix2, i64 %61
  %gepload1117 = load i8, i8* %arrayIdx1116, align 1, !tbaa !21
  %62 = add i64 %1, 14
  %arrayIdx1118 = getelementptr inbounds i8, i8* %pix1, i64 %62
  %gepload1119 = load i8, i8* %arrayIdx1118, align 1, !tbaa !21
  %63 = add i64 %3, 14
  %arrayIdx1120 = getelementptr inbounds i8, i8* %pix2, i64 %63
  %gepload1121 = load i8, i8* %arrayIdx1120, align 1, !tbaa !21
  %64 = add i64 %1, 11
  %arrayIdx1122 = getelementptr inbounds i8, i8* %pix1, i64 %64
  %gepload1123 = load i8, i8* %arrayIdx1122, align 1, !tbaa !21
  %65 = add i64 %3, 11
  %arrayIdx1124 = getelementptr inbounds i8, i8* %pix2, i64 %65
  %gepload1125 = load i8, i8* %arrayIdx1124, align 1, !tbaa !21
  %66 = add i64 %1, 15
  %arrayIdx1126 = getelementptr inbounds i8, i8* %pix1, i64 %66
  %gepload1127 = load i8, i8* %arrayIdx1126, align 1, !tbaa !21
  %67 = add i64 %3, 15
  %arrayIdx1128 = getelementptr inbounds i8, i8* %pix2, i64 %67
  %gepload1129 = load i8, i8* %arrayIdx1128, align 1, !tbaa !21
  %arrayIdx1131 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 4
  %68 = zext i8 %gepload1123 to i32
  %69 = zext i8 %gepload1115 to i32
  %70 = zext i8 %gepload1107 to i32
  %71 = zext i8 %gepload1099 to i32
  %72 = zext i8 %gepload1129 to i32
  %73 = zext i8 %gepload1121 to i32
  %74 = zext i8 %gepload1113 to i32
  %75 = zext i8 %gepload1105 to i32
  %76 = zext i8 %gepload1125 to i32
  %77 = zext i8 %gepload1117 to i32
  %78 = zext i8 %gepload1109 to i32
  %79 = zext i8 %gepload1101 to i32
  %80 = zext i8 %gepload1127 to i32
  %81 = zext i8 %gepload1119 to i32
  %82 = zext i8 %gepload1111 to i32
  %83 = zext i8 %gepload1103 to i32
  %reass.add1993 = sub nsw i32 %83, %75
  %reass.add1994 = add nsw i32 %reass.add1993, %82
  %reass.add1995 = sub nsw i32 %reass.add1994, %74
  %reass.add1996 = add nsw i32 %reass.add1995, %81
  %reass.add1997 = sub nsw i32 %reass.add1996, %73
  %reass.add1998 = add nsw i32 %reass.add1997, %80
  %reass.add1999 = sub i32 %reass.add1998, %72
  %reass.mul2000 = shl i32 %reass.add1999, 16
  %reass.add2001 = add nuw nsw i32 %78, %79
  %reass.add2002 = add nuw nsw i32 %reass.add2001, %77
  %reass.add2003 = add nuw nsw i32 %reass.add2002, %76
  %84 = add nuw nsw i32 %70, %71
  %85 = add nuw nsw i32 %84, %69
  %86 = add nuw nsw i32 %85, %68
  %87 = sub nsw i32 %86, %reass.add2003
  %88 = add i32 %87, %reass.mul2000
  store i32 %88, i32* %arrayIdx1131, align 4
  %arrayIdx1133 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 6
  %reass.add2012 = sub nsw i32 %reass.add1995, %81
  %reass.add2013 = add nsw i32 %reass.add2012, %73
  %reass.add2014 = sub nsw i32 %reass.add2013, %80
  %reass.add2015 = add i32 %reass.add2014, %72
  %reass.mul2016 = shl i32 %reass.add2015, 16
  %reass.add2018 = add nuw nsw i32 %reass.add2001, %69
  %reass.add2019 = add nuw nsw i32 %reass.add2018, %68
  %89 = add nuw nsw i32 %84, %77
  %90 = add nuw nsw i32 %89, %76
  %91 = sub nsw i32 %90, %reass.add2019
  %92 = add i32 %91, %reass.mul2016
  store i32 %92, i32* %arrayIdx1133, align 4
  %arrayIdx1151 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 5
  %reass.add2026 = sub nsw i32 %reass.add1993, %82
  %reass.add2027 = add nsw i32 %reass.add2026, %74
  %reass.add2028 = add nsw i32 %reass.add2027, %81
  %reass.add2029 = sub nsw i32 %reass.add2028, %73
  %reass.add2030 = sub nsw i32 %reass.add2029, %80
  %reass.add2031 = add i32 %reass.add2030, %72
  %reass.mul2032 = shl i32 %reass.add2031, 16
  %reass.add2033 = add nuw nsw i32 %70, %79
  %reass.add2034 = add nuw nsw i32 %reass.add2033, %77
  %reass.add2035 = add nuw nsw i32 %reass.add2034, %68
  %93 = add nuw nsw i32 %78, %71
  %94 = add nuw nsw i32 %93, %69
  %95 = add nuw nsw i32 %94, %76
  %96 = sub nsw i32 %95, %reass.add2035
  %97 = add i32 %96, %reass.mul2032
  store i32 %97, i32* %arrayIdx1151, align 4
  %arrayIdx1169 = getelementptr inbounds [16 x [8 x i32]], [16 x [8 x i32]]* %alloca, i64 0, i64 %i1.i64.0, i64 7
  %reass.add2044 = sub nsw i32 %reass.add2027, %81
  %reass.add2045 = add nsw i32 %reass.add2044, %73
  %reass.add2046 = add nsw i32 %reass.add2045, %80
  %reass.add2047 = sub i32 %reass.add2046, %72
  %reass.mul2048 = shl i32 %reass.add2047, 16
  %reass.add2050 = add nuw nsw i32 %reass.add2033, %69
  %reass.add2051 = add nuw nsw i32 %reass.add2050, %76
  %98 = add nuw nsw i32 %93, %77
 %99 = add nuw nsw i32 %98, %68
  %100 = sub nsw i32 %99, %reass.add2051
  %101 = add i32 %100, %reass.mul2048
  store i32 %101, i32* %arrayIdx1169, align 4
  %nextivloop.1247 = add nuw nsw i64 %i1.i64.0, 1
  %condloop.1247 = icmp ult i64 %nextivloop.1247, 16
  br i1 %condloop.1247, label %loop.1247, label %afterloop.1247
afterloop.1247:
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
