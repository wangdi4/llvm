; RUN: opt %s -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-temp-array-transpose"  -print-after=hir-temp-array-transpose -disable-output 2>&1 | FileCheck %s


; Check that we transpose the array with following dims: [i4][i3 + sext.i32.i64(%phi73) + -1]
; Even though i3 is not standalone and %phi73 is non-linear, the dimsizes are constant,
; which allows us to copy the contents of the array and use the transposed copy.

; Note that there are 3 uses in 3 loops where we reuse the copied transpose temparray.

; HIR before transformation
;        BEGIN REGION { }
;              + DO i1 = 0, zext.i32.i64(%load4) + -1, 1
;              |      %phi17 = 1;
;              |      %phi18 = 0;
;              |   + DO i2 = 0, sext.i32.i64(%load8) + -1, 1
;              |   |   %phi18.out = %phi18;
;              |   |   %load20 = (getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523600))[i2];
;              |   |   %phi18 = %load20 + %phi18  +  1;
;              |   |   if (%phi18 >= %phi17)
;              |   |   {
;              |   |      %load27 = (getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523720))[i2];
;              |   |
;              |   |      + DO i3 = 0, zext.i32.i64((1 + (-1 * %phi17) + %load20 + %phi18.out)), 1
;              |   |      |   %phi50 = 0.000000e+00;
;              |   |      |
;              |   |      |      %phi38 = 0.000000e+00;
;              |   |      |   + DO i4 = 0, sext.i32.i64(%load20), 1
;              |   |      |   |   %fmul = (@global.2)[i1][i4 + sext.i32.i64(%phi17) + -1]  *  (getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2484000))[i4][i3 + sext.i32.i64(%phi17) + -1];
;              |   |      |   |   %phi38 = %fmul  +  %phi38;
;              |   |      |   + END LOOP
;              |   |      |      %phi50 = %phi38;
;              |   |      |
;              |   |      |   %fmul51 = %load27  *  %phi50;
;              |   |      |   (@global.5)[i1][i3 + sext.i32.i64(%phi17) + -1] = %fmul51;
;              |   |      + END LOOP
;              |   |   }
;              |   |   %phi17 = %load20 + %phi17  +  1;
;              |   + END LOOP
;              + END LOOP

; CHECK: BEGIN REGION { modified }
; CHECK:       %call15 = @llvm.stacksave.p0();
; CHECK:       %TranspTmpArr = alloca 39600;
;
;              + DO i1 = 0, 149, 1
;              |   + DO i2 = 0, 32, 1
;              |   |   (%TranspTmpArr)[i1][i2] = (getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2484000))[i2][i1];
;              |   + END LOOP
;              + END LOOP
;
;
;              + DO i1 = 0, zext.i32.i64(%load4) + -1, 1
;              |      %phi17 = 1;
;              |      %phi18 = 0;
;              |   + DO i2 = 0, sext.i32.i64(%load8) + -1, 1
;              |   |   %phi18.out = %phi18;
;              |   |   %load20 = (getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523600))[i2];
;              |   |   %phi18 = %load20 + %phi18  +  1;
;              |   |   if (%phi18 >= %phi17)
;              |   |   {
;              |   |      %load27 = (getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523720))[i2];
;              |   |
;              |   |      + DO i3 = 0, zext.i32.i64((1 + (-1 * %phi17) + %load20 + %phi18.out)), 1
;              |   |      |   %phi50 = 0.000000e+00;
;              |   |      |
;              |   |      |      %phi38 = 0.000000e+00;
;              |   |      |   + DO i4 = 0, sext.i32.i64(%load20), 1
; CHECK:       |   |      |   |   %fmul = (@global.2)[i1][i4 + sext.i32.i64(%phi17) + -1]  *  (%TranspTmpArr)[i3 + sext.i32.i64(%phi17) + -1][i4];

; CHECK:       |   |      |   |   %fmul102 = (getelementptr inbounds ([1092056 x i8], ptr @global.2, i64 0, i64 180000))[i1][i4 + sext.i32.i64(%phi73) + -1]  *  (%TranspTmpArr)[i3 + sext.i32.i64(%phi73) + -1][i4];

; CHECK:       |   |      |   |   %fmul161 = (getelementptr inbounds ([1092056 x i8], ptr @global.2, i64 0, i64 720000))[i1][i4 + sext.i32.i64(%phi132) + -1]  *  (%TranspTmpArr)[i3 + sext.i32.i64(%phi132) + -1][i4];




target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@global = external hidden unnamed_addr global [2523968 x i8], align 32
@global.1 = external hidden unnamed_addr global [2523968 x i8], align 32
@global.2 = external hidden unnamed_addr global [1092056 x i8], align 32
@global.3 = external hidden unnamed_addr global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.4 = external hidden unnamed_addr global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.5 = external hidden unnamed_addr global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.6 = external hidden global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.7 = external hidden global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.8 = external hidden global [150 x [150 x double]], align 16, !llfort.type_idx !0
@global.9 = external hidden global [150 x [150 x double]], align 16, !llfort.type_idx !0

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

; Function Attrs: nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none)

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.experimental.noalias.scope.decl(metadata) #1

; Function Attrs: nofree nosync nounwind uwtable
declare hidden fastcc void @hoge(i32, i32, ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8)) unnamed_addr #2

; Function Attrs: nofree nosync nounwind uwtable
declare hidden fastcc void @barney(i32, ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8)) unnamed_addr #2

; Function Attrs: nofree nosync nounwind uwtable
declare hidden fastcc void @wibble(i32, i32, ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8)) unnamed_addr #2

; Function Attrs: nofree nosync nounwind uwtable
declare hidden fastcc void @spam(i32, i32, ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8), ptr noalias nocapture writeonly dereferenceable(8)) unnamed_addr #2

; Function Attrs: nounwind uwtable
define dso_local i1 @wobble(ptr %arg, ptr %arg1, ptr %arg2) #3 {
bb:
  br label %bb3

bb3:                                              ; preds = %bb
  call fastcc void @hoge(i32 1, i32 1, ptr @global.9, ptr @global.8, ptr @global.7, ptr @global.6) #4
  call fastcc void @barney(i32 1, ptr @global.9, ptr @global.8, ptr @global.7, ptr @global.6) #4
  call fastcc void @wibble(i32 1, i32 1, ptr @global.9, ptr @global.8, ptr @global.7, ptr @global.6) #4
  %load = load i32, ptr getelementptr inbounds ([2523968 x i8], ptr @global.1, i64 0, i64 2523964), align 4, !tbaa !4, !llfort.type_idx !9
  %add = add nsw i32 %load, -1
  call fastcc void @spam(i32 2, i32 %add, ptr @global.9, ptr @global.8, ptr @global.7, ptr @global.6) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !10) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !13) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !15) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !17) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !19) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !21) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !23) #4
  %load4 = load i32, ptr getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523964), align 4, !tbaa !25, !alias.scope !23, !noalias !30, !llfort.type_idx !31
  store i32 %load4, ptr %arg, align 4
  %icmp = icmp slt i32 %load4, 1
  store i1 %icmp, ptr %arg1, align 1
  br i1 %icmp, label %bb5, label %bb7

bb5:                                              ; preds = %bb3
  %load6 = load i32, ptr getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523960), align 8, !tbaa !32
  br label %bb184

bb7:                                              ; preds = %bb3
  %load8 = load i32, ptr getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523960), align 8, !tbaa !34, !alias.scope !21, !noalias !36, !llfort.type_idx !37
  %icmp9 = icmp slt i32 %load8, 1
  %add10 = add nuw nsw i32 %load8, 1
  %add11 = add nuw nsw i32 %load4, 1
  %zext = zext i32 %add11 to i64
  %sext = sext i32 %add10 to i64
  br label %bb12

bb12:                                             ; preds = %bb62, %bb7
  %phi = phi i64 [ 1, %bb7 ], [ %add63, %bb62 ]
  br i1 %icmp9, label %bb62, label %bb13

bb13:                                             ; preds = %bb12
  %call = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1200, ptr nonnull elementtype(double) @global.5, i64 %phi) #4
  %call14 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1200, ptr nonnull elementtype(double) @global.2, i64 %phi) #4
  br label %bb15

bb15:                                             ; preds = %bb56, %bb13
  %phi16 = phi i64 [ 1, %bb13 ], [ %add59, %bb56 ]
  %phi17 = phi i32 [ 1, %bb13 ], [ %add58, %bb56 ]
  %phi18 = phi i32 [ 0, %bb13 ], [ %add22, %bb56 ]
  %call19 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523600), i64 %phi16) #4, !llfort.type_idx !38, !ifx.array_extent !39
  %load20 = load i32, ptr %call19, align 1, !tbaa !40, !alias.scope !19, !noalias !42
  %add21 = add nsw i32 %phi18, %load20
  %add22 = add nsw i32 %add21, 1
  %icmp23 = icmp slt i32 %add22, %phi17
  br i1 %icmp23, label %bb56, label %bb24

bb24:                                             ; preds = %bb15
  %icmp25 = icmp slt i32 %load20, 0
  %call26 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523720), i64 %phi16) #4, !llfort.type_idx !43, !ifx.array_extent !39
  %load27 = load double, ptr %call26, align 1, !tbaa !44, !alias.scope !17, !noalias !46, !llfort.type_idx !47
  %sext28 = sext i32 %phi17 to i64
  %add29 = add nuw nsw i32 %load20, 1
  %add30 = add i32 %phi18, 2
  %add31 = add nsw i32 %add30, %load20
  %sext32 = sext i32 %add29 to i64
  br label %bb33

bb33:                                             ; preds = %bb49, %bb24
  %phi34 = phi i64 [ %sext28, %bb24 ], [ %add53, %bb49 ]
  br i1 %icmp25, label %bb49, label %bb35

bb35:                                             ; preds = %bb33
  br label %bb36

bb36:                                             ; preds = %bb36, %bb35
  %phi37 = phi i64 [ %add39, %bb36 ], [ 0, %bb35 ]
  %phi38 = phi double [ %fadd, %bb36 ], [ 0.000000e+00, %bb35 ]
  %add39 = add nuw nsw i64 %phi37, 1
  %call40 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1200, ptr nonnull elementtype(double) getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2484000), i64 %add39) #4, !llfort.type_idx !48, !ifx.array_extent !49
  %call41 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %call40, i64 %phi34) #4, !llfort.type_idx !50
  %load42 = load double, ptr %call41, align 1, !tbaa !51, !alias.scope !10, !noalias !53, !llfort.type_idx !54
  %add43 = add nsw i64 %phi37, %sext28
  %call44 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %call14, i64 %add43) #4, !llfort.type_idx !55
  %load45 = load double, ptr %call44, align 1, !tbaa !56, !alias.scope !13, !noalias !58, !llfort.type_idx !59
  %fmul = fmul fast double %load45, %load42
  %fadd = fadd fast double %fmul, %phi38
  %icmp46 = icmp eq i64 %add39, %sext32
  br i1 %icmp46, label %bb47, label %bb36

bb47:                                             ; preds = %bb36
  %phi48 = phi double [ %fadd, %bb36 ]
  br label %bb49

bb49:                                             ; preds = %bb47, %bb33
  %phi50 = phi double [ 0.000000e+00, %bb33 ], [ %phi48, %bb47 ]
  %fmul51 = fmul fast double %load27, %phi50
  %call52 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %call, i64 %phi34) #4, !llfort.type_idx !60
  store double %fmul51, ptr %call52, align 1, !tbaa !61, !alias.scope !15, !noalias !63
  %add53 = add nsw i64 %phi34, 1
  %trunc = trunc i64 %add53 to i32
  %icmp54 = icmp eq i32 %add31, %trunc
  br i1 %icmp54, label %bb55, label %bb33

bb55:                                             ; preds = %bb49
  br label %bb56

bb56:                                             ; preds = %bb55, %bb15
  %add57 = add nsw i32 %phi17, %load20
  %add58 = add nsw i32 %add57, 1
  %add59 = add nuw nsw i64 %phi16, 1
  %icmp60 = icmp eq i64 %add59, %sext
  br i1 %icmp60, label %bb61, label %bb15

bb61:                                             ; preds = %bb56
  br label %bb62

bb62:                                             ; preds = %bb61, %bb12
  %add63 = add nuw nsw i64 %phi, 1
  %icmp64 = icmp eq i64 %add63, %zext
  br i1 %icmp64, label %bb65, label %bb12

bb65:                                             ; preds = %bb62
  call void @llvm.experimental.noalias.scope.decl(metadata !64) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !67) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !69) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !71) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !73) #4
  br label %bb66

bb66:                                             ; preds = %bb121, %bb65
  %phi67 = phi i64 [ 1, %bb65 ], [ %add122, %bb121 ]
  br i1 %icmp9, label %bb121, label %bb68

bb68:                                             ; preds = %bb66
  %call69 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1200, ptr nonnull elementtype(double) @global.4, i64 %phi67) #4
  %call70 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1200, ptr nonnull elementtype(double) getelementptr inbounds ([1092056 x i8], ptr @global.2, i64 0, i64 180000), i64 %phi67) #4
  br label %bb71

bb71:                                             ; preds = %bb115, %bb68
  %phi72 = phi i64 [ 1, %bb68 ], [ %add118, %bb115 ]
  %phi73 = phi i32 [ 1, %bb68 ], [ %add117, %bb115 ]
  %phi74 = phi i32 [ 0, %bb68 ], [ %add78, %bb115 ]
  %call75 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523600), i64 %phi72) #4, !llfort.type_idx !38, !ifx.array_extent !39
  %load76 = load i32, ptr %call75, align 1, !tbaa !75, !alias.scope !73, !noalias !80
  %add77 = add nsw i32 %phi74, %load76
  %add78 = add nsw i32 %add77, 1
  %icmp79 = icmp slt i32 %add78, %phi73
  br i1 %icmp79, label %bb115, label %bb80

bb80:                                             ; preds = %bb71
  %icmp81 = icmp slt i32 %load76, 0
  %call82 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523720), i64 %phi72) #4, !llfort.type_idx !43, !ifx.array_extent !39
  %load83 = load double, ptr %call82, align 1, !tbaa !83, !alias.scope !71, !noalias !85, !llfort.type_idx !47
  %sext84 = sext i32 %phi73 to i64
  %add85 = add nuw nsw i32 %load76, 1
  %add86 = add i32 %phi74, 2
  %add87 = add nsw i32 %add86, %load76
  %sext88 = sext i32 %add85 to i64
  br label %bb89

bb89:                                             ; preds = %bb107, %bb80
  %phi90 = phi i64 [ %sext84, %bb80 ], [ %add111, %bb107 ]
  br i1 %icmp81, label %bb107, label %bb91

bb91:                                             ; preds = %bb89
  br label %bb92

bb92:                                             ; preds = %bb92, %bb91
  %phi93 = phi i64 [ %add95, %bb92 ], [ 0, %bb91 ]
  %phi94 = phi double [ %fadd103, %bb92 ], [ 0.000000e+00, %bb91 ]
  %add95 = add nuw nsw i64 %phi93, 1
  %call96 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1200, ptr nonnull elementtype(double) getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2484000), i64 %add95) #4, !llfort.type_idx !48, !ifx.array_extent !49
  %call97 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %call96, i64 %phi90) #4, !llfort.type_idx !50
  %load98 = load double, ptr %call97, align 1, !tbaa !86, !alias.scope !64, !noalias !88, !llfort.type_idx !54
  %add99 = add nsw i64 %phi93, %sext84
  %call100 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %call70, i64 %add99) #4, !llfort.type_idx !55
  %load101 = load double, ptr %call100, align 1, !tbaa !89, !alias.scope !67, !noalias !91, !llfort.type_idx !59
  %fmul102 = fmul fast double %load101, %load98
  %fadd103 = fadd fast double %fmul102, %phi94
  %icmp104 = icmp eq i64 %add95, %sext88
  br i1 %icmp104, label %bb105, label %bb92

bb105:                                            ; preds = %bb92
  %phi106 = phi double [ %fadd103, %bb92 ]
  br label %bb107

bb107:                                            ; preds = %bb105, %bb89
  %phi108 = phi double [ 0.000000e+00, %bb89 ], [ %phi106, %bb105 ]
  %fmul109 = fmul fast double %load83, %phi108
  %call110 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %call69, i64 %phi90) #4, !llfort.type_idx !60
  store double %fmul109, ptr %call110, align 1, !tbaa !92, !alias.scope !69, !noalias !94
  %add111 = add nsw i64 %phi90, 1
  %trunc112 = trunc i64 %add111 to i32
  %icmp113 = icmp eq i32 %add87, %trunc112
  br i1 %icmp113, label %bb114, label %bb89

bb114:                                            ; preds = %bb107
  br label %bb115

bb115:                                            ; preds = %bb114, %bb71
  %add116 = add nsw i32 %phi73, %load76
  %add117 = add nsw i32 %add116, 1
  %add118 = add nuw nsw i64 %phi72, 1
  %icmp119 = icmp eq i64 %add118, %sext
  br i1 %icmp119, label %bb120, label %bb71

bb120:                                            ; preds = %bb115
  br label %bb121

bb121:                                            ; preds = %bb120, %bb66
  %add122 = add nuw nsw i64 %phi67, 1
  %icmp123 = icmp eq i64 %add122, %zext
  br i1 %icmp123, label %bb124, label %bb66

bb124:                                            ; preds = %bb121
  call void @llvm.experimental.noalias.scope.decl(metadata !95) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !98) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !100) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !102) #4
  call void @llvm.experimental.noalias.scope.decl(metadata !104) #4
  br label %bb125

bb125:                                            ; preds = %bb180, %bb124
  %phi126 = phi i64 [ 1, %bb124 ], [ %add181, %bb180 ]
  br i1 %icmp9, label %bb180, label %bb127

bb127:                                            ; preds = %bb125
  %call128 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1200, ptr nonnull elementtype(double) @global.3, i64 %phi126) #4
  %call129 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1200, ptr nonnull elementtype(double) getelementptr inbounds ([1092056 x i8], ptr @global.2, i64 0, i64 720000), i64 %phi126) #4
  br label %bb130

bb130:                                            ; preds = %bb174, %bb127
  %phi131 = phi i64 [ 1, %bb127 ], [ %add177, %bb174 ]
  %phi132 = phi i32 [ 1, %bb127 ], [ %add176, %bb174 ]
  %phi133 = phi i32 [ 0, %bb127 ], [ %add137, %bb174 ]
  %call134 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr nonnull elementtype(i32) getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523600), i64 %phi131) #4, !llfort.type_idx !38, !ifx.array_extent !39
  %load135 = load i32, ptr %call134, align 1, !tbaa !106, !alias.scope !104, !noalias !111
  %add136 = add nsw i32 %phi133, %load135
  %add137 = add nsw i32 %add136, 1
  %icmp138 = icmp slt i32 %add137, %phi132
  br i1 %icmp138, label %bb174, label %bb139

bb139:                                            ; preds = %bb130
  %icmp140 = icmp slt i32 %load135, 0
  %call141 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2523720), i64 %phi131) #4, !llfort.type_idx !43, !ifx.array_extent !39
  %load142 = load double, ptr %call141, align 1, !tbaa !114, !alias.scope !102, !noalias !116, !llfort.type_idx !47
  %sext143 = sext i32 %phi132 to i64
  %add144 = add nuw nsw i32 %load135, 1
  %add145 = add i32 %phi133, 2
  %add146 = add nsw i32 %add145, %load135
  %sext147 = sext i32 %add144 to i64
  br label %bb148

bb148:                                            ; preds = %bb166, %bb139
  %phi149 = phi i64 [ %sext143, %bb139 ], [ %add170, %bb166 ]
  br i1 %icmp140, label %bb166, label %bb150

bb150:                                            ; preds = %bb148
  br label %bb151

bb151:                                            ; preds = %bb151, %bb150
  %phi152 = phi i64 [ %add154, %bb151 ], [ 0, %bb150 ]
  %phi153 = phi double [ %fadd162, %bb151 ], [ 0.000000e+00, %bb150 ]
  %add154 = add nuw nsw i64 %phi152, 1
  %call155 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 1200, ptr nonnull elementtype(double) getelementptr inbounds ([2523968 x i8], ptr @global, i64 0, i64 2484000), i64 %add154) #4, !llfort.type_idx !48, !ifx.array_extent !49
  %call156 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %call155, i64 %phi149) #4, !llfort.type_idx !50
  %load157 = load double, ptr %call156, align 1, !tbaa !117, !alias.scope !95, !noalias !119, !llfort.type_idx !54
  %add158 = add nsw i64 %phi152, %sext143
  %call159 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %call129, i64 %add158) #4, !llfort.type_idx !55
  %load160 = load double, ptr %call159, align 1, !tbaa !120, !alias.scope !98, !noalias !122, !llfort.type_idx !59
  %fmul161 = fmul fast double %load160, %load157
  %fadd162 = fadd fast double %fmul161, %phi153
  %icmp163 = icmp eq i64 %add154, %sext147
  br i1 %icmp163, label %bb164, label %bb151

bb164:                                            ; preds = %bb151
  %phi165 = phi double [ %fadd162, %bb151 ]
  br label %bb166

bb166:                                            ; preds = %bb164, %bb148
  %phi167 = phi double [ 0.000000e+00, %bb148 ], [ %phi165, %bb164 ]
  %fmul168 = fmul fast double %load142, %phi167
  %call169 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr nonnull elementtype(double) %call128, i64 %phi149) #4, !llfort.type_idx !60
  store double %fmul168, ptr %call169, align 1, !tbaa !123, !alias.scope !100, !noalias !125
  %add170 = add nsw i64 %phi149, 1
  %trunc171 = trunc i64 %add170 to i32
  %icmp172 = icmp eq i32 %add146, %trunc171
  br i1 %icmp172, label %bb173, label %bb148

bb173:                                            ; preds = %bb166
  br label %bb174

bb174:                                            ; preds = %bb173, %bb130
  %add175 = add nsw i32 %phi132, %load135
  %add176 = add nsw i32 %add175, 1
  %add177 = add nuw nsw i64 %phi131, 1
  %icmp178 = icmp eq i64 %add177, %sext
  br i1 %icmp178, label %bb179, label %bb130

bb179:                                            ; preds = %bb174
  br label %bb180

bb180:                                            ; preds = %bb179, %bb125
  %add181 = add nuw nsw i64 %phi126, 1
  %icmp182 = icmp eq i64 %add181, %zext
  br i1 %icmp182, label %bb183, label %bb125

bb183:                                            ; preds = %bb180
  br label %bb184

bb184:                                            ; preds = %bb183, %bb5
  %phi185 = phi i32 [ %load6, %bb5 ], [ %load8, %bb183 ]
  store i32 %phi185, ptr %arg2, align 4
  %icmp186 = icmp slt i32 %phi185, 2
  br i1 %icmp186, label %bb187, label %bb188

bb187:                                            ; preds = %bb184
  ret i1 true

bb188:                                            ; preds = %bb184
  ret i1 false
}

attributes #0 = { nocallback nofree norecurse nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #2 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nounwind uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "pre_loopopt" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!omp_offload.info = !{}
!llvm.module.flags = !{!1, !2, !3}

!0 = !{i64 16}
!1 = !{i32 1, !"ThinLTO", i32 0}
!2 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!3 = !{i32 1, !"LTOPostLink", i32 1}
!4 = !{!5, !5, i64 0}
!5 = !{!"ifx$unique_sym$109", !6, i64 0}
!6 = !{!"Fortran Data Symbol", !7, i64 0}
!7 = !{!"Generic Fortran Symbol", !8, i64 0}
!8 = !{!"ifx$root$1$MAIN__"}
!9 = !{i64 113}
!10 = !{!11}
!11 = distinct !{!11, !12, !"derivx_: %derivx_$D"}
!12 = distinct !{!12, !"derivx_"}
!13 = !{!14}
!14 = distinct !{!14, !12, !"derivx_: %derivx_$U"}
!15 = !{!16}
!16 = distinct !{!16, !12, !"derivx_: %derivx_$UX"}
!17 = !{!18}
!18 = distinct !{!18, !12, !"derivx_: %derivx_$AL"}
!19 = !{!20}
!20 = distinct !{!20, !12, !"derivx_: %derivx_$NP"}
!21 = !{!22}
!22 = distinct !{!22, !12, !"derivx_: %derivx_$ND"}
!23 = !{!24}
!24 = distinct !{!24, !12, !"derivx_: %derivx_$M"}
!25 = !{!26, !26, i64 0}
!26 = !{!"ifx$unique_sym$603$4$12", !27, i64 0}
!27 = !{!"Fortran Data Symbol", !28, i64 0}
!28 = !{!"Generic Fortran Symbol", !29, i64 0}
!29 = !{!"ifx$root$8$derivx_$4$12"}
!30 = !{!11, !14, !16, !18, !20, !22}
!31 = !{i64 6851}
!32 = !{!33, !33, i64 0}
!33 = !{!"ifx$unique_sym$62", !6, i64 0}
!34 = !{!35, !35, i64 0}
!35 = !{!"ifx$unique_sym$607$4$12", !27, i64 0}
!36 = !{!11, !14, !16, !18, !20, !24}
!37 = !{i64 6850}
!38 = !{i64 6861}
!39 = !{i64 30}
!40 = !{!41, !41, i64 0}
!41 = !{!"ifx$unique_sym$609$4$12", !27, i64 0}
!42 = !{!11, !14, !16, !18, !22, !24}
!43 = !{i64 6875}
!44 = !{!45, !45, i64 0}
!45 = !{!"ifx$unique_sym$615$4$12", !27, i64 0}
!46 = !{!11, !14, !16, !20, !22, !24}
!47 = !{i64 6876}
!48 = !{i64 6867}
!49 = !{i64 33}
!50 = !{i64 6868}
!51 = !{!52, !52, i64 0}
!52 = !{!"ifx$unique_sym$613$4$12", !27, i64 0}
!53 = !{!14, !16, !18, !20, !22, !24}
!54 = !{i64 6869}
!55 = !{i64 6872}
!56 = !{!57, !57, i64 0}
!57 = !{!"ifx$unique_sym$614$4$12", !27, i64 0}
!58 = !{!11, !16, !18, !20, !22, !24}
!59 = !{i64 6873}
!60 = !{i64 6879}
!61 = !{!62, !62, i64 0}
!62 = !{!"ifx$unique_sym$616$4$12", !27, i64 0}
!63 = !{!11, !14, !18, !20, !22, !24}
!64 = !{!65}
!65 = distinct !{!65, !66, !"derivx_: %derivx_$D"}
!66 = distinct !{!66, !"derivx_"}
!67 = !{!68}
!68 = distinct !{!68, !66, !"derivx_: %derivx_$U"}
!69 = !{!70}
!70 = distinct !{!70, !66, !"derivx_: %derivx_$UX"}
!71 = !{!72}
!72 = distinct !{!72, !66, !"derivx_: %derivx_$AL"}
!73 = !{!74}
!74 = distinct !{!74, !66, !"derivx_: %derivx_$NP"}
!75 = !{!76, !76, i64 0}
!76 = !{!"ifx$unique_sym$609$4$13", !77, i64 0}
!77 = !{!"Fortran Data Symbol", !78, i64 0}
!78 = !{!"Generic Fortran Symbol", !79, i64 0}
!79 = !{!"ifx$root$8$derivx_$4$13"}
!80 = !{!65, !68, !70, !72, !81, !82}
!81 = distinct !{!81, !66, !"derivx_: %derivx_$ND"}
!82 = distinct !{!82, !66, !"derivx_: %derivx_$M"}
!83 = !{!84, !84, i64 0}
!84 = !{!"ifx$unique_sym$615$4$13", !77, i64 0}
!85 = !{!65, !68, !70, !74, !81, !82}
!86 = !{!87, !87, i64 0}
!87 = !{!"ifx$unique_sym$613$4$13", !77, i64 0}
!88 = !{!68, !70, !72, !74, !81, !82}
!89 = !{!90, !90, i64 0}
!90 = !{!"ifx$unique_sym$614$4$13", !77, i64 0}
!91 = !{!65, !70, !72, !74, !81, !82}
!92 = !{!93, !93, i64 0}
!93 = !{!"ifx$unique_sym$616$4$13", !77, i64 0}
!94 = !{!65, !68, !72, !74, !81, !82}
!95 = !{!96}
!96 = distinct !{!96, !97, !"derivx_: %derivx_$D"}
!97 = distinct !{!97, !"derivx_"}
!98 = !{!99}
!99 = distinct !{!99, !97, !"derivx_: %derivx_$U"}
!100 = !{!101}
!101 = distinct !{!101, !97, !"derivx_: %derivx_$UX"}
!102 = !{!103}
!103 = distinct !{!103, !97, !"derivx_: %derivx_$AL"}
!104 = !{!105}
!105 = distinct !{!105, !97, !"derivx_: %derivx_$NP"}
!106 = !{!107, !107, i64 0}
!107 = !{!"ifx$unique_sym$609$4$14", !108, i64 0}
!108 = !{!"Fortran Data Symbol", !109, i64 0}
!109 = !{!"Generic Fortran Symbol", !110, i64 0}
!110 = !{!"ifx$root$8$derivx_$4$14"}
!111 = !{!96, !99, !101, !103, !112, !113}
!112 = distinct !{!112, !97, !"derivx_: %derivx_$ND"}
!113 = distinct !{!113, !97, !"derivx_: %derivx_$M"}
!114 = !{!115, !115, i64 0}
!115 = !{!"ifx$unique_sym$615$4$14", !108, i64 0}
!116 = !{!96, !99, !101, !105, !112, !113}
!117 = !{!118, !118, i64 0}
!118 = !{!"ifx$unique_sym$613$4$14", !108, i64 0}
!119 = !{!99, !101, !103, !105, !112, !113}
!120 = !{!121, !121, i64 0}
!121 = !{!"ifx$unique_sym$614$4$14", !108, i64 0}
!122 = !{!96, !101, !103, !105, !112, !113}
!123 = !{!124, !124, i64 0}
!124 = !{!"ifx$unique_sym$616$4$14", !108, i64 0}
!125 = !{!96, !99, !103, !105, !112, !113}
