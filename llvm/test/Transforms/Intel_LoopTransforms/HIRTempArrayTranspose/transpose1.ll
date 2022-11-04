; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose,print<hir>" -disable-output 2>&1 | FileCheck %s

; Check that we successfully transpose the array for non-unit stride access
; (%4)[i2 + sext.i32.i64(%1) * i3]

;     BEGIN REGION { }
;           + DO i1 = 0, sext.i32.i64(%0) + -1, 1
;           |   %98 = (%54)[i1];
;           |
;           |   + DO i2 = 0, sext.i32.i64(%1) + -1, 1
;           |   |   %102 = (%6)[0].0[i2];
;           |   |   %105 = (%5)[sext.i32.i64(%1) * i1 + i2];
;           |   |   %110 = %105;
;           |   |
;           |   |      %123 = %105;
;           |   |   + DO i3 = 0, sext.i32.i64(%2) + -1, 1
;           |   |   |   %126 = (%3)[sext.i32.i64(%2) * i1 + i3];
;           |   |   |   %130 = (%4)[i2 + sext.i32.i64(%1) * i3];
;           |   |   |   %123 = (%126 * %130)  +  %123;
;           |   |   + END LOOP
;           |   |      %110 = %123;
;           |   |
;           |   |   %112 = (%18)[i2];
;           |   |   (%5)[sext.i32.i64(%1) * i1 + i2] = %105 + -1 * ((%98 * %102) + %112) + (%2 * %102) + %110;
;           |   + END LOOP
;           + END LOOP
;     END REGION

; CHECK:   BEGIN REGION { modified }
; CHECK:         %TranspTmpArr = alloca 4 * (sext.i32.i64(%1) * sext.i32.i64(%2));
; CHECK:         + DO i1 = 0, sext.i32.i64(%1) + -1, 1
; CHECK:         |   + DO i2 = 0, sext.i32.i64(%2) + -1, 1
; CHECK:         |   |   (%TranspTmpArr)[i1][i2] = (%4)[i2][i1];
; CHECK:         |   + END LOOP
; CHECK:         + END LOOP
; CHECK:         + DO i1 = 0, sext.i32.i64(%0) + -1, 1
; CHECK:         |   %98 = (%54)[i1];
; CHECK:         |
; CHECK:         |   + DO i2 = 0, sext.i32.i64(%1) + -1, 1
; CHECK:         |   |   %102 = (%6)[0].0[i2];
; CHECK:         |   |   %105 = (%5)[sext.i32.i64(%1) * i1 + i2];
; CHECK:         |   |   %110 = %105;
; CHECK:         |   |
; CHECK:         |   |      %123 = %105;
; CHECK:         |   |   + DO i3 = 0, sext.i32.i64(%2) + -1, 1
; CHECK:         |   |   |   %126 = (%3)[sext.i32.i64(%2) * i1 + i3];
; CHECK:         |   |   |   %130 = (%TranspTmpArr)[i2][i3];
; CHECK:         |   |   |   %123 = (%126 * %130)  +  %123;
; CHECK:         |   |   + END LOOP
; CHECK:         |   |      %110 = %123;
; CHECK:         |   |
; CHECK:         |   |   %112 = (%18)[i2];
; CHECK:         |   |   (%5)[sext.i32.i64(%1) * i1 + i2] = %105 + -1 * ((%98 * %102) + %112) + (%2 * %102) + %110;
; CHECK:         |   + END LOOP
; CHECK:         + END LOOP
; CHECK:   END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_istream" = type { i32 (...)**, i64, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", %"class.std::basic_ostream"*, i8, i8, %"class.std::basic_streambuf"*, %"class.std::ctype"*, %"class.std::num_put"*, %"class.std::num_put"* }
%"class.std::ios_base" = type { i32 (...)**, i64, i64, i32, i32, i32, %"struct.std::ios_base::_Callback_list"*, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, %"struct.std::ios_base::_Words"*, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { %"struct.std::ios_base::_Callback_list"*, void (i32, %"class.std::ios_base"*, i32)*, i32, i32 }
%"struct.std::ios_base::_Words" = type { i8*, i64 }
%"class.std::locale" = type { %"class.std::locale::_Impl"* }
%"class.std::locale::_Impl" = type { i32, %"class.std::locale::facet"**, i64, %"class.std::locale::facet"**, i8** }
%"class.std::locale::facet" = type <{ i32 (...)**, i32, [4 x i8] }>
%"class.std::basic_ostream" = type { i32 (...)**, %"class.std::basic_ios" }
%"class.std::basic_streambuf" = type { i32 (...)**, i8*, i8*, i8*, i8*, i8*, i8*, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], %struct.__locale_struct*, i8, [7 x i8], i32*, i32*, i16*, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ i32 (...)**, i32 }>
%struct.__locale_struct = type { [13 x %struct.__locale_data*], i16*, i32*, i32*, [13 x i8*] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, %struct._IO_codecvt*, %struct._IO_wide_data*, %struct._IO_FILE*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque
%"struct.std::array" = type { [1024 x i32] }

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_obj3.cpp, i8* null }]
@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZSt3cin = external dso_local global %"class.std::basic_istream", align 8
@stderr = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [11 x i8] c"\0A sum = %f\00", align 1
@.str.1 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1

; Function Attrs: nofree uwtable
define internal void @_GLOBAL__sub_I_obj3.cpp() #0 section ".text.startup" {
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* noundef nonnull align 1 dereferenceable(1) @_ZStL8__ioinit)
  %1 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZStL8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #11
  ret void
}

; Function Attrs: nofree
declare dso_local void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* noundef nonnull align 1 dereferenceable(1)) unnamed_addr #1

; Function Attrs: nofree nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"* noundef nonnull align 1 dereferenceable(1)) unnamed_addr #2

; Function Attrs: nofree nounwind
declare dso_local i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #3

; Function Attrs: norecurse uwtable
define internal fastcc void @_Z16gemm_lowp_matmuliiiPKiS0_PiiSt5arrayIiLm1024EE(i32 noundef %0, i32 noundef %1, i32 noundef %2, i32* noalias nocapture noundef readonly %3, i32* noalias nocapture noundef readonly %4, i32* noalias nocapture noundef %5, %"struct.std::array"* noalias nocapture noundef readonly byval(%"struct.std::array") align 8 %6) unnamed_addr #4 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
  %8 = sext i32 %1 to i64
  %9 = icmp slt i32 %1, 0
  br i1 %9, label %10, label %11

10:                                               ; preds = %7
  tail call void @_ZSt20__throw_length_errorPKc(i8* noundef getelementptr inbounds ([49 x i8], [49 x i8]* @.str.1, i64 0, i64 0)) #12
  unreachable

11:                                               ; preds = %7
  %12 = icmp eq i32 %1, 0
  br i1 %12, label %17, label %13

13:                                               ; preds = %11
  %14 = shl nuw nsw i64 %8, 2
  %15 = tail call noalias noundef nonnull i8* @_Znwm(i64 noundef %14) #13
  %16 = bitcast i8* %15 to i32*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull align 4 %15, i8 0, i64 %14, i1 false), !tbaa !6
  br label %17

17:                                               ; preds = %13, %11
  %18 = phi i32* [ %16, %13 ], [ null, %11 ]
  %19 = sext i32 %0 to i64
  %20 = icmp slt i32 %0, 0
  br i1 %20, label %21, label %23

21:                                               ; preds = %17
  invoke void @_ZSt20__throw_length_errorPKc(i8* noundef getelementptr inbounds ([49 x i8], [49 x i8]* @.str.1, i64 0, i64 0)) #12
          to label %22 unwind label %78

22:                                               ; preds = %21
  unreachable

23:                                               ; preds = %17
  %24 = icmp eq i32 %0, 0
  br i1 %24, label %53, label %25

25:                                               ; preds = %23
  %26 = shl nuw nsw i64 %19, 2
  %27 = invoke noalias noundef nonnull i8* @_Znwm(i64 noundef %26) #13
          to label %28 unwind label %78

28:                                               ; preds = %25
  %29 = bitcast i8* %27 to i32*
  tail call void @llvm.memset.p0i8.i64(i8* nonnull align 4 %27, i8 0, i64 %26, i1 false), !tbaa !6
  %30 = icmp sgt i32 %2, 0
  br i1 %30, label %32, label %31

31:                                               ; preds = %28
  br label %83

32:                                               ; preds = %28
  %33 = zext i32 %2 to i64
  br label %34

34:                                               ; preds = %47, %32
  %35 = phi i64 [ 0, %32 ], [ %49, %47 ]
  %36 = getelementptr inbounds i32, i32* %29, i64 %35, !intel-tbaa !6
  store i32 0, i32* %36, align 4, !tbaa !6
  %37 = mul nuw nsw i64 %35, %33
  br label %38

38:                                               ; preds = %38, %34
  %39 = phi i32 [ 0, %34 ], [ %44, %38 ]
  %40 = phi i64 [ 0, %34 ], [ %45, %38 ]
  %41 = add nuw nsw i64 %40, %37
  %42 = getelementptr inbounds i32, i32* %3, i64 %41
  %43 = load i32, i32* %42, align 4, !tbaa !6
  %44 = add nsw i32 %39, %43
  %45 = add nuw nsw i64 %40, 1
  %46 = icmp eq i64 %45, %33
  br i1 %46, label %47, label %38, !llvm.loop !10

47:                                               ; preds = %38
  %48 = phi i32 [ %44, %38 ]
  store i32 %48, i32* %36, align 4, !tbaa !6
  %49 = add nuw nsw i64 %35, 1
  %50 = icmp eq i64 %49, %19
  br i1 %50, label %51, label %34, !llvm.loop !12

51:                                               ; preds = %47
  br label %53

52:                                               ; preds = %83
  br label %53

53:                                               ; preds = %52, %51, %23
  %54 = phi i32* [ null, %23 ], [ %29, %51 ], [ %29, %52 ]
  %55 = icmp sgt i32 %1, 0
  br i1 %55, label %56, label %143

56:                                               ; preds = %53
  %57 = icmp sgt i32 %2, 0
  br i1 %57, label %59, label %58

58:                                               ; preds = %56
  br label %138

59:                                               ; preds = %56
  %60 = zext i32 %2 to i64
  br label %61

61:                                               ; preds = %74, %59
  %62 = phi i64 [ 0, %59 ], [ %76, %74 ]
  %63 = getelementptr inbounds i32, i32* %18, i64 %62, !intel-tbaa !6
  store i32 0, i32* %63, align 4, !tbaa !6
  br label %64

64:                                               ; preds = %64, %61
  %65 = phi i32 [ 0, %61 ], [ %71, %64 ]
  %66 = phi i64 [ 0, %61 ], [ %72, %64 ]
  %67 = mul nsw i64 %66, %8
  %68 = add nsw i64 %67, %62
  %69 = getelementptr inbounds i32, i32* %4, i64 %68
  %70 = load i32, i32* %69, align 4, !tbaa !6
  %71 = add nsw i32 %65, %70
  %72 = add nuw nsw i64 %66, 1
  %73 = icmp eq i64 %72, %60
  br i1 %73, label %74, label %64, !llvm.loop !13

74:                                               ; preds = %64
  %75 = phi i32 [ %71, %64 ]
  store i32 %75, i32* %63, align 4, !tbaa !6
  %76 = add nuw nsw i64 %62, 1
  %77 = icmp eq i64 %76, %8
  br i1 %77, label %88, label %61, !llvm.loop !14

78:                                               ; preds = %25, %21
  %79 = landingpad { i8*, i32 }
          cleanup
  %80 = icmp eq i32* %18, null
  br i1 %80, label %153, label %81

81:                                               ; preds = %78
  %82 = bitcast i32* %18 to i8*
  tail call void @_ZdlPv(i8* noundef nonnull %82) #11
  br label %153

83:                                               ; preds = %31, %83
  %84 = phi i64 [ %86, %83 ], [ 0, %31 ]
  %85 = getelementptr inbounds i32, i32* %29, i64 %84, !intel-tbaa !6
  store i32 0, i32* %85, align 4, !tbaa !6
  %86 = add nuw nsw i64 %84, 1
  %87 = icmp eq i64 %86, %19
  br i1 %87, label %52, label %83, !llvm.loop !12

88:                                               ; preds = %74
  br label %90

89:                                               ; preds = %138
  br label %90

90:                                               ; preds = %89, %88
  br i1 %24, label %143, label %91

91:                                               ; preds = %90
  %92 = sext i32 %2 to i64
  br label %93

93:                                               ; preds = %135, %91
  %94 = phi i64 [ 0, %91 ], [ %136, %135 ]
  %95 = mul nsw i64 %94, %8
  %96 = mul nsw i64 %94, %92
  %97 = getelementptr inbounds i32, i32* %54, i64 %94, !intel-tbaa !6
  %98 = load i32, i32* %97, align 4, !tbaa !6
  br label %99

99:                                               ; preds = %109, %93
  %100 = phi i64 [ 0, %93 ], [ %119, %109 ]
  %101 = getelementptr inbounds %"struct.std::array", %"struct.std::array"* %6, i64 0, i32 0, i64 %100, !intel-tbaa !15
  %102 = load i32, i32* %101, align 4, !tbaa !15
  %103 = add nsw i64 %100, %95
  %104 = getelementptr inbounds i32, i32* %5, i64 %103
  %105 = load i32, i32* %104, align 4, !tbaa !6
  br i1 %57, label %106, label %109

106:                                              ; preds = %99
  br label %121

107:                                              ; preds = %121
  %108 = phi i32 [ %132, %121 ]
  br label %109

109:                                              ; preds = %107, %99
  %110 = phi i32 [ %105, %99 ], [ %108, %107 ]
  %111 = getelementptr inbounds i32, i32* %18, i64 %100, !intel-tbaa !6
  %112 = load i32, i32* %111, align 4, !tbaa !6
  %113 = mul i32 %98, %102
  %114 = mul i32 %102, %2
  %115 = add i32 %113, %112
  %116 = add i32 %114, %105
  %117 = add i32 %116, %110
  %118 = sub i32 %117, %115
  store i32 %118, i32* %104, align 4, !tbaa !6
  %119 = add nuw nsw i64 %100, 1
  %120 = icmp eq i64 %119, %8
  br i1 %120, label %135, label %99, !llvm.loop !18

121:                                              ; preds = %106, %121
  %122 = phi i64 [ %133, %121 ], [ 0, %106 ]
  %123 = phi i32 [ %132, %121 ], [ %105, %106 ]
  %124 = add nsw i64 %122, %96
  %125 = getelementptr inbounds i32, i32* %3, i64 %124
  %126 = load i32, i32* %125, align 4, !tbaa !6
  %127 = mul nsw i64 %122, %8
  %128 = add nsw i64 %127, %100
  %129 = getelementptr inbounds i32, i32* %4, i64 %128
  %130 = load i32, i32* %129, align 4, !tbaa !6
  %131 = mul nsw i32 %130, %126
  %132 = add nsw i32 %131, %123
  %133 = add nuw nsw i64 %122, 1
  %134 = icmp eq i64 %133, %92
  br i1 %134, label %107, label %121, !llvm.loop !19

135:                                              ; preds = %109
  %136 = add nuw nsw i64 %94, 1
  %137 = icmp eq i64 %136, %19
  br i1 %137, label %145, label %93, !llvm.loop !20

138:                                              ; preds = %58, %138
  %139 = phi i64 [ %141, %138 ], [ 0, %58 ]
  %140 = getelementptr inbounds i32, i32* %18, i64 %139, !intel-tbaa !6
  store i32 0, i32* %140, align 4, !tbaa !6
  %141 = add nuw nsw i64 %139, 1
  %142 = icmp eq i64 %141, %8
  br i1 %142, label %89, label %138, !llvm.loop !14

143:                                              ; preds = %90, %53
  %144 = icmp eq i32* %54, null
  br i1 %144, label %148, label %146

145:                                              ; preds = %135
  br label %146

146:                                              ; preds = %145, %143
  %147 = bitcast i32* %54 to i8*
  tail call void @_ZdlPv(i8* noundef nonnull %147) #11
  br label %148

148:                                              ; preds = %146, %143
  %149 = icmp eq i32* %18, null
  br i1 %149, label %152, label %150

150:                                              ; preds = %148
  %151 = bitcast i32* %18 to i8*
  tail call void @_ZdlPv(i8* noundef nonnull %151) #11
  br label %152

152:                                              ; preds = %150, %148
  ret void

153:                                              ; preds = %81, %78
  resume { i8*, i32 } %79
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nofree noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(i8* noundef) local_unnamed_addr #5

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noundef nonnull i8* @_Znwm(i64 noundef) local_unnamed_addr #6

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #7

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(i8* noundef) local_unnamed_addr #8

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() local_unnamed_addr #9 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  %4 = alloca [1048576 x i32], align 16
  %5 = alloca [1048576 x i32], align 16
  %6 = alloca [1048576 x i32], align 16
  %7 = bitcast [1048576 x i32]* %6 to i8*
  %8 = alloca %"struct.std::array", align 8
  %9 = bitcast i32* %1 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %9) #11
  %10 = bitcast i32* %2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %10) #11
  %11 = bitcast i32* %3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %11) #11
  %12 = bitcast [1048576 x i32]* %4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4194304, i8* nonnull %12) #11
  %13 = bitcast [1048576 x i32]* %5 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4194304, i8* nonnull %13) #11
  call void @llvm.lifetime.start.p0i8(i64 4194304, i8* nonnull %7) #11
  %14 = call noundef nonnull align 8 dereferenceable(16) %"class.std::basic_istream"* @_ZNSirsERi(%"class.std::basic_istream"* noundef nonnull align 8 dereferenceable(16) @_ZSt3cin, i32* noundef nonnull align 4 dereferenceable(4) %1)
  %15 = call noundef nonnull align 8 dereferenceable(16) %"class.std::basic_istream"* @_ZNSirsERi(%"class.std::basic_istream"* noundef nonnull align 8 dereferenceable(16) @_ZSt3cin, i32* noundef nonnull align 4 dereferenceable(4) %2)
  %16 = call noundef nonnull align 8 dereferenceable(16) %"class.std::basic_istream"* @_ZNSirsERi(%"class.std::basic_istream"* noundef nonnull align 8 dereferenceable(16) @_ZSt3cin, i32* noundef nonnull align 4 dereferenceable(4) %3)
  %17 = bitcast %"struct.std::array"* %8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4096, i8* nonnull %17) #11
  %18 = load i32, i32* %1, align 4, !tbaa !6
  %19 = icmp sgt i32 %18, 0
  br i1 %19, label %20, label %46

20:                                               ; preds = %0
  %21 = load i32, i32* %3, align 4, !tbaa !6
  %22 = icmp sgt i32 %21, 0
  br i1 %22, label %23, label %41

23:                                               ; preds = %20
  %24 = zext i32 %21 to i64
  %25 = zext i32 %18 to i64
  br label %26

26:                                               ; preds = %35, %23
  %27 = phi i64 [ 0, %23 ], [ %36, %35 ]
  %28 = mul nuw nsw i64 %27, %24
  br label %29

29:                                               ; preds = %29, %26
  %30 = phi i64 [ 0, %26 ], [ %33, %29 ]
  %31 = add nuw nsw i64 %28, %30
  %32 = getelementptr inbounds [1048576 x i32], [1048576 x i32]* %4, i64 0, i64 %31, !intel-tbaa !21
  store i32 1, i32* %32, align 4, !tbaa !21
  %33 = add nuw nsw i64 %30, 1
  %34 = icmp eq i64 %33, %24
  br i1 %34, label %35, label %29, !llvm.loop !23

35:                                               ; preds = %29
  %36 = add nuw nsw i64 %27, 1
  %37 = icmp eq i64 %36, %25
  br i1 %37, label %38, label %26, !llvm.loop !24

38:                                               ; preds = %35
  %39 = load i32, i32* %2, align 4, !tbaa !6
  %40 = icmp sgt i32 %39, 0
  br i1 %40, label %54, label %77

41:                                               ; preds = %20
  %42 = load i32, i32* %2, align 4, !tbaa !6
  %43 = icmp sgt i32 %42, 0
  br i1 %43, label %71, label %44

44:                                               ; preds = %41
  %45 = zext i32 %18 to i64
  br label %77

46:                                               ; preds = %0
  %47 = load i32, i32* %2, align 4, !tbaa !6
  %48 = icmp sgt i32 %47, 0
  %49 = load i32, i32* %3, align 4
  %50 = icmp sgt i32 %49, 0
  %51 = select i1 %48, i1 %50, i1 false
  br i1 %51, label %52, label %80

52:                                               ; preds = %46
  %53 = zext i32 %49 to i64
  br label %54

54:                                               ; preds = %38, %52
  %55 = phi i64 [ %53, %52 ], [ %24, %38 ]
  %56 = phi i32 [ %47, %52 ], [ %39, %38 ]
  %57 = zext i32 %56 to i64
  br label %58

58:                                               ; preds = %67, %54
  %59 = phi i64 [ 0, %54 ], [ %68, %67 ]
  br label %60

60:                                               ; preds = %60, %58
  %61 = phi i64 [ 0, %58 ], [ %65, %60 ]
  %62 = mul nuw nsw i64 %61, %57
  %63 = add nuw nsw i64 %62, %59
  %64 = getelementptr inbounds [1048576 x i32], [1048576 x i32]* %5, i64 0, i64 %63, !intel-tbaa !21
  store i32 2, i32* %64, align 4, !tbaa !21
  %65 = add nuw nsw i64 %61, 1
  %66 = icmp eq i64 %65, %55
  br i1 %66, label %67, label %60, !llvm.loop !25

67:                                               ; preds = %60
  %68 = add nuw nsw i64 %59, 1
  %69 = icmp eq i64 %68, %57
  br i1 %69, label %70, label %58, !llvm.loop !26

70:                                               ; preds = %67
  br i1 %19, label %71, label %80

71:                                               ; preds = %70, %41
  %72 = phi i32 [ %42, %41 ], [ %56, %70 ]
  %73 = zext i32 %72 to i64
  %74 = zext i32 %18 to i64
  %75 = mul nuw nsw i64 %73, %74
  %76 = shl i64 %75, 2
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 %7, i8 0, i64 %76, i1 false), !tbaa !21
  br label %77

77:                                               ; preds = %44, %71, %38
  %78 = phi i64 [ %45, %44 ], [ %74, %71 ], [ %25, %38 ]
  br label %84

79:                                               ; preds = %84
  br label %80

80:                                               ; preds = %79, %70, %46
  %81 = getelementptr inbounds [1048576 x i32], [1048576 x i32]* %4, i64 0, i64 0, !intel-tbaa !21
  %82 = getelementptr inbounds [1048576 x i32], [1048576 x i32]* %5, i64 0, i64 0, !intel-tbaa !21
  %83 = getelementptr inbounds [1048576 x i32], [1048576 x i32]* %6, i64 0, i64 0, !intel-tbaa !21
  br label %117

84:                                               ; preds = %84, %77
  %85 = phi i64 [ 0, %77 ], [ %88, %84 ]
  %86 = getelementptr inbounds %"struct.std::array", %"struct.std::array"* %8, i64 0, i32 0, i64 %85, !intel-tbaa !15
  %87 = trunc i64 %85 to i32
  store i32 %87, i32* %86, align 4, !tbaa !15
  %88 = add nuw nsw i64 %85, 1
  %89 = icmp eq i64 %88, %78
  br i1 %89, label %79, label %84, !llvm.loop !27

90:                                               ; preds = %117
  %91 = phi i32 [ %122, %117 ]
  %92 = icmp sgt i32 %91, 0
  br i1 %92, label %93, label %130

93:                                               ; preds = %90
  %94 = load i32, i32* %2, align 4, !tbaa !6
  %95 = icmp sgt i32 %94, 0
  br i1 %95, label %96, label %130

96:                                               ; preds = %93
  %97 = zext i32 %94 to i64
  %98 = zext i32 %91 to i64
  br label %99

99:                                               ; preds = %113, %96
  %100 = phi i64 [ 0, %96 ], [ %115, %113 ]
  %101 = phi double [ 0.000000e+00, %96 ], [ %114, %113 ]
  %102 = mul nuw nsw i64 %100, %97
  br label %103

103:                                              ; preds = %103, %99
  %104 = phi i64 [ 0, %99 ], [ %111, %103 ]
  %105 = phi double [ %101, %99 ], [ %110, %103 ]
  %106 = add nuw nsw i64 %102, %104
  %107 = getelementptr inbounds [1048576 x i32], [1048576 x i32]* %6, i64 0, i64 %106, !intel-tbaa !21
  %108 = load i32, i32* %107, align 4, !tbaa !21
  %109 = sitofp i32 %108 to double
  %110 = fadd fast double %105, %109
  %111 = add nuw nsw i64 %104, 1
  %112 = icmp eq i64 %111, %97
  br i1 %112, label %113, label %103, !llvm.loop !28

113:                                              ; preds = %103
  %114 = phi double [ %110, %103 ]
  %115 = add nuw nsw i64 %100, 1
  %116 = icmp eq i64 %115, %98
  br i1 %116, label %128, label %99, !llvm.loop !29

117:                                              ; preds = %117, %80
  %118 = phi i32 [ %18, %80 ], [ %122, %117 ]
  %119 = phi i32 [ 0, %80 ], [ %126, %117 ]
  %120 = load i32, i32* %2, align 4, !tbaa !6
  %121 = load i32, i32* %3, align 4, !tbaa !6
  call fastcc void @_Z16gemm_lowp_matmuliiiPKiS0_PiiSt5arrayIiLm1024EE(i32 noundef %118, i32 noundef %120, i32 noundef %121, i32* noundef nonnull %81, i32* noundef nonnull %82, i32* noundef nonnull %83, %"struct.std::array"* noundef nonnull byval(%"struct.std::array") align 8 %8) #14
  %122 = load i32, i32* %1, align 4, !tbaa !6
  %123 = srem i32 %119, %122
  %124 = zext i32 %123 to i64
  %125 = getelementptr inbounds [1048576 x i32], [1048576 x i32]* %6, i64 0, i64 %124, !intel-tbaa !21
  store i32 %123, i32* %125, align 4, !tbaa !21
  %126 = add nuw nsw i32 %119, 1
  %127 = icmp eq i32 %126, 100
  br i1 %127, label %90, label %117, !llvm.loop !30

128:                                              ; preds = %113
  %129 = phi double [ %114, %113 ]
  br label %130

130:                                              ; preds = %128, %93, %90
  %131 = phi double [ 0.000000e+00, %90 ], [ 0.000000e+00, %93 ], [ %129, %128 ]
  %132 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !31
  %133 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* noundef %132, i8* noundef getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0), double noundef %131) #15
  call void @llvm.lifetime.end.p0i8(i64 4096, i8* nonnull %17) #11
  call void @llvm.lifetime.end.p0i8(i64 4194304, i8* nonnull %7) #11
  call void @llvm.lifetime.end.p0i8(i64 4194304, i8* nonnull %13) #11
  call void @llvm.lifetime.end.p0i8(i64 4194304, i8* nonnull %12) #11
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %11) #11
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %10) #11
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %9) #11
  ret i32 0
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #10

; Function Attrs: nofree
declare dso_local noundef nonnull align 8 dereferenceable(16) %"class.std::basic_istream"* @_ZNSirsERi(%"class.std::basic_istream"* noundef nonnull align 8 dereferenceable(16), i32* noundef nonnull align 4 dereferenceable(4)) local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(%struct._IO_FILE* nocapture noundef, i8* nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #10

attributes #0 = { nofree uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind }
attributes #4 = { norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nofree noreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { argmemonly mustprogress nofree nounwind willreturn writeonly }
attributes #8 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #9 = { mustprogress norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #10 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #11 = { nounwind }
attributes #12 = { noreturn }
attributes #13 = { allocsize(0) }
attributes #14 = { noinline }
attributes #15 = { cold }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1, !2, !3, !4, !5}

!0 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{i32 1, !"LTOPostLink", i32 1}
!6 = !{!7, !7, i64 0}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
!10 = distinct !{!10, !11}
!11 = !{!"llvm.loop.mustprogress"}
!12 = distinct !{!12, !11}
!13 = distinct !{!13, !11}
!14 = distinct !{!14, !11}
!15 = !{!16, !7, i64 0}
!16 = !{!"struct@_ZTSSt5arrayIiLm1024EE", !17, i64 0}
!17 = !{!"array@_ZTSA1024_i", !7, i64 0}
!18 = distinct !{!18, !11}
!19 = distinct !{!19, !11}
!20 = distinct !{!20, !11}
!21 = !{!22, !7, i64 0}
!22 = !{!"array@_ZTSA1048576_i", !7, i64 0}
!23 = distinct !{!23, !11}
!24 = distinct !{!24, !11}
!25 = distinct !{!25, !11}
!26 = distinct !{!26, !11}
!27 = distinct !{!27, !11}
!28 = distinct !{!28, !11}
!29 = distinct !{!29, !11}
!30 = distinct !{!30, !11}
!31 = !{!32, !32, i64 0}
!32 = !{!"pointer@_ZTSP8_IO_FILE", !8, i64 0}
