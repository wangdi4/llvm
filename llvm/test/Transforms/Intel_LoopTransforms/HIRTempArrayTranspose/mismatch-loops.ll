; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose,print<hir>" -hir-details -disable-output 2>&1 | FileCheck %s

; Check that transpose does not occur for mismatched loop refs which have different types.
; i2 and i3 loops have different types which are the the candidates for transpose.

;      BEGIN REGION { }
;            + DO i1 = 0, sext.i32.i64(%arg) + -1, 1   <DO_LOOP>
;            |   %i96 = (%i52)[i1];
;            |
;            |   + DO i2 = 0, sext.i32.i64(%arg1) + -1, 1   <DO_LOOP>
;            |   |   %i100 = (%arg6)[0].0[i2];
;            |   |   %i103 = (%arg5)[sext.i32.i64(%arg1) * i1 + i2];
;            |   |   %i108 = %i103;
;            |   |
;            |   |      %i121 = %i103;
;            |   |   + DO i3 = 0, %arg2 + -1, 1   <DO_LOOP>
;            |   |   |   %i124 = (%arg3)[sext.i32.i64(%arg2) * i1 + i3];
;            |   |   |   %i128 = (%arg4)[i2 + sext.i32.i64(%arg1) * i3];
;            |   |   |   %i121 = (%i124 * %i128)  +  %i121;
;            |   |   + END LOOP
;            |   |      %i108 = %i121;
;            |   |
;            |   |   %i110 = (%i16)[i2];
;            |   |   (%arg5)[sext.i32.i64(%arg1) * i1 + i2] = %i103 + -1 * ((%i96 * %i100) + %i110) + (%arg2 * %i100) + %i108;
;            |   + END LOOP
;            + END LOOP
;      END REGION

; CHECK: BEGIN REGION
; CHECK-NOT: modified


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%"class.std::basic_istream" = type { ptr, i64, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, ptr, %"class.std::locale" }
%"struct.std::ios_base::_Callback_list" = type { ptr, ptr, i32, i32 }
%"struct.std::ios_base::_Words" = type { ptr, i64 }
%"class.std::locale" = type { ptr }
%"class.std::locale::_Impl" = type { i32, ptr, i64, ptr, ptr }
%"class.std::locale::facet" = type <{ ptr, i32, [4 x i8] }>
%"class.std::basic_ostream" = type { ptr, %"class.std::basic_ios" }
%"class.std::basic_streambuf" = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, %"class.std::locale" }
%"class.std::ctype" = type <{ %"class.std::locale::facet.base", [4 x i8], ptr, i8, [7 x i8], ptr, ptr, ptr, i8, [256 x i8], [256 x i8], i8, [6 x i8] }>
%"class.std::locale::facet.base" = type <{ ptr, i32 }>
%struct.__locale_struct = type { [13 x ptr], ptr, ptr, ptr, [13 x ptr] }
%struct.__locale_data = type opaque
%"class.std::num_put" = type { %"class.std::locale::facet.base", [4 x i8] }
%struct._IO_FILE = type { i32, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, i64, i16, i8, [1 x i8], ptr, i64, ptr, ptr, ptr, ptr, i64, i32, [20 x i8] }
%struct._IO_marker = type opaque
%struct._IO_codecvt = type opaque
%struct._IO_wide_data = type opaque
%"struct.std::array" = type { [1024 x i32] }

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_obj3.cpp, ptr null }]
@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZSt3cin = external dso_local global %"class.std::basic_istream", align 8
@stderr = external dso_local local_unnamed_addr global ptr, align 8
@.str = private unnamed_addr constant [11 x i8] c"\0A sum = %f\00", align 1
@.str.1 = private unnamed_addr constant [49 x i8] c"cannot create std::vector larger than max_size()\00", align 1

; Function Attrs: nofree uwtable
define internal void @_GLOBAL__sub_I_obj3.cpp() #0 section ".text.startup" {
bb:
  tail call void @_ZNSt8ios_base4InitC1Ev(ptr noundef nonnull align 1 dereferenceable(1) @_ZStL8__ioinit)
  %i = tail call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr @_ZStL8__ioinit, ptr nonnull @__dso_handle) #11
  ret void
}

; Function Attrs: nofree
declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #1

; Function Attrs: nofree nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr noundef nonnull align 1 dereferenceable(1)) unnamed_addr #2

; Function Attrs: nofree nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #3

; Function Attrs: norecurse uwtable
define internal fastcc void @_Z16gemm_lowp_matmuliiiPKiS0_PiiSt5arrayIiLm1024EE(i32 noundef %arg, i32 noundef %arg1, i32 noundef %arg2, ptr noalias nocapture noundef readonly %arg3, ptr noalias nocapture noundef readonly %arg4, ptr noalias nocapture noundef %arg5, ptr noalias nocapture noundef readonly byval(%"struct.std::array") align 8 %arg6) unnamed_addr #4 personality ptr @__gxx_personality_v0 {
bb:
  %i = sext i32 %arg1 to i64
  %i7 = icmp slt i32 %arg1, 0
  br i1 %i7, label %bb8, label %bb9

bb8:                                              ; preds = %bb
  tail call void @_ZSt20__throw_length_errorPKc(ptr noundef @.str.1) #12
  unreachable

bb9:                                              ; preds = %bb
  %i10 = icmp eq i32 %arg1, 0
  br i1 %i10, label %bb15, label %bb11

bb11:                                             ; preds = %bb9
  %i12 = shl nuw nsw i64 %i, 2
  %i13 = tail call noalias noundef nonnull ptr @_Znwm(i64 noundef %i12) #13
  %i14 = bitcast ptr %i13 to ptr
  tail call void @llvm.memset.p0.i64(ptr nonnull align 4 %i13, i8 0, i64 %i12, i1 false), !tbaa !6
  br label %bb15

bb15:                                             ; preds = %bb11, %bb9
  %i16 = phi ptr [ %i14, %bb11 ], [ null, %bb9 ]
  %i17 = sext i32 %arg to i64
  %i18 = icmp slt i32 %arg, 0
  br i1 %i18, label %bb19, label %bb21

bb19:                                             ; preds = %bb15
  invoke void @_ZSt20__throw_length_errorPKc(ptr noundef @.str.1) #12
          to label %bb20 unwind label %bb76

bb20:                                             ; preds = %bb19
  unreachable

bb21:                                             ; preds = %bb15
  %i22 = icmp eq i32 %arg, 0
  br i1 %i22, label %bb51, label %bb23

bb23:                                             ; preds = %bb21
  %i24 = shl nuw nsw i64 %i17, 2
  %i25 = invoke noalias noundef nonnull ptr @_Znwm(i64 noundef %i24) #13
          to label %bb26 unwind label %bb76

bb26:                                             ; preds = %bb23
  %i27 = bitcast ptr %i25 to ptr
  tail call void @llvm.memset.p0.i64(ptr nonnull align 4 %i25, i8 0, i64 %i24, i1 false), !tbaa !6
  %i28 = icmp sgt i32 %arg2, 0
  br i1 %i28, label %bb30, label %bb29

bb29:                                             ; preds = %bb26
  br label %bb81

bb30:                                             ; preds = %bb26
  %i31 = zext i32 %arg2 to i64
  br label %bb32

bb32:                                             ; preds = %bb45, %bb30
  %i33 = phi i64 [ 0, %bb30 ], [ %i47, %bb45 ]
  %i34 = getelementptr inbounds i32, ptr %i27, i64 %i33, !intel-tbaa !6
  store i32 0, ptr %i34, align 4, !tbaa !6
  %i35 = mul nuw nsw i64 %i33, %i31
  br label %bb36

bb36:                                             ; preds = %bb36, %bb32
  %i37 = phi i32 [ 0, %bb32 ], [ %i42, %bb36 ]
  %i38 = phi i64 [ 0, %bb32 ], [ %i43, %bb36 ]
  %i39 = add nuw nsw i64 %i38, %i35
  %i40 = getelementptr inbounds i32, ptr %arg3, i64 %i39
  %i41 = load i32, ptr %i40, align 4, !tbaa !6
  %i42 = add nsw i32 %i37, %i41
  %i43 = add nuw nsw i64 %i38, 1
  %i44 = icmp eq i64 %i43, %i31
  br i1 %i44, label %bb45, label %bb36, !llvm.loop !10

bb45:                                             ; preds = %bb36
  %i46 = phi i32 [ %i42, %bb36 ]
  store i32 %i46, ptr %i34, align 4, !tbaa !6
  %i47 = add nuw nsw i64 %i33, 1
  %i48 = icmp eq i64 %i47, %i17
  br i1 %i48, label %bb49, label %bb32, !llvm.loop !12

bb49:                                             ; preds = %bb45
  br label %bb51

bb50:                                             ; preds = %bb81
  br label %bb51

bb51:                                             ; preds = %bb50, %bb49, %bb21
  %i52 = phi ptr [ null, %bb21 ], [ %i27, %bb49 ], [ %i27, %bb50 ]
  %i53 = icmp sgt i32 %arg1, 0
  br i1 %i53, label %bb54, label %bb141

bb54:                                             ; preds = %bb51
  %i55 = icmp sgt i32 %arg2, 0
  br i1 %i55, label %bb57, label %bb56

bb56:                                             ; preds = %bb54
  br label %bb136

bb57:                                             ; preds = %bb54
  %i58 = zext i32 %arg2 to i64
  br label %bb59

bb59:                                             ; preds = %bb72, %bb57
  %i60 = phi i64 [ 0, %bb57 ], [ %i74, %bb72 ]
  %i61 = getelementptr inbounds i32, ptr %i16, i64 %i60, !intel-tbaa !6
  store i32 0, ptr %i61, align 4, !tbaa !6
  br label %bb62

bb62:                                             ; preds = %bb62, %bb59
  %i63 = phi i32 [ 0, %bb59 ], [ %i69, %bb62 ]
  %i64 = phi i64 [ 0, %bb59 ], [ %i70, %bb62 ]
  %i65 = mul nsw i64 %i64, %i
  %i66 = add nsw i64 %i65, %i60
  %i67 = getelementptr inbounds i32, ptr %arg4, i64 %i66
  %i68 = load i32, ptr %i67, align 4, !tbaa !6
  %i69 = add nsw i32 %i63, %i68
  %i70 = add nuw nsw i64 %i64, 1
  %i71 = icmp eq i64 %i70, %i58
  br i1 %i71, label %bb72, label %bb62, !llvm.loop !13

bb72:                                             ; preds = %bb62
  %i73 = phi i32 [ %i69, %bb62 ]
  store i32 %i73, ptr %i61, align 4, !tbaa !6
  %i74 = add nuw nsw i64 %i60, 1
  %i75 = icmp eq i64 %i74, %i
  br i1 %i75, label %bb86, label %bb59, !llvm.loop !14

bb76:                                             ; preds = %bb23, %bb19
  %i77 = landingpad { ptr, i32 }
          cleanup
  %i78 = icmp eq ptr %i16, null
  br i1 %i78, label %bb151, label %bb79

bb79:                                             ; preds = %bb76
  %i80 = bitcast ptr %i16 to ptr
  tail call void @_ZdlPv(ptr noundef nonnull %i80) #11
  br label %bb151

bb81:                                             ; preds = %bb81, %bb29
  %i82 = phi i64 [ %i84, %bb81 ], [ 0, %bb29 ]
  %i83 = getelementptr inbounds i32, ptr %i27, i64 %i82, !intel-tbaa !6
  store i32 0, ptr %i83, align 4, !tbaa !6
  %i84 = add nuw nsw i64 %i82, 1
  %i85 = icmp eq i64 %i84, %i17
  br i1 %i85, label %bb50, label %bb81, !llvm.loop !12

bb86:                                             ; preds = %bb72
  br label %bb88

bb87:                                             ; preds = %bb136
  br label %bb88

bb88:                                             ; preds = %bb87, %bb86
  br i1 %i22, label %bb141, label %bb89

bb89:                                             ; preds = %bb88
  %i90 = sext i32 %arg2 to i64
  br label %bb91

bb91:                                             ; preds = %bb133, %bb89
  %i92 = phi i64 [ 0, %bb89 ], [ %i134, %bb133 ]
  %i93 = mul nsw i64 %i92, %i
  %i94 = mul nsw i64 %i92, %i90
  %i95 = getelementptr inbounds i32, ptr %i52, i64 %i92, !intel-tbaa !6
  %i96 = load i32, ptr %i95, align 4, !tbaa !6
  br label %bb97

bb97:                                             ; preds = %bb107, %bb91
  %i98 = phi i64 [ 0, %bb91 ], [ %i117, %bb107 ]
  %i99 = getelementptr inbounds %"struct.std::array", ptr %arg6, i64 0, i32 0, i64 %i98, !intel-tbaa !15
  %i100 = load i32, ptr %i99, align 4, !tbaa !15
  %i101 = add nsw i64 %i98, %i93
  %i102 = getelementptr inbounds i32, ptr %arg5, i64 %i101
  %i103 = load i32, ptr %i102, align 4, !tbaa !6
  br i1 %i55, label %bb104, label %bb107

bb104:                                            ; preds = %bb97
  br label %bb119

bb105:                                            ; preds = %bb119
  %i106 = phi i32 [ %i130, %bb119 ]
  br label %bb107

bb107:                                            ; preds = %bb105, %bb97
  %i108 = phi i32 [ %i103, %bb97 ], [ %i106, %bb105 ]
  %i109 = getelementptr inbounds i32, ptr %i16, i64 %i98, !intel-tbaa !6
  %i110 = load i32, ptr %i109, align 4, !tbaa !6
  %i111 = mul i32 %i96, %i100
  %i112 = mul i32 %i100, %arg2
  %i113 = add i32 %i111, %i110
  %i114 = add i32 %i112, %i103
  %i115 = add i32 %i114, %i108
  %i116 = sub i32 %i115, %i113
  store i32 %i116, ptr %i102, align 4, !tbaa !6
  %i117 = add nuw nsw i64 %i98, 1
  %i118 = icmp eq i64 %i117, %i
  br i1 %i118, label %bb133, label %bb97, !llvm.loop !18

bb119:                                            ; preds = %bb119, %bb104
  %i120 = phi i32 [ %i131, %bb119 ], [ 0, %bb104 ]
  %i121 = phi i32 [ %i130, %bb119 ], [ %i103, %bb104 ]
  %isext = sext i32 %i120 to i64
  %i122 = add nsw i64 %isext, %i94
  %i123 = getelementptr inbounds i32, ptr %arg3, i64 %i122
  %i124 = load i32, ptr %i123, align 4, !tbaa !6
  %i125 = mul nsw i64 %isext, %i
  %i126 = add nsw i64 %i125, %i98
  %i127 = getelementptr inbounds i32, ptr %arg4, i64 %i126
  %i128 = load i32, ptr %i127, align 4, !tbaa !6
  %i129 = mul nsw i32 %i128, %i124
  %i130 = add nsw i32 %i129, %i121
  %i131 = add nuw nsw i32 %i120, 1
  %i132 = icmp eq i32 %i131, %arg2
  br i1 %i132, label %bb105, label %bb119, !llvm.loop !19

bb133:                                            ; preds = %bb107
  %i134 = add nuw nsw i64 %i92, 1
  %i135 = icmp eq i64 %i134, %i17
  br i1 %i135, label %bb143, label %bb91, !llvm.loop !20

bb136:                                            ; preds = %bb136, %bb56
  %i137 = phi i64 [ %i139, %bb136 ], [ 0, %bb56 ]
  %i138 = getelementptr inbounds i32, ptr %i16, i64 %i137, !intel-tbaa !6
  store i32 0, ptr %i138, align 4, !tbaa !6
  %i139 = add nuw nsw i64 %i137, 1
  %i140 = icmp eq i64 %i139, %i
  br i1 %i140, label %bb87, label %bb136, !llvm.loop !14

bb141:                                            ; preds = %bb88, %bb51
  %i142 = icmp eq ptr %i52, null
  br i1 %i142, label %bb146, label %bb144

bb143:                                            ; preds = %bb133
  br label %bb144

bb144:                                            ; preds = %bb143, %bb141
  %i145 = bitcast ptr %i52 to ptr
  tail call void @_ZdlPv(ptr noundef nonnull %i145) #11
  br label %bb146

bb146:                                            ; preds = %bb144, %bb141
  %i147 = icmp eq ptr %i16, null
  br i1 %i147, label %bb150, label %bb148

bb148:                                            ; preds = %bb146
  %i149 = bitcast ptr %i16 to ptr
  tail call void @_ZdlPv(ptr noundef nonnull %i149) #11
  br label %bb150

bb150:                                            ; preds = %bb148, %bb146
  ret void

bb151:                                            ; preds = %bb79, %bb76
  resume { ptr, i32 } %i77
}

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nofree noreturn
declare dso_local void @_ZSt20__throw_length_errorPKc(ptr noundef) local_unnamed_addr #5

; Function Attrs: nobuiltin allocsize(0)
declare dso_local noundef nonnull ptr @_Znwm(i64 noundef) local_unnamed_addr #6

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #7

; Function Attrs: nobuiltin nounwind
declare dso_local void @_ZdlPv(ptr noundef) local_unnamed_addr #8

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() local_unnamed_addr #9 {
bb:
  %i = alloca i32, align 4
  %i1 = alloca i32, align 4
  %i2 = alloca i32, align 4
  %i3 = alloca [1048576 x i32], align 16
  %i4 = alloca [1048576 x i32], align 16
  %i5 = alloca [1048576 x i32], align 16
  %i6 = bitcast ptr %i5 to ptr
  %i7 = alloca %"struct.std::array", align 8
  %i8 = bitcast ptr %i to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i8) #11
  %i9 = bitcast ptr %i1 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i9) #11
  %i10 = bitcast ptr %i2 to ptr
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i10) #11
  %i11 = bitcast ptr %i3 to ptr
  call void @llvm.lifetime.start.p0(i64 4194304, ptr nonnull %i11) #11
  %i12 = bitcast ptr %i4 to ptr
  call void @llvm.lifetime.start.p0(i64 4194304, ptr nonnull %i12) #11
  call void @llvm.lifetime.start.p0(i64 4194304, ptr nonnull %i6) #11
  %i13 = call noundef nonnull align 8 dereferenceable(16) ptr @_ZNSirsERi(ptr noundef nonnull align 8 dereferenceable(16) @_ZSt3cin, ptr noundef nonnull align 4 dereferenceable(4) %i)
  %i14 = call noundef nonnull align 8 dereferenceable(16) ptr @_ZNSirsERi(ptr noundef nonnull align 8 dereferenceable(16) @_ZSt3cin, ptr noundef nonnull align 4 dereferenceable(4) %i1)
  %i15 = call noundef nonnull align 8 dereferenceable(16) ptr @_ZNSirsERi(ptr noundef nonnull align 8 dereferenceable(16) @_ZSt3cin, ptr noundef nonnull align 4 dereferenceable(4) %i2)
  %i16 = bitcast ptr %i7 to ptr
  call void @llvm.lifetime.start.p0(i64 4096, ptr nonnull %i16) #11
  %i17 = load i32, ptr %i, align 4, !tbaa !6
  %i18 = icmp sgt i32 %i17, 0
  br i1 %i18, label %bb19, label %bb45

bb19:                                             ; preds = %bb
  %i20 = load i32, ptr %i2, align 4, !tbaa !6
  %i21 = icmp sgt i32 %i20, 0
  br i1 %i21, label %bb22, label %bb40

bb22:                                             ; preds = %bb19
  %i23 = zext i32 %i20 to i64
  %i24 = zext i32 %i17 to i64
  br label %bb25

bb25:                                             ; preds = %bb34, %bb22
  %i26 = phi i64 [ 0, %bb22 ], [ %i35, %bb34 ]
  %i27 = mul nuw nsw i64 %i26, %i23
  br label %bb28

bb28:                                             ; preds = %bb28, %bb25
  %i29 = phi i64 [ 0, %bb25 ], [ %i32, %bb28 ]
  %i30 = add nuw nsw i64 %i27, %i29
  %i31 = getelementptr inbounds [1048576 x i32], ptr %i3, i64 0, i64 %i30, !intel-tbaa !21
  store i32 1, ptr %i31, align 4, !tbaa !21
  %i32 = add nuw nsw i64 %i29, 1
  %i33 = icmp eq i64 %i32, %i23
  br i1 %i33, label %bb34, label %bb28, !llvm.loop !23

bb34:                                             ; preds = %bb28
  %i35 = add nuw nsw i64 %i26, 1
  %i36 = icmp eq i64 %i35, %i24
  br i1 %i36, label %bb37, label %bb25, !llvm.loop !24

bb37:                                             ; preds = %bb34
  %i38 = load i32, ptr %i1, align 4, !tbaa !6
  %i39 = icmp sgt i32 %i38, 0
  br i1 %i39, label %bb53, label %bb76

bb40:                                             ; preds = %bb19
  %i41 = load i32, ptr %i1, align 4, !tbaa !6
  %i42 = icmp sgt i32 %i41, 0
  br i1 %i42, label %bb70, label %bb43

bb43:                                             ; preds = %bb40
  %i44 = zext i32 %i17 to i64
  br label %bb76

bb45:                                             ; preds = %bb
  %i46 = load i32, ptr %i1, align 4, !tbaa !6
  %i47 = icmp sgt i32 %i46, 0
  %i48 = load i32, ptr %i2, align 4
  %i49 = icmp sgt i32 %i48, 0
  %i50 = select i1 %i47, i1 %i49, i1 false
  br i1 %i50, label %bb51, label %bb79

bb51:                                             ; preds = %bb45
  %i52 = zext i32 %i48 to i64
  br label %bb53

bb53:                                             ; preds = %bb51, %bb37
  %i54 = phi i64 [ %i52, %bb51 ], [ %i23, %bb37 ]
  %i55 = phi i32 [ %i46, %bb51 ], [ %i38, %bb37 ]
  %i56 = zext i32 %i55 to i64
  br label %bb57

bb57:                                             ; preds = %bb66, %bb53
  %i58 = phi i64 [ 0, %bb53 ], [ %i67, %bb66 ]
  br label %bb59

bb59:                                             ; preds = %bb59, %bb57
  %i60 = phi i64 [ 0, %bb57 ], [ %i64, %bb59 ]
  %i61 = mul nuw nsw i64 %i60, %i56
  %i62 = add nuw nsw i64 %i61, %i58
  %i63 = getelementptr inbounds [1048576 x i32], ptr %i4, i64 0, i64 %i62, !intel-tbaa !21
  store i32 2, ptr %i63, align 4, !tbaa !21
  %i64 = add nuw nsw i64 %i60, 1
  %i65 = icmp eq i64 %i64, %i54
  br i1 %i65, label %bb66, label %bb59, !llvm.loop !25

bb66:                                             ; preds = %bb59
  %i67 = add nuw nsw i64 %i58, 1
  %i68 = icmp eq i64 %i67, %i56
  br i1 %i68, label %bb69, label %bb57, !llvm.loop !26

bb69:                                             ; preds = %bb66
  br i1 %i18, label %bb70, label %bb79

bb70:                                             ; preds = %bb69, %bb40
  %i71 = phi i32 [ %i41, %bb40 ], [ %i55, %bb69 ]
  %i72 = zext i32 %i71 to i64
  %i73 = zext i32 %i17 to i64
  %i74 = mul nuw nsw i64 %i72, %i73
  %i75 = shl i64 %i74, 2
  call void @llvm.memset.p0.i64(ptr nonnull align 16 %i6, i8 0, i64 %i75, i1 false), !tbaa !21
  br label %bb76

bb76:                                             ; preds = %bb70, %bb43, %bb37
  %i77 = phi i64 [ %i44, %bb43 ], [ %i73, %bb70 ], [ %i24, %bb37 ]
  br label %bb83

bb78:                                             ; preds = %bb83
  br label %bb79

bb79:                                             ; preds = %bb78, %bb69, %bb45
  %i80 = getelementptr inbounds [1048576 x i32], ptr %i3, i64 0, i64 0, !intel-tbaa !21
  %i81 = getelementptr inbounds [1048576 x i32], ptr %i4, i64 0, i64 0, !intel-tbaa !21
  %i82 = getelementptr inbounds [1048576 x i32], ptr %i5, i64 0, i64 0, !intel-tbaa !21
  br label %bb116

bb83:                                             ; preds = %bb83, %bb76
  %i84 = phi i64 [ 0, %bb76 ], [ %i87, %bb83 ]
  %i85 = getelementptr inbounds %"struct.std::array", ptr %i7, i64 0, i32 0, i64 %i84, !intel-tbaa !15
  %i86 = trunc i64 %i84 to i32
  store i32 %i86, ptr %i85, align 4, !tbaa !15
  %i87 = add nuw nsw i64 %i84, 1
  %i88 = icmp eq i64 %i87, %i77
  br i1 %i88, label %bb78, label %bb83, !llvm.loop !27

bb89:                                             ; preds = %bb116
  %i90 = phi i32 [ %i121, %bb116 ]
  %i91 = icmp sgt i32 %i90, 0
  br i1 %i91, label %bb92, label %bb129

bb92:                                             ; preds = %bb89
  %i93 = load i32, ptr %i1, align 4, !tbaa !6
  %i94 = icmp sgt i32 %i93, 0
  br i1 %i94, label %bb95, label %bb129

bb95:                                             ; preds = %bb92
  %i96 = zext i32 %i93 to i64
  %i97 = zext i32 %i90 to i64
  br label %bb98

bb98:                                             ; preds = %bb112, %bb95
  %i99 = phi i64 [ 0, %bb95 ], [ %i114, %bb112 ]
  %i100 = phi double [ 0.000000e+00, %bb95 ], [ %i113, %bb112 ]
  %i101 = mul nuw nsw i64 %i99, %i96
  br label %bb102

bb102:                                            ; preds = %bb102, %bb98
  %i103 = phi i64 [ 0, %bb98 ], [ %i110, %bb102 ]
  %i104 = phi double [ %i100, %bb98 ], [ %i109, %bb102 ]
  %i105 = add nuw nsw i64 %i101, %i103
  %i106 = getelementptr inbounds [1048576 x i32], ptr %i5, i64 0, i64 %i105, !intel-tbaa !21
  %i107 = load i32, ptr %i106, align 4, !tbaa !21
  %i108 = sitofp i32 %i107 to double
  %i109 = fadd fast double %i104, %i108
  %i110 = add nuw nsw i64 %i103, 1
  %i111 = icmp eq i64 %i110, %i96
  br i1 %i111, label %bb112, label %bb102, !llvm.loop !28

bb112:                                            ; preds = %bb102
  %i113 = phi double [ %i109, %bb102 ]
  %i114 = add nuw nsw i64 %i99, 1
  %i115 = icmp eq i64 %i114, %i97
  br i1 %i115, label %bb127, label %bb98, !llvm.loop !29

bb116:                                            ; preds = %bb116, %bb79
  %i117 = phi i32 [ %i17, %bb79 ], [ %i121, %bb116 ]
  %i118 = phi i32 [ 0, %bb79 ], [ %i125, %bb116 ]
  %i119 = load i32, ptr %i1, align 4, !tbaa !6
  %i120 = load i32, ptr %i2, align 4, !tbaa !6
  call fastcc void @_Z16gemm_lowp_matmuliiiPKiS0_PiiSt5arrayIiLm1024EE(i32 noundef %i117, i32 noundef %i119, i32 noundef %i120, ptr noundef nonnull %i80, ptr noundef nonnull %i81, ptr noundef nonnull %i82, ptr noundef nonnull byval(%"struct.std::array") align 8 %i7) #14
  %i121 = load i32, ptr %i, align 4, !tbaa !6
  %i122 = srem i32 %i118, %i121
  %i123 = zext i32 %i122 to i64
  %i124 = getelementptr inbounds [1048576 x i32], ptr %i5, i64 0, i64 %i123, !intel-tbaa !21
  store i32 %i122, ptr %i124, align 4, !tbaa !21
  %i125 = add nuw nsw i32 %i118, 1
  %i126 = icmp eq i32 %i125, 100
  br i1 %i126, label %bb89, label %bb116, !llvm.loop !30

bb127:                                            ; preds = %bb112
  %i128 = phi double [ %i113, %bb112 ]
  br label %bb129

bb129:                                            ; preds = %bb127, %bb92, %bb89
  %i130 = phi double [ 0.000000e+00, %bb89 ], [ 0.000000e+00, %bb92 ], [ %i128, %bb127 ]
  %i131 = load ptr, ptr @stderr, align 8, !tbaa !31
  %i132 = call i32 (ptr, ptr, ...) @fprintf(ptr noundef %i131, ptr noundef @.str, double noundef %i130) #15
  call void @llvm.lifetime.end.p0(i64 4096, ptr nonnull %i16) #11
  call void @llvm.lifetime.end.p0(i64 4194304, ptr nonnull %i6) #11
  call void @llvm.lifetime.end.p0(i64 4194304, ptr nonnull %i12) #11
  call void @llvm.lifetime.end.p0(i64 4194304, ptr nonnull %i11) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i10) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i9) #11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i8) #11
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #10

; Function Attrs: nofree
declare dso_local noundef nonnull align 8 dereferenceable(16) ptr @_ZNSirsERi(ptr noundef nonnull align 8 dereferenceable(16), ptr noundef nonnull align 4 dereferenceable(4)) local_unnamed_addr #1

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @fprintf(ptr nocapture noundef, ptr nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #10

attributes #0 = { nofree uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nofree "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind }
attributes #4 = { norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nofree noreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #8 = { nobuiltin nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #9 = { mustprogress norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #10 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
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
