; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -S -passes='tilemvinlmarker' -tile-candidate-mark -tile-candidate-test -tile-candidate-min=4 -tile-candidate-arg-min=3 -tile-candidate-sub-arg-min=2 2>&1 | FileCheck %s

; Check that the !dbg metadata on the generated loads in the global
; multiversioning test matches the !dbg metadata for the call on which
; the multiversioning was performed.

; CHECK: define dso_local void @MAIN__()
; CHECK: call fastcc void @leapfrog_({{.*}}), !dbg ![[D1:[0-9]+]]
; CHECK: load {{.*}} @mymod_mp_mytester_, {{.*}} !dbg ![[D1]]
; CHECK: load {{.*}} @mymod_mp_myglobal_, {{.*}} !dbg ![[D1]]
; CHECK: load {{.*}} @mymod_mp_mybool_, {{.*}} !dbg ![[D1]]
; CHECK: load {{.*}} @mymod_mp_mynnodes_, {{.*}} !dbg ![[D1]]
; CHECK: call fastcc void @leapfrog_.1({{.*}}), !dbg ![[D1]]
; CHECK: ![[D2:[0-9]+]] = distinct !DISubprogram{{.*}} linkageName: "MAIN__"
; CHECK: ![[D1]] = !DILocation({{.*}}, scope: ![[D2]])
@"main_$A" = internal global [100 x double] zeroinitializer, align 16
@"main_$B" = internal global [100 x double] zeroinitializer, align 16
@anon.0 = internal unnamed_addr constant [0 x i8] zeroinitializer
@anon.1 = internal unnamed_addr constant i32 2
@anon.2 = internal unnamed_addr constant i32 100
@mymod_mp_myglobal_ = internal global i32 0, align 8
@mymod_mp_mynnodes_ = internal global i32 0, align 8
@mymod_mp_mytester_ = internal global i32 0, align 8
@mymod_mp_mybool_ = internal global i1 false, align 8

declare dso_local i32 @for_stop_core_quiet(ptr, i32, i32, i64, i32, i32, ...) local_unnamed_addr

declare dso_local i32 @for_set_reentrancy(ptr) local_unnamed_addr

declare dso_local i32 @for_read_seq_lis(ptr, i32, i64, ptr, ptr, ...) local_unnamed_addr

define internal void @extra_(ptr noalias nocapture %arg, ptr noalias nocapture readonly %arg1) !dbg !6 {
bb:
  %i = load i32, ptr %arg1, align 4
  %i2 = icmp slt i32 %i, 1
  br i1 %i2, label %bb11, label %bb3

bb3:                                              ; preds = %bb
  %i4 = add nuw i32 %i, 1
  %i5 = zext i32 %i4 to i64
  br label %bb6

bb6:                                              ; preds = %bb6, %bb3
  %i7 = phi i64 [ 1, %bb3 ], [ %i9, %bb6 ]
  %i8 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i7), !dbg !9
  store double 5.000000e+00, ptr %i8, align 8
  %i9 = add nuw nsw i64 %i7, 1
  %i10 = icmp eq i64 %i9, %i5
  br i1 %i10, label %bb11, label %bb6

bb11:                                             ; preds = %bb6, %bb
  ret void
}

define internal fastcc void @init_() unnamed_addr !dbg !10 {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca [4 x i8], align 1
  %i2 = alloca { ptr }, align 8
  %i3 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 0
  store i8 9, ptr %i3, align 1
  %i4 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 1
  store i8 3, ptr %i4, align 1
  %i5 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 2
  store i8 1, ptr %i5, align 1
  %i6 = getelementptr inbounds [4 x i8], ptr %i1, i64 0, i64 3
  store i8 0, ptr %i6, align 1
  %i7 = getelementptr inbounds { ptr }, ptr %i2, i64 0, i32 0
  store ptr @mymod_mp_mynnodes_, ptr %i7, align 8
  %i10 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_read_seq_lis(ptr nonnull %i, i32 5, i64 1239157112576, ptr nonnull %i3, ptr nonnull %i2), !dbg !11
  ret void
}

define dso_local void @MAIN__() local_unnamed_addr !dbg !12 {
bb:
  %i = alloca [8 x i64], align 16
  %i1 = alloca i32, align 8
  %i2 = alloca [4 x i8], align 1
  %i3 = alloca { ptr }, align 8
  %i4 = alloca [4 x i8], align 1
  %i5 = alloca { ptr }, align 8
  %i6 = tail call i32 @for_set_reentrancy(ptr nonnull @anon.1), !dbg !13
  %i7 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 0
  store i8 16, ptr %i7, align 1
  %i8 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 1
  store i8 3, ptr %i8, align 1
  %i9 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 2
  store i8 1, ptr %i9, align 1
  %i10 = getelementptr inbounds [4 x i8], ptr %i2, i64 0, i64 3
  store i8 0, ptr %i10, align 1
  store ptr %i1, ptr %i3, align 8
  %i14 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_read_seq_lis(ptr nonnull %i, i32 5, i64 1239157112576, ptr nonnull %i7, ptr nonnull %i3), !dbg !14
  %i15 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 0
  store i8 9, ptr %i15, align 1
  %i16 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 1
  store i8 3, ptr %i16, align 1
  %i17 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 2
  store i8 1, ptr %i17, align 1
  %i18 = getelementptr inbounds [4 x i8], ptr %i4, i64 0, i64 3
  store i8 0, ptr %i18, align 1
  %i19 = getelementptr inbounds { ptr }, ptr %i5, i64 0, i32 0
  store ptr @mymod_mp_myglobal_, ptr %i19, align 8
  %i21 = call i32 (ptr, i32, i64, ptr, ptr, ...) @for_read_seq_lis(ptr nonnull %i, i32 5, i64 1239157112576, ptr nonnull %i15, ptr nonnull %i5), !dbg !15
  call fastcc void @init_(), !dbg !16
  call fastcc void @leapfrog_(ptr getelementptr inbounds ([100 x double], ptr @"main_$A", i64 0, i64 0), ptr getelementptr inbounds ([100 x double], ptr @"main_$B", i64 0, i64 0), ptr nonnull @anon.2, ptr nonnull %i1), !dbg !17
  ret void
}

define internal fastcc void @leapfrog_(ptr noalias nocapture %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture readonly %arg3) unnamed_addr !dbg !18 {
bb:
  %t5 = load i32, ptr @mymod_mp_mytester_, align 8, !dbg !19
  %t6 = icmp sgt i32 %t5, 5
  br i1 %t6, label %L70, label %L71

L70:                                              ; preds = %bb
  tail call fastcc void @fun0_(ptr %arg, ptr %arg1, ptr %arg2), !dbg !20
  br label %L71

L71:                                              ; preds = %L70, %bb
  %t51 = load i32, ptr @mymod_mp_myglobal_, align 8, !dbg !21
  %t61 = icmp eq i32 %t51, 1
  br i1 %t61, label %L72, label %L73

L72:                                              ; preds = %L71
  tail call fastcc void @fun1_(ptr %arg, ptr %arg1, ptr %arg2), !dbg !22
  br label %L73

L73:                                              ; preds = %L72, %L71
  %t52 = load i32, ptr @mymod_mp_myglobal_, align 8, !dbg !23
  %t62 = icmp sge i32 %t52, 1
  br i1 %t62, label %L74, label %L75

L74:                                              ; preds = %L73
  tail call fastcc void @fun2_(ptr %arg, ptr %arg1, ptr %arg2), !dbg !24
  br label %L75

L75:                                              ; preds = %L74, %L73
  %t53 = load i1, ptr @mymod_mp_mybool_, align 8, !dbg !25
  br i1 %t53, label %L76, label %L77

L76:                                              ; preds = %L75
  tail call fastcc void @extra_(ptr %arg, ptr %arg2), !dbg !26
  br label %L77

L77:                                              ; preds = %L76, %L75
  tail call fastcc void @switch_(ptr %arg, ptr %arg1, ptr %arg2, ptr %arg3), !dbg !27
  br label %L8

L8:                                               ; preds = %L77
  br label %L9

L9:                                               ; preds = %L8
  %t7 = load i32, ptr @mymod_mp_mynnodes_, align 8, !dbg !28
  %t8 = icmp eq i32 %t7, -2
  br i1 %t8, label %L12, label %L14

L12:                                              ; preds = %L9
  %t9 = tail call i32 (ptr, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(ptr getelementptr inbounds ([0 x i8], ptr @anon.0, i64 0, i64 0), i32 0, i32 0, i64 1239157112576, i32 0, i32 0), !dbg !29
  br label %L14

L14:                                              ; preds = %L12, %L9
  ret void
}

define internal fastcc void @switch_(ptr noalias nocapture %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2, ptr noalias nocapture readonly %arg3) unnamed_addr !dbg !30 {
bb:
  %i = load i32, ptr %arg3, align 4
  %i4 = and i32 %i, 1
  %i5 = icmp eq i32 %i4, 0
  br i1 %i5, label %bb7, label %bb6

bb6:                                              ; preds = %bb
  tail call fastcc void @fun00_(ptr %arg, ptr %arg1, ptr %arg2), !dbg !31
  br label %bb8

bb7:                                              ; preds = %bb
  tail call fastcc void @fun01_(ptr %arg, ptr %arg1, ptr %arg2), !dbg !32
  br label %bb8

bb8:                                              ; preds = %bb7, %bb6
  ret void
}

define internal fastcc void @fun01_(ptr noalias nocapture readonly %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr !dbg !33 {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 5
  br i1 %i3, label %bb21, label %bb4

bb4:                                              ; preds = %bb
  %i5 = add nsw i32 %i, -1
  %i6 = zext i32 %i5 to i64
  br label %bb7

bb7:                                              ; preds = %bb7, %bb4
  %i8 = phi i64 [ 3, %bb4 ], [ %i12, %bb7 ]
  %i9 = add nsw i64 %i8, -1
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i9), !dbg !34
  %i11 = load double, ptr %i10, align 8
  %i12 = add nuw nsw i64 %i8, 1
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i12), !dbg !35
  %i14 = load double, ptr %i13, align 8
  %i15 = fadd double %i11, %i14
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i8), !dbg !36
  %i17 = load double, ptr %i16, align 8
  %i18 = fadd double %i15, %i17
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i8), !dbg !37
  store double %i18, ptr %i19, align 8
  %i20 = icmp eq i64 %i12, %i6
  br i1 %i20, label %bb21, label %bb7

bb21:                                             ; preds = %bb7, %bb
  ret void
}

define internal fastcc void @fun00_(ptr noalias nocapture %arg, ptr noalias nocapture readonly %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr !dbg !38 {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 5
  br i1 %i3, label %bb21, label %bb4

bb4:                                              ; preds = %bb
  %i5 = add nsw i32 %i, -1
  %i6 = zext i32 %i5 to i64
  br label %bb7

bb7:                                              ; preds = %bb7, %bb4
  %i8 = phi i64 [ 3, %bb4 ], [ %i12, %bb7 ]
  %i9 = add nsw i64 %i8, -1
  %i10 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i9), !dbg !39
  %i11 = load double, ptr %i10, align 8
  %i12 = add nuw nsw i64 %i8, 1
  %i13 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i12), !dbg !40
  %i14 = load double, ptr %i13, align 8
  %i15 = fadd double %i11, %i14
  %i16 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i8), !dbg !41
  %i17 = load double, ptr %i16, align 8
  %i18 = fadd double %i15, %i17
  %i19 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i8), !dbg !42
  store double %i18, ptr %i19, align 8
  %i20 = icmp eq i64 %i12, %i6
  br i1 %i20, label %bb21, label %bb7

bb21:                                             ; preds = %bb7, %bb
  ret void
}

define internal fastcc void @fun1_(ptr noalias nocapture readonly %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr !dbg !43 {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 3
  br i1 %i3, label %bb20, label %bb4

bb4:                                              ; preds = %bb
  %i5 = zext i32 %i to i64
  br label %bb6

bb6:                                              ; preds = %bb6, %bb4
  %i7 = phi i64 [ 2, %bb4 ], [ %i11, %bb6 ]
  %i8 = add nsw i64 %i7, -1
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i8), !dbg !44
  %i10 = load double, ptr %i9, align 8
  %i11 = add nuw nsw i64 %i7, 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i11), !dbg !45
  %i13 = load double, ptr %i12, align 8
  %i14 = fadd double %i10, %i13
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i7), !dbg !46
  %i16 = load double, ptr %i15, align 8
  %i17 = fadd double %i14, %i16
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i7), !dbg !47
  store double %i17, ptr %i18, align 8
  %i19 = icmp eq i64 %i11, %i5
  br i1 %i19, label %bb20, label %bb6

bb20:                                             ; preds = %bb6, %bb
  ret void
}

define internal fastcc void @fun2_(ptr noalias nocapture readonly %arg, ptr noalias nocapture %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr !dbg !48 {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 3
  br i1 %i3, label %bb20, label %bb4

bb4:                                              ; preds = %bb
  %i5 = zext i32 %i to i64
  br label %bb6

bb6:                                              ; preds = %bb6, %bb4
  %i7 = phi i64 [ 2, %bb4 ], [ %i11, %bb6 ]
  %i8 = add nsw i64 %i7, -1
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i8), !dbg !49
  %i10 = load double, ptr %i9, align 8
  %i11 = add nuw nsw i64 %i7, 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i11), !dbg !50
  %i13 = load double, ptr %i12, align 8
  %i14 = fadd double %i10, %i13
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i7), !dbg !51
  %i16 = load double, ptr %i15, align 8
  %i17 = fadd double %i14, %i16
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i7), !dbg !52
  store double %i17, ptr %i18, align 8
  %i19 = icmp eq i64 %i11, %i5
  br i1 %i19, label %bb20, label %bb6

bb20:                                             ; preds = %bb6, %bb
  ret void
}

define internal fastcc void @fun0_(ptr noalias nocapture %arg, ptr noalias nocapture readonly %arg1, ptr noalias nocapture readonly %arg2) unnamed_addr !dbg !53 {
bb:
  %i = load i32, ptr %arg2, align 4
  %i3 = icmp slt i32 %i, 3
  br i1 %i3, label %bb20, label %bb4

bb4:                                              ; preds = %bb
  %i5 = zext i32 %i to i64
  br label %bb6

bb6:                                              ; preds = %bb6, %bb4
  %i7 = phi i64 [ 2, %bb4 ], [ %i11, %bb6 ]
  %i8 = add nsw i64 %i7, -1
  %i9 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i8), !dbg !54
  %i10 = load double, ptr %i9, align 8
  %i11 = add nuw nsw i64 %i7, 1
  %i12 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i11), !dbg !55
  %i13 = load double, ptr %i12, align 8
  %i14 = fadd double %i10, %i13
  %i15 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg1, i64 %i7), !dbg !56
  %i16 = load double, ptr %i15, align 8
  %i17 = fadd double %i14, %i16
  %i18 = tail call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 8, ptr elementtype(double) %arg, i64 %i7), !dbg !57
  store double %i17, ptr %i18, align 8
  %i19 = icmp eq i64 %i11, %i5
  br i1 %i19, label %bb20, label %bb6

bb20:                                             ; preds = %bb6, %bb
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0

attributes #0 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}

!0 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !1, producer: "Intel(R) Fortran 21.0-2184", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "mine.f90", directory: "mydir")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = distinct !DISubprogram(name: "extra", linkageName: "extra_", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!7 = !DISubroutineType(types: !8)
!8 = !{null}
!9 = !DILocation(line: 20, column: 1, scope: !6)
!10 = distinct !DISubprogram(name: "init", linkageName: "init_", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!11 = !DILocation(line: 21, column: 1, scope: !10)
!12 = distinct !DISubprogram(name: "pscyee_mpi", linkageName: "MAIN__", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!13 = !DILocation(line: 22, column: 1, scope: !12)
!14 = !DILocation(line: 23, column: 1, scope: !12)
!15 = !DILocation(line: 24, column: 1, scope: !12)
!16 = !DILocation(line: 25, column: 1, scope: !12)
!17 = !DILocation(line: 26, column: 1, scope: !12)
!18 = distinct !DISubprogram(name: "leapfrog", linkageName: "leapfrog_", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!19 = !DILocation(line: 55, column: 1, scope: !18)
!20 = !DILocation(line: 27, column: 1, scope: !18)
!21 = !DILocation(line: 56, column: 1, scope: !18)
!22 = !DILocation(line: 28, column: 1, scope: !18)
!23 = !DILocation(line: 57, column: 1, scope: !18)
!24 = !DILocation(line: 29, column: 1, scope: !18)
!25 = !DILocation(line: 58, column: 1, scope: !18)
!26 = !DILocation(line: 30, column: 1, scope: !18)
!27 = !DILocation(line: 31, column: 1, scope: !18)
!28 = !DILocation(line: 59, column: 1, scope: !18)
!29 = !DILocation(line: 32, column: 1, scope: !18)
!30 = distinct !DISubprogram(name: "switch", linkageName: "switch_", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!31 = !DILocation(line: 33, column: 1, scope: !30)
!32 = !DILocation(line: 34, column: 1, scope: !30)
!33 = distinct !DISubprogram(name: "fun01", linkageName: "fun01_", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!34 = !DILocation(line: 35, column: 1, scope: !33)
!35 = !DILocation(line: 36, column: 1, scope: !33)
!36 = !DILocation(line: 37, column: 1, scope: !33)
!37 = !DILocation(line: 38, column: 1, scope: !33)
!38 = distinct !DISubprogram(name: "fun00", linkageName: "fun00_", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!39 = !DILocation(line: 39, column: 1, scope: !38)
!40 = !DILocation(line: 40, column: 1, scope: !38)
!41 = !DILocation(line: 41, column: 1, scope: !38)
!42 = !DILocation(line: 42, column: 1, scope: !38)
!43 = distinct !DISubprogram(name: "fun1", linkageName: "fun1_", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!44 = !DILocation(line: 43, column: 1, scope: !43)
!45 = !DILocation(line: 44, column: 1, scope: !43)
!46 = !DILocation(line: 45, column: 1, scope: !43)
!47 = !DILocation(line: 46, column: 1, scope: !43)
!48 = distinct !DISubprogram(name: "fun2", linkageName: "fun2_", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!49 = !DILocation(line: 47, column: 1, scope: !48)
!50 = !DILocation(line: 48, column: 1, scope: !48)
!51 = !DILocation(line: 49, column: 1, scope: !48)
!52 = !DILocation(line: 50, column: 1, scope: !48)
!53 = distinct !DISubprogram(name: "fun0", linkageName: "fun0_", scope: !1, file: !1, line: 81, type: !7, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!54 = !DILocation(line: 51, column: 1, scope: !53)
!55 = !DILocation(line: 52, column: 1, scope: !53)
!56 = !DILocation(line: 53, column: 1, scope: !53)
!57 = !DILocation(line: 54, column: 1, scope: !53)
; end INTEL_FEATURE_SW_ADVANCED
