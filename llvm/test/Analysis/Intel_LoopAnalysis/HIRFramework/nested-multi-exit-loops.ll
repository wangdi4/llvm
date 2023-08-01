; RUN: opt < %s -hir-cost-model-throttling=0 -passes="hir-ssa-deconstruction,print<hir>" 2>&1 | FileCheck %s


; This test was compfailing because of incorrect lexical link formation for
; nested multi-exit i2-i3 loops and a def-level bug in parser.
; I did not try to reduce the test case much because of complicated CFG
; required to trigger the bugs.

; Parser fix is checked using verification.

; CHECK: + DO i1
; CHECK: |
; CHECK: | + UNKNOWN LOOP i2
; CHECK: | |
; CHECK: | |  + UNKNOWN LOOP i3
; CHECK: | |  |    goto %68;
; CHECK: | |  + END LOOP
; CHECK: | |
; CHECK: | |   goto %85;
; CHECK: | |   %68:
; CHECK: | |
; CHECK: | |   goto %83;
; CHECK: | + END LOOP
; CHECK: |
; CHECK: | %83:
; CHECK: |
; CHECK: | %85:
; CHECK: |
; CHECK: | + UNKNOWN LOOP i2
; CHECK: | |
; CHECK: | |  + UNKNOWN LOOP i3
; CHECK: | |  + END LOOP
; CHECK: | |
; CHECK: | + END LOOP
; CHECK: + END LOOP


target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.T_SKTREE = type { ptr, ptr, ptr, ptr, ptr, ptr }
%struct.uExpressionValue = type { i32, %union.anon }
%union.anon = type { double }
%struct.IMPLEMENTATION = type { i32, ptr, ptr, i32, ptr, i32, ptr }
%struct.PARAM = type { ptr, ptr, i32, ptr, i32, ptr, ptr }
%struct.PARAM_PROPS = type { ptr, ptr, i32, ptr, ptr, i32, ptr, i32, i32, i32, i32, ptr }
%struct.RANGE = type { ptr, ptr, ptr, ptr, i32, ptr }

@implist = external hidden unnamed_addr global ptr, align 8
@.str.606 = external hidden unnamed_addr constant [21 x i8], align 1
@.str.47.608 = external hidden unnamed_addr constant [51 x i8], align 1
@.str.48.609 = external hidden unnamed_addr constant [45 x i8], align 1
@.str.49.610 = external hidden unnamed_addr constant [46 x i8], align 1
@.str.72 = external hidden unnamed_addr constant [51 x i8], align 1
@.str.73 = external hidden unnamed_addr constant [60 x i8], align 1
@.str.977 = external hidden unnamed_addr constant [3 x i8], align 1
@.str.2.1260 = external hidden unnamed_addr constant [3 x i8], align 1
@.str.1.10510 = external hidden unnamed_addr constant [7 x i8], align 1

; Function Attrs: nofree nounwind
declare dso_local noalias ptr @malloc(i64) local_unnamed_addr #1

; Function Attrs: nounwind
declare dso_local void @free(ptr nocapture) local_unnamed_addr #2

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #3

; Function Attrs: nofree nounwind readnone
declare dso_local nonnull ptr @__ctype_tolower_loc() local_unnamed_addr #4

; Function Attrs: argmemonly nofree nounwind readonly
declare dso_local i64 @strlen(ptr nocapture) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare dso_local i64 @strtol(ptr readonly, ptr nocapture, i32) local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare dso_local ptr @strcpy(ptr noalias returned, ptr noalias nocapture readonly) local_unnamed_addr #1

; Function Attrs: norecurse nounwind uwtable
define hidden i32 @IntParameterEvaluator(i32 %0, ptr nocapture readonly %1, ptr nocapture %2, ptr %3) {
  %5 = alloca ptr, align 8
  %6 = alloca ptr, align 8
  %7 = alloca ptr, align 8
  %8 = bitcast ptr %5 to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %8) #8
  %9 = bitcast ptr %6 to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %9) #8
  %10 = bitcast ptr %7 to ptr
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %10) #8
  %11 = icmp sgt i32 %0, 0
  br i1 %11, label %12, label %182

12:                                               ; preds = %4
  %13 = sext i32 %0 to i64
  br label %14

14:                                               ; preds = %178, %12
  %15 = phi i64 [ 0, %12 ], [ %179, %178 ]
  %16 = getelementptr inbounds %struct.uExpressionValue, ptr %2, i64 %15, i32 0
  store i32 1, ptr %16, align 8
  %17 = getelementptr inbounds ptr, ptr %1, i64 %15
  %18 = load ptr, ptr %17, align 8
  %19 = tail call i64 @strlen(ptr %18) #9
  %20 = add i64 %19, 1
  %21 = tail call noalias ptr @malloc(i64 %20) #8
  %22 = icmp eq ptr %21, null
  br i1 %22, label %25, label %23

23:                                               ; preds = %14
  %24 = tail call ptr @strcpy(ptr nonnull %21, ptr %18) #8
  br label %25

25:                                               ; preds = %23, %14
  %26 = call i64 @strtol(ptr %21, ptr nonnull %5, i32 0) #8
  %27 = trunc i64 %26 to i32
  %28 = getelementptr inbounds %struct.uExpressionValue, ptr %2, i64 %15, i32 1
  %29 = bitcast ptr %28 to ptr
  store i32 %27, ptr %29, align 8
  %30 = load ptr, ptr %5, align 8
  %31 = icmp eq ptr %30, %21
  br i1 %31, label %32, label %178

32:                                               ; preds = %25
  %33 = call fastcc i32 @Util_SplitString(ptr nonnull %6, ptr nonnull %7, ptr %21, ptr @.str.2.1260) #8
  switch i32 %33, label %36 [
    i32 0, label %38
    i32 1, label %34
    i32 2, label %35
  ]

34:                                               ; preds = %32
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 8, i32 1198, ptr @.str.606, ptr @.str.1.10510, ptr @.str.47.608, ptr %21) #8
  br label %37

35:                                               ; preds = %32
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 2, i32 1205, ptr @.str.606, ptr @.str.1.10510, ptr @.str.977, ptr @.str.48.609) #8
  br label %37

36:                                               ; preds = %32
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 1, i32 1211, ptr @.str.606, ptr @.str.1.10510, ptr @.str.977, ptr @.str.49.610) #8
  br label %37

37:                                               ; preds = %36, %35, %34
  store ptr null, ptr %6, align 8
  store ptr null, ptr %7, align 8
  br label %154

38:                                               ; preds = %32
  %39 = load ptr, ptr %6, align 8
  %40 = load ptr, ptr @implist, align 8
  %41 = icmp eq ptr %40, null
  br i1 %41, label %151, label %42

42:                                               ; preds = %38
  %43 = tail call ptr @__ctype_tolower_loc() #10
  %44 = load ptr, ptr %43, align 8
  br label %45

45:                                               ; preds = %77, %42
  %46 = phi ptr [ %79, %77 ], [ %40, %42 ]
  %47 = getelementptr inbounds %struct.T_SKTREE, ptr %46, i64 0, i32 4
  %48 = load ptr, ptr %47, align 8
  br label %49

49:                                               ; preds = %64, %45
  %50 = phi ptr [ %48, %45 ], [ %66, %64 ]
  %51 = phi ptr [ %39, %45 ], [ %65, %64 ]
  %52 = load i8, ptr %51, align 1
  %53 = sext i8 %52 to i64
  %54 = getelementptr inbounds i32, ptr %44, i64 %53
  %55 = load i32, ptr %54, align 4
  %56 = load i8, ptr %50, align 1
  %57 = sext i8 %56 to i64
  %58 = getelementptr inbounds i32, ptr %44, i64 %57
  %59 = load i32, ptr %58, align 4
  %60 = sub nsw i32 %55, %59
  %61 = icmp ne i32 %60, 0
  %62 = icmp eq i8 %52, 0
  %63 = or i1 %62, %61
  br i1 %63, label %68, label %64

64:                                               ; preds = %49
  %65 = getelementptr inbounds i8, ptr %51, i64 1
  %66 = getelementptr inbounds i8, ptr %50, i64 1
  %67 = icmp eq i8 %56, 0
  br i1 %67, label %81, label %49

68:                                               ; preds = %49
  %69 = phi i32 [ %60, %49 ]
  %70 = icmp slt i32 %69, 0
  br i1 %70, label %71, label %73

71:                                               ; preds = %68
  %72 = getelementptr inbounds %struct.T_SKTREE, ptr %46, i64 0, i32 0
  br label %77

73:                                               ; preds = %68
  %74 = icmp eq i32 %69, 0
  br i1 %74, label %83, label %75

75:                                               ; preds = %73
  %76 = getelementptr inbounds %struct.T_SKTREE, ptr %46, i64 0, i32 1
  br label %77

77:                                               ; preds = %75, %71
  %78 = phi ptr [ %76, %75 ], [ %72, %71 ]
  %79 = load ptr, ptr %78, align 8
  %80 = icmp eq ptr %79, null
  br i1 %80, label %150, label %45

81:                                               ; preds = %64
  %82 = phi ptr [ %46, %64 ]
  br label %85

83:                                               ; preds = %73
  %84 = phi ptr [ %46, %73 ]
  br label %85

85:                                               ; preds = %83, %81
  %86 = phi ptr [ %82, %81 ], [ %84, %83 ]
  %87 = icmp eq ptr %86, null
  br i1 %87, label %151, label %88

88:                                               ; preds = %85
  %89 = getelementptr inbounds %struct.T_SKTREE, ptr %86, i64 0, i32 5
  %90 = bitcast ptr %89 to ptr
  %91 = load ptr, ptr %90, align 8
  %92 = getelementptr inbounds %struct.IMPLEMENTATION, ptr %91, i64 0, i32 0
  %93 = load i32, ptr %92, align 8
  %94 = icmp eq i32 %93, 0
  br i1 %94, label %151, label %95

95:                                               ; preds = %88
  br label %96

96:                                               ; preds = %128, %95
  %97 = phi ptr [ %130, %128 ], [ %40, %95 ]
  %98 = getelementptr inbounds %struct.T_SKTREE, ptr %97, i64 0, i32 4
  %99 = load ptr, ptr %98, align 8
  br label %100

100:                                              ; preds = %115, %96
  %101 = phi ptr [ %99, %96 ], [ %117, %115 ]
  %102 = phi ptr [ %39, %96 ], [ %116, %115 ]
  %103 = load i8, ptr %102, align 1
  %104 = sext i8 %103 to i64
  %105 = getelementptr inbounds i32, ptr %44, i64 %104
  %106 = load i32, ptr %105, align 4
  %107 = load i8, ptr %101, align 1
  %108 = sext i8 %107 to i64
  %109 = getelementptr inbounds i32, ptr %44, i64 %108
  %110 = load i32, ptr %109, align 4
  %111 = sub nsw i32 %106, %110
  %112 = icmp ne i32 %111, 0
  %113 = icmp eq i8 %103, 0
  %114 = or i1 %113, %112
  br i1 %114, label %119, label %115

115:                                              ; preds = %100
  %116 = getelementptr inbounds i8, ptr %102, i64 1
  %117 = getelementptr inbounds i8, ptr %101, i64 1
  %118 = icmp eq i8 %107, 0
  br i1 %118, label %132, label %100

119:                                              ; preds = %100
  %120 = phi i32 [ %111, %100 ]
  %121 = icmp slt i32 %120, 0
  br i1 %121, label %122, label %124

122:                                              ; preds = %119
  %123 = getelementptr inbounds %struct.T_SKTREE, ptr %97, i64 0, i32 0
  br label %128

124:                                              ; preds = %119
  %125 = icmp eq i32 %120, 0
  br i1 %125, label %134, label %126

126:                                              ; preds = %124
  %127 = getelementptr inbounds %struct.T_SKTREE, ptr %97, i64 0, i32 1
  br label %128

128:                                              ; preds = %126, %122
  %129 = phi ptr [ %127, %126 ], [ %123, %122 ]
  %130 = load ptr, ptr %129, align 8
  %131 = icmp eq ptr %130, null
  br i1 %131, label %149, label %96

132:                                              ; preds = %115
  %133 = phi ptr [ %97, %115 ]
  br label %136

134:                                              ; preds = %124
  %135 = phi ptr [ %97, %124 ]
  br label %136

136:                                              ; preds = %134, %132
  %137 = phi ptr [ %133, %132 ], [ %135, %134 ]
  %138 = icmp eq ptr %137, null
  br i1 %138, label %151, label %139

139:                                              ; preds = %136
  %140 = getelementptr inbounds %struct.T_SKTREE, ptr %137, i64 0, i32 5
  %141 = bitcast ptr %140 to ptr
  %142 = load ptr, ptr %141, align 8
  %143 = getelementptr inbounds %struct.IMPLEMENTATION, ptr %142, i64 0, i32 0
  %144 = load i32, ptr %143, align 8
  %145 = icmp eq i32 %144, 0
  br i1 %145, label %151, label %146

146:                                              ; preds = %139
  %147 = getelementptr inbounds %struct.IMPLEMENTATION, ptr %142, i64 0, i32 2
  %148 = load ptr, ptr %147, align 8
  br label %151

149:                                              ; preds = %128
  br label %151

150:                                              ; preds = %77
  br label %151

151:                                              ; preds = %150, %149, %146, %139, %136, %88, %85, %38
  %152 = phi ptr [ %39, %88 ], [ %148, %146 ], [ null, %139 ], [ null, %136 ], [ %39, %85 ], [ %39, %38 ], [ null, %149 ], [ %39, %150 ]
  %153 = load ptr, ptr %7, align 8
  br label %154

154:                                              ; preds = %151, %37
  %155 = phi ptr [ null, %37 ], [ %153, %151 ]
  %156 = phi ptr [ null, %37 ], [ %39, %151 ]
  %157 = phi ptr [ %3, %37 ], [ %152, %151 ]
  %158 = phi ptr [ %21, %37 ], [ %153, %151 ]
  %159 = tail call fastcc ptr @ParameterFind(ptr %158, ptr %157, i32 905) #8
  %160 = icmp eq ptr %159, null
  br i1 %160, label %175, label %161

161:                                              ; preds = %154
  %162 = getelementptr inbounds %struct.PARAM, ptr %159, i64 0, i32 0
  %163 = load ptr, ptr %162, align 8
  %164 = getelementptr inbounds %struct.PARAM_PROPS, ptr %163, i64 0, i32 5
  %165 = load i32, ptr %164, align 8
  %166 = getelementptr inbounds %struct.PARAM, ptr %159, i64 0, i32 1
  %167 = load ptr, ptr %166, align 8
  %168 = bitcast ptr %167 to ptr
  %169 = icmp ne ptr %167, null
  %170 = icmp eq i32 %165, 704
  %171 = and i1 %169, %170
  br i1 %171, label %172, label %174

172:                                              ; preds = %161
  %173 = load i32, ptr %168, align 4
  store i32 %173, ptr %29, align 8
  br label %177

174:                                              ; preds = %161
  br i1 %169, label %176, label %175

175:                                              ; preds = %174, %154
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 0, i32 2882, ptr @.str.606, ptr @.str.1.10510, ptr @.str.72, ptr %157, ptr %158) #8
  br label %177

176:                                              ; preds = %174
  tail call void (i32, i32, ptr, ptr, ptr, ...) @CCTK_VWarn(i32 0, i32 2888, ptr @.str.606, ptr @.str.1.10510, ptr @.str.73, ptr %157, ptr %158) #8
  br label %177

177:                                              ; preds = %176, %175, %172
  tail call void @free(ptr %156) #8
  tail call void @free(ptr %155) #8
  br label %178

178:                                              ; preds = %177, %25
  tail call void @free(ptr %21) #8
  %179 = add nuw nsw i64 %15, 1
  %180 = icmp eq i64 %179, %13
  br i1 %180, label %181, label %14

181:                                              ; preds = %178
  br label %182

182:                                              ; preds = %181, %4
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %10) #8
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %9) #8
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %8) #8
  ret i32 0
}

; Function Attrs: nounwind uwtable
declare hidden fastcc ptr @ParameterFind(ptr, ptr readonly, i32) unnamed_addr #7

; Function Attrs: nounwind uwtable
declare hidden void @CCTK_VWarn(i32, i32, ptr, ptr, ptr nocapture readonly, ...) unnamed_addr #7

; Function Attrs: nounwind uwtable
declare hidden fastcc i32 @Util_SplitString(ptr nocapture, ptr nocapture, ptr, ptr nocapture readonly) unnamed_addr #7

