; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -S -passes='tilemvinlmarker' -tile-candidate-mark -tile-candidate-test -tile-candidate-min=4 -tile-candidate-arg-min=3 -tile-candidate-sub-arg-min=2 2>&1 | FileCheck %s

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

declare dso_local i32 @for_stop_core_quiet(i8*, i32, i32, i64, i32, i32, ...) local_unnamed_addr

declare double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8, i64, i64, double*, i64) #1

declare dso_local i32 @for_set_reentrancy(i32*) local_unnamed_addr

declare dso_local i32 @for_read_seq_lis(i8*, i32, i64, i8*, i8*, ...) local_unnamed_addr

@mymod_mp_myglobal_ = internal global i32 0, align 8
@mymod_mp_mynnodes_ = internal global i32 0, align 8
@mymod_mp_mytester_ = internal global i32 0, align 8
@mymod_mp_mybool_ = internal global i1 0, align 8

define internal void @extra_(double* noalias nocapture %0, i32* noalias nocapture readonly %1) #0 !dbg !10 {
  %3 = load i32, i32* %1, align 4
  %4 = icmp slt i32 %3, 1
  br i1 %4, label %13, label %5

5:                                                ; preds = %2
  %6 = add nuw i32 %3, 1
  %7 = zext i32 %6 to i64
  br label %8

8:                                                ; preds = %8, %5
  %9 = phi i64 [ 1, %5 ], [ %11, %8 ]
  %10 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %9), !dbg !20
  store double 5.000000e+00, double* %10, align 8
  %11 = add nuw nsw i64 %9, 1
  %12 = icmp eq i64 %11, %7
  br i1 %12, label %13, label %8

13:                                               ; preds = %8, %2
  ret void
}

; Function Attrs: noinline
define internal fastcc void @init_() unnamed_addr #1 !dbg !11 {
  %1 = alloca [8 x i64], align 16
  %2 = alloca [4 x i8], align 1
  %3 = alloca { i8* }, align 8
  %4 = getelementptr inbounds [4 x i8], [4 x i8]* %2, i64 0, i64 0
  store i8 9, i8* %4, align 1
  %5 = getelementptr inbounds [4 x i8], [4 x i8]* %2, i64 0, i64 1
  store i8 3, i8* %5, align 1
  %6 = getelementptr inbounds [4 x i8], [4 x i8]* %2, i64 0, i64 2
  store i8 1, i8* %6, align 1
  %7 = getelementptr inbounds [4 x i8], [4 x i8]* %2, i64 0, i64 3
  store i8 0, i8* %7, align 1
  %8 = getelementptr inbounds { i8* }, { i8* }* %3, i64 0, i32 0
  store i8* bitcast (i32* @mymod_mp_mynnodes_ to i8*), i8** %8, align 8
  %9 = bitcast [8 x i64]* %1 to i8*
  %10 = bitcast { i8* }* %3 to i8*
  %11 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_read_seq_lis(i8* nonnull %9, i32 5, i64 1239157112576, i8* nonnull %4, i8* nonnull %10), !dbg !21
  ret void
}

define dso_local void @MAIN__() local_unnamed_addr #2 !dbg !12 {
  %1 = alloca [8 x i64], align 16
  %2 = alloca i32, align 8
  %3 = alloca [4 x i8], align 1
  %4 = alloca { i8* }, align 8
  %5 = alloca [4 x i8], align 1
  %6 = alloca { i8* }, align 8
  %7 = tail call i32 @for_set_reentrancy(i32* nonnull @anon.1), !dbg !22
  %8 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 0
  store i8 16, i8* %8, align 1
  %9 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 1
  store i8 3, i8* %9, align 1
  %10 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 2
  store i8 1, i8* %10, align 1
  %11 = getelementptr inbounds [4 x i8], [4 x i8]* %3, i64 0, i64 3
  store i8 0, i8* %11, align 1
  %12 = bitcast { i8* }* %4 to i32**
  store i32* %2, i32** %12, align 8
  %13 = bitcast [8 x i64]* %1 to i8*
  %14 = bitcast { i8* }* %4 to i8*
  %15 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_read_seq_lis(i8* nonnull %13, i32 5, i64 1239157112576, i8* nonnull %8, i8* nonnull %14), !dbg !23
  %16 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i64 0, i64 0
  store i8 9, i8* %16, align 1
  %17 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i64 0, i64 1
  store i8 3, i8* %17, align 1
  %18 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i64 0, i64 2
  store i8 1, i8* %18, align 1
  %19 = getelementptr inbounds [4 x i8], [4 x i8]* %5, i64 0, i64 3
  store i8 0, i8* %19, align 1
  %20 = getelementptr inbounds { i8* }, { i8* }* %6, i64 0, i32 0
  store i8* bitcast (i32* @mymod_mp_myglobal_ to i8*), i8** %20, align 8
  %21 = bitcast { i8* }* %6 to i8*
  %22 = call i32 (i8*, i32, i64, i8*, i8*, ...) @for_read_seq_lis(i8* nonnull %13, i32 5, i64 1239157112576, i8* nonnull %16, i8* nonnull %21), !dbg !24
  call fastcc void @init_() #3, !dbg !25
  call fastcc void @leapfrog_(double* getelementptr inbounds ([100 x double], [100 x double]* @"main_$A", i64 0, i64 0), double* getelementptr inbounds ([100 x double], [100 x double]* @"main_$B", i64 0, i64 0), i32* nonnull @anon.2, i32* nonnull %2) #3, !dbg !26
  ret void
}

define internal fastcc void @leapfrog_(double* noalias nocapture %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2, i32* noalias nocapture readonly %3) unnamed_addr #1 !dbg !13 {
  %t5 = load i32, i32* @mymod_mp_mytester_, align 8, !dbg !55
  %t6 = icmp sgt i32 %t5, 5
  br i1 %t6, label %L70, label %L71
L70:                                               ; preds = %4
  tail call fastcc void @fun0_(double* %0, double* %1, i32* %2) #3, !dbg !27
  br label %L71
L71:                                               ; preds = %4, %L70
  %t51 = load i32, i32* @mymod_mp_myglobal_, align 8, !dbg !56
  %t61 = icmp eq i32 %t51, 1
  br i1 %t61, label %L72, label %L73
L72:                                               ; preds = %L71
  tail call fastcc void @fun1_(double* %0, double* %1, i32* %2), !dbg !28
  br label %L73
L73:                                               ; preds = %L71, %l72
  %t52 = load i32, i32* @mymod_mp_myglobal_, align 8, !dbg !57
  %t62 = icmp sge i32 %t52, 1
  br i1 %t62, label %L74, label %L75
L74:                                               ; preds = %L73
  tail call fastcc void @fun2_(double* %0, double* %1, i32* %2), !dbg !29
  br label %L75
L75:                                               ; preds = %L73, %L74
  %t53 = load i1, i1* @mymod_mp_mybool_, align 8, !dbg !58
  br i1 %t53, label %L76, label %L77
L76:                                               ; preds = %L75
  tail call fastcc void @extra_(double* %0, i32* %2), !dbg !30
  br label %L77
L77:                                               ; preds = %l75, %L76
  tail call fastcc void @switch_(double* %0, double* %1, i32* %2, i32* %3), !dbg !31
  br label %L8
L8:                                                ; preds = %L77
  br label %L9
L9:                                                ; preds = %L8
  %t7 = load i32, i32* @mymod_mp_mynnodes_, align 8, !dbg !59
  %t8 = icmp eq i32 %t7, -2
  br i1 %t8, label %L12, label %L14
L12:                                               ; preds = %L9
  %t9 = tail call i32 (i8*, i32, i32, i64, i32, i32, ...) @for_stop_core_quiet(i8* getelementptr inbounds ([0 x i8], [0 x i8]* @anon.0, i64 0, i64 0), i32 0, i32 0, i64 1239157112576, i32 0, i32 0), !dbg !32
  br label %L14
L14:                                               ; preds = %L12, %L9
  ret void
}

define internal fastcc void @switch_(double* noalias nocapture %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2, i32* noalias nocapture readonly %3) unnamed_addr #0 !dbg !14 {
  %5 = load i32, i32* %3, align 4
  %6 = and i32 %5, 1
  %7 = icmp eq i32 %6, 0
  br i1 %7, label %9, label %8

8:                                                ; preds = %4
  tail call fastcc void @fun00_(double* %0, double* %1, i32* %2), !dbg !33
  br label %10

9:                                                ; preds = %4
  tail call fastcc void @fun01_(double* %0, double* %1, i32* %2), !dbg !34
  br label %10

10:                                               ; preds = %9, %8
  ret void
}

define internal fastcc void @fun01_(double* noalias nocapture readonly %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2) unnamed_addr #0 !dbg !15 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 5
  br i1 %5, label %23, label %6

6:                                                ; preds = %3
  %7 = add nsw i32 %4, -1
  %8 = zext i32 %7 to i64
  br label %9

9:                                                ; preds = %9, %6
  %10 = phi i64 [ 3, %6 ], [ %14, %9 ]
  %11 = add nsw i64 %10, -1
  %12 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %11), !dbg !35
  %13 = load double, double* %12, align 8
  %14 = add nuw nsw i64 %10, 1
  %15 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %14), !dbg !36
  %16 = load double, double* %15, align 8
  %17 = fadd double %13, %16
  %18 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %10), !dbg !37
  %19 = load double, double* %18, align 8
  %20 = fadd double %17, %19
  %21 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %10), !dbg !38
  store double %20, double* %21, align 8
  %22 = icmp eq i64 %14, %8
  br i1 %22, label %23, label %9

23:                                               ; preds = %9, %3
  ret void
}

define internal fastcc void @fun00_(double* noalias nocapture %0, double* noalias nocapture readonly %1, i32* noalias nocapture readonly %2) unnamed_addr #0 !dbg !16 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 5
  br i1 %5, label %23, label %6

6:                                                ; preds = %3
  %7 = add nsw i32 %4, -1
  %8 = zext i32 %7 to i64
  br label %9

9:                                                ; preds = %9, %6
  %10 = phi i64 [ 3, %6 ], [ %14, %9 ]
  %11 = add nsw i64 %10, -1
  %12 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %11), !dbg !39
  %13 = load double, double* %12, align 8
  %14 = add nuw nsw i64 %10, 1
  %15 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %14), !dbg !40
  %16 = load double, double* %15, align 8
  %17 = fadd double %13, %16
  %18 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %10), !dbg !41
  %19 = load double, double* %18, align 8
  %20 = fadd double %17, %19
  %21 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %10), !dbg !42
  store double %20, double* %21, align 8
  %22 = icmp eq i64 %14, %8
  br i1 %22, label %23, label %9

23:                                               ; preds = %9, %3
  ret void
}

define internal fastcc void @fun1_(double* noalias nocapture readonly %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2) unnamed_addr #0 !dbg !17 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 3
  br i1 %5, label %22, label %6

6:                                                ; preds = %3
  %7 = zext i32 %4 to i64
  br label %8

8:                                                ; preds = %8, %6
  %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
  %10 = add nsw i64 %9, -1
  %11 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %10), !dbg !43
  %12 = load double, double* %11, align 8
  %13 = add nuw nsw i64 %9, 1
  %14 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %13), !dbg !44
  %15 = load double, double* %14, align 8
  %16 = fadd double %12, %15
  %17 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %9), !dbg !45
  %18 = load double, double* %17, align 8
  %19 = fadd double %16, %18
  %20 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %9), !dbg !46
  store double %19, double* %20, align 8
  %21 = icmp eq i64 %13, %7
  br i1 %21, label %22, label %8

22:                                               ; preds = %8, %3
  ret void
}

define internal fastcc void @fun2_(double* noalias nocapture readonly %0, double* noalias nocapture %1, i32* noalias nocapture readonly %2) unnamed_addr #0 !dbg !18 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 3
  br i1 %5, label %22, label %6

6:                                                ; preds = %3
  %7 = zext i32 %4 to i64
  br label %8

8:                                                ; preds = %8, %6
  %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
  %10 = add nsw i64 %9, -1
  %11 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %10), !dbg !47
  %12 = load double, double* %11, align 8
  %13 = add nuw nsw i64 %9, 1
  %14 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %13), !dbg !48
  %15 = load double, double* %14, align 8
  %16 = fadd double %12, %15
  %17 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %9), !dbg !49
  %18 = load double, double* %17, align 8
  %19 = fadd double %16, %18
  %20 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %9), !dbg !50
  store double %19, double* %20, align 8
  %21 = icmp eq i64 %13, %7
  br i1 %21, label %22, label %8

22:                                               ; preds = %8, %3
  ret void
}

define internal fastcc void @fun0_(double* noalias nocapture %0, double* noalias nocapture readonly %1, i32* noalias nocapture readonly %2) unnamed_addr #0 !dbg !19 {
  %4 = load i32, i32* %2, align 4
  %5 = icmp slt i32 %4, 3
  br i1 %5, label %22, label %6

6:                                                ; preds = %3
  %7 = zext i32 %4 to i64
  br label %8

8:                                                ; preds = %8, %6
  %9 = phi i64 [ 2, %6 ], [ %13, %8 ]
  %10 = add nsw i64 %9, -1
  %11 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %10), !dbg !51
  %12 = load double, double* %11, align 8
  %13 = add nuw nsw i64 %9, 1
  %14 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %13), !dbg !52
  %15 = load double, double* %14, align 8
  %16 = fadd double %12, %15
  %17 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %1, i64 %9), !dbg !53
  %18 = load double, double* %17, align 8
  %19 = fadd double %16, %18
  %20 = tail call double* @llvm.intel.subscript.p0f64.i64.i64.p0f64.i64(i8 0, i64 1, i64 8, double* elementtype(double) %0, i64 %9), !dbg !54
  store double %19, double* %20, align 8
  %21 = icmp eq i64 %13, %7
  br i1 %21, label %22, label %8

22:                                               ; preds = %8, %3
  ret void
}

!llvm.dbg.cu = !{!6}
!llvm.module.flags = !{!2, !3, !4}
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{}
!6 = distinct !DICompileUnit(language: DW_LANG_Fortran95, file: !7, producer: "Intel(R) Fortran 21.0-2184", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !5, globals: !5, splitDebugInlining: false, nameTableKind: None)
!7 = !DIFile(filename: "mine.f90", directory: "mydir")
!8 = !{null}
!9 = !DISubroutineType(types: !8)
!10 = distinct !DISubprogram(name: "extra", linkageName: "extra_", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!11 = distinct !DISubprogram(name: "init", linkageName: "init_", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!12 = distinct !DISubprogram(name: "pscyee_mpi", linkageName: "MAIN__", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!13 = distinct !DISubprogram(name: "leapfrog", linkageName: "leapfrog_", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!14 = distinct !DISubprogram(name: "switch", linkageName: "switch_", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!15 = distinct !DISubprogram(name: "fun01", linkageName: "fun01_", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!16 = distinct !DISubprogram(name: "fun00", linkageName: "fun00_", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!17 = distinct !DISubprogram(name: "fun1", linkageName: "fun1_", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!18 = distinct !DISubprogram(name: "fun2", linkageName: "fun2_", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!19 = distinct !DISubprogram(name: "fun0", linkageName: "fun0_", scope: !7, file: !7, line: 81, type: !9, scopeLine: 81, spFlags: DISPFlagDefinition, unit: !6, retainedNodes: !5)
!20 = !DILocation(line: 20, column: 1, scope: !10)
!21 = !DILocation(line: 21, column: 1, scope: !11)
!22 = !DILocation(line: 22, column: 1, scope: !12)
!23 = !DILocation(line: 23, column: 1, scope: !12)
!24 = !DILocation(line: 24, column: 1, scope: !12)
!25 = !DILocation(line: 25, column: 1, scope: !12)
!26 = !DILocation(line: 26, column: 1, scope: !12)
!27 = !DILocation(line: 27, column: 1, scope: !13)
!28 = !DILocation(line: 28, column: 1, scope: !13)
!29 = !DILocation(line: 29, column: 1, scope: !13)
!30 = !DILocation(line: 30, column: 1, scope: !13)
!31 = !DILocation(line: 31, column: 1, scope: !13)
!32 = !DILocation(line: 32, column: 1, scope: !13)
!33 = !DILocation(line: 33, column: 1, scope: !14)
!34 = !DILocation(line: 34, column: 1, scope: !14)
!35 = !DILocation(line: 35, column: 1, scope: !15)
!36 = !DILocation(line: 36, column: 1, scope: !15)
!37 = !DILocation(line: 37, column: 1, scope: !15)
!38 = !DILocation(line: 38, column: 1, scope: !15)
!39 = !DILocation(line: 39, column: 1, scope: !16)
!40 = !DILocation(line: 40, column: 1, scope: !16)
!41 = !DILocation(line: 41, column: 1, scope: !16)
!42 = !DILocation(line: 42, column: 1, scope: !16)
!43 = !DILocation(line: 43, column: 1, scope: !17)
!44 = !DILocation(line: 44, column: 1, scope: !17)
!45 = !DILocation(line: 45, column: 1, scope: !17)
!46 = !DILocation(line: 46, column: 1, scope: !17)
!47 = !DILocation(line: 47, column: 1, scope: !18)
!48 = !DILocation(line: 48, column: 1, scope: !18)
!49 = !DILocation(line: 49, column: 1, scope: !18)
!50 = !DILocation(line: 50, column: 1, scope: !18)
!51 = !DILocation(line: 51, column: 1, scope: !19)
!52 = !DILocation(line: 52, column: 1, scope: !19)
!53 = !DILocation(line: 53, column: 1, scope: !19)
!54 = !DILocation(line: 54, column: 1, scope: !19)
!55 = !DILocation(line: 55, column: 1, scope: !13)
!56 = !DILocation(line: 56, column: 1, scope: !13)
!57 = !DILocation(line: 57, column: 1, scope: !13)
!58 = !DILocation(line: 58, column: 1, scope: !13)
!59 = !DILocation(line: 59, column: 1, scope: !13)
; end INTEL_FEATURE_SW_ADVANCED
