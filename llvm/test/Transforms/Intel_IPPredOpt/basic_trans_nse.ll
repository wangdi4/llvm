; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test makes sure IPPredOpt is triggered for a simple case.

; RUN: opt < %s -S -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=ippredopt 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; #include <stdio.h>
; #include <stdlib.h>
; 
; #define ASIZE 1000
; 
; struct S {
;   int field1;
;   int field2;
;   int field3;
; };
; 
; struct S *GV;
; 
; int report1 = 0;
; int report2 = 0;
; int report3 = 0;
; 
; bool __attribute__((noinline)) check(struct S *A, struct S *B) {
;   int i;
;   if (GV->field3 >  GV->field1)
;     return false;
;   for (i = 0; i < ASIZE; i++) {
;     if (A[i].field1 == B[i].field1)
;       return true;
;   }
;   for (i = 0; i < ASIZE; i++) {
;     if (A[i].field2 == B[i].field2)
;       return true;
;   }
;   for (i = 0; i < ASIZE; i++) {
;     if (A[i].field3 == B[i].field3)
;      return true;
;  }
;  for (i = 0; i < ASIZE; i++) {
;    if (A[i].field1 < B[i].field1)
;      return true;
;  }
;  for (i = 0; i < ASIZE; i++) {
;    if (A[i].field2 < B[i].field3)
;      return true;
;  }
;  return false;
;}
;
;int main() {
;  int i = 0;
;  struct S *A, *B; 
;  report1 = rand();
;  report2 = rand();
;  report3 = rand();
;  GV = (struct S*)calloc(1, sizeof(struct S));
;  if (rand()) {
;    GV->field1 = 10;
;    GV->field2 = 20;
;    GV->field3 = 30;
;  } else {
;    GV->field1 = 40;
;    GV->field2 = 50;
;    GV->field3 = 60;
;  }
;  A = (struct S*)malloc(ASIZE * sizeof(struct S));
;  B = (struct S*)malloc(ASIZE * sizeof(struct S));
;  for (i = 0; i < ASIZE; i++) {
;    A[i].field1 = rand();
;    A[i].field2 = rand();
;    A[i].field3 = rand();
;    B[i].field1 = rand();
;    B[i].field2 = rand();
;    B[i].field3 = rand();
;  }
;  if (check(A, B)) {
;      if (GV->field1) {
;        if (GV->field2)
;          printf("Hello 1\n");
;        else if (GV->field3)
;          printf("Hello 2\n");
;      }
;  }
;  printf("Hello 3\n");
;  return 0;
;}

; CHECK: 27:                                               ; preds = %11
; CHECK:  %28 = load i32, ptr @GV.body
; CHECK:  %29 = icmp ne i32 %28, 0
; CHECK:  %30 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4)
; CHECK:  %31 = icmp eq i32 %30, 0
; CHECK:  %32 = select i1 %29, i1 %31, i1 false
; CHECK:  %33 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 8)
; CHECK:  %34 = icmp ne i32 %33, 0
; CHECK:  %35 = select i1 %32, i1 %34, i1 false
; CHECK:  %36 = load i32, ptr @GV.body, align 4, !tbaa !17
; CHECK:  %37 = icmp ne i32 %36, 0
; CHECK:  %38 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4)
; CHECK:  %39 = icmp ne i32 %38, 0
; CHECK:  %40 = select i1 %37, i1 %39, i1 false
; CHECK:  %41 = select i1 %35, i1 true, i1 %40
; CHECK:  br i1 %41, label %42, label %57

; CHECK: 42:                                               ; preds = %27
; CHECK:  %43 = tail call fastcc noundef zeroext i1 @_Z5checkP1SS0_(ptr noundef nonnull %9, ptr noundef nonnull %10)
; CHECK:  br i1 %43, label %44, label %57


%struct._ZTS1S.S = type { i32, i32, i32 }

@str = private unnamed_addr constant [8 x i8] c"Hello 2\00", align 1
@str.3 = private unnamed_addr constant [8 x i8] c"Hello 1\00", align 1
@str.4 = private unnamed_addr constant [8 x i8] c"Hello 3\00", align 1
@GV.body = internal unnamed_addr global [12 x i8] undef
@llvm.compiler.used = appending global [1 x ptr] [ptr @__intel_new_feature_proc_init], section "llvm.metadata", !intel_dtrans_type !0

; Function Attrs: nofree nounwind
declare dso_local void @__intel_new_feature_proc_init(i32, i64) #0

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable
define internal fastcc noundef zeroext i1 @_Z5checkP1SS0_(ptr nocapture noundef readonly "intel_dtrans_func_index"="1" %0, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %1) unnamed_addr #1 !intel.dtrans.func.type !10 {
  %3 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 8), align 4, !tbaa !12
  %4 = load i32, ptr @GV.body, align 4, !tbaa !17
  %5 = icmp sgt i32 %3, %4
  br i1 %5, label %56, label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, 1000
  br i1 %8, label %19, label %9, !llvm.loop !18

9:                                                ; preds = %6, %2
  %10 = phi i64 [ %7, %6 ], [ 0, %2 ]
  %11 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %10, i32 0
  %12 = load i32, ptr %11, align 4, !tbaa !17
  %13 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %10, i32 0
  %14 = load i32, ptr %13, align 4, !tbaa !17
  %15 = icmp eq i32 %12, %14
  br i1 %15, label %56, label %6

16:                                               ; preds = %19
  %17 = add nuw nsw i64 %20, 1
  %18 = icmp eq i64 %17, 1000
  br i1 %18, label %29, label %19, !llvm.loop !20

19:                                               ; preds = %16, %6
  %20 = phi i64 [ %17, %16 ], [ 0, %6 ]
  %21 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %20, i32 1
  %22 = load i32, ptr %21, align 4, !tbaa !21
  %23 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %20, i32 1
  %24 = load i32, ptr %23, align 4, !tbaa !21
  %25 = icmp eq i32 %22, %24
  br i1 %25, label %56, label %16

26:                                               ; preds = %29
  %27 = add nuw nsw i64 %30, 1
  %28 = icmp eq i64 %27, 1000
  br i1 %28, label %39, label %29, !llvm.loop !22

29:                                               ; preds = %26, %16
  %30 = phi i64 [ %27, %26 ], [ 0, %16 ]
  %31 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %30, i32 2
  %32 = load i32, ptr %31, align 4, !tbaa !12
  %33 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %30, i32 2
  %34 = load i32, ptr %33, align 4, !tbaa !12
  %35 = icmp eq i32 %32, %34
  br i1 %35, label %56, label %26

36:                                               ; preds = %39
  %37 = add nuw nsw i64 %40, 1
  %38 = icmp eq i64 %37, 1000
  br i1 %38, label %49, label %39, !llvm.loop !23

39:                                               ; preds = %36, %26
  %40 = phi i64 [ %37, %36 ], [ 0, %26 ]
  %41 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %40, i32 0
  %42 = load i32, ptr %41, align 4, !tbaa !17
  %43 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %40, i32 0
  %44 = load i32, ptr %43, align 4, !tbaa !17
  %45 = icmp slt i32 %42, %44
  br i1 %45, label %56, label %36

46:                                               ; preds = %49
  %47 = add nuw nsw i64 %50, 1
  %48 = icmp eq i64 %47, 1000
  br i1 %48, label %56, label %49, !llvm.loop !24

49:                                               ; preds = %46, %36
  %50 = phi i64 [ %47, %46 ], [ 0, %36 ]
  %51 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %50, i32 1
  %52 = load i32, ptr %51, align 4, !tbaa !21
  %53 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %50, i32 2
  %54 = load i32, ptr %53, align 4, !tbaa !12
  %55 = icmp slt i32 %52, %54
  br i1 %55, label %56, label %46

56:                                               ; preds = %49, %46, %39, %29, %19, %9, %2
  %57 = phi i1 [ false, %2 ], [ true, %49 ], [ false, %46 ], [ true, %39 ], [ true, %29 ], [ true, %19 ], [ true, %9 ]
  ret i1 %57
}

; Function Attrs: mustprogress nofree norecurse nounwind uwtable
define dso_local noundef i32 @main() local_unnamed_addr #2 {
  %1 = tail call i32 @rand() #6
  %2 = tail call i32 @rand() #6
  %3 = tail call i32 @rand() #6
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 1 dereferenceable(12) @GV.body, i8 0, i64 12, i1 false)
  %4 = tail call i32 @rand() #6
  %5 = icmp eq i32 %4, 0
  %6 = select i1 %5, i32 40, i32 10
  %7 = select i1 %5, i32 50, i32 20
  %8 = select i1 %5, i32 60, i32 30
  store i32 %6, ptr @GV.body, align 4
  store i32 %7, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4), align 4
  store i32 %8, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 8), align 4
  %9 = tail call noalias dereferenceable_or_null(12000) ptr @malloc(i64 noundef 12000) #7
  %10 = tail call noalias dereferenceable_or_null(12000) ptr @malloc(i64 noundef 12000) #7
  br label %11

11:                                               ; preds = %11, %0
  %12 = phi i64 [ 0, %0 ], [ %25, %11 ]
  %13 = tail call i32 @rand() #6
  %14 = getelementptr inbounds %struct._ZTS1S.S, ptr %9, i64 %12, i32 0
  store i32 %13, ptr %14, align 4, !tbaa !17
  %15 = tail call i32 @rand() #6
  %16 = getelementptr inbounds %struct._ZTS1S.S, ptr %9, i64 %12, i32 1
  store i32 %15, ptr %16, align 4, !tbaa !21
  %17 = tail call i32 @rand() #6
  %18 = getelementptr inbounds %struct._ZTS1S.S, ptr %9, i64 %12, i32 2
  store i32 %17, ptr %18, align 4, !tbaa !12
  %19 = tail call i32 @rand() #6
  %20 = getelementptr inbounds %struct._ZTS1S.S, ptr %10, i64 %12, i32 0
  store i32 %19, ptr %20, align 4, !tbaa !17
  %21 = tail call i32 @rand() #6
  %22 = getelementptr inbounds %struct._ZTS1S.S, ptr %10, i64 %12, i32 1
  store i32 %21, ptr %22, align 4, !tbaa !21
  %23 = tail call i32 @rand() #6
  %24 = getelementptr inbounds %struct._ZTS1S.S, ptr %10, i64 %12, i32 2
  store i32 %23, ptr %24, align 4, !tbaa !12
  %25 = add nuw nsw i64 %12, 1
  %26 = icmp eq i64 %25, 1000
  br i1 %26, label %27, label %11, !llvm.loop !25

27:                                               ; preds = %11
  %28 = tail call fastcc noundef zeroext i1 @_Z5checkP1SS0_(ptr noundef nonnull %9, ptr noundef nonnull %10)
  br i1 %28, label %29, label %42

29:                                               ; preds = %27
  %30 = load i32, ptr @GV.body, align 4, !tbaa !17
  %31 = icmp eq i32 %30, 0
  br i1 %31, label %42, label %32

32:                                               ; preds = %29
  %33 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4), align 4, !tbaa !21
  %34 = icmp eq i32 %33, 0
  br i1 %34, label %37, label %35

35:                                               ; preds = %32
  %36 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.3)
  br label %42

37:                                               ; preds = %32
  %38 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 8), align 4, !tbaa !12
  %39 = icmp eq i32 %38, 0
  br i1 %39, label %42, label %40

40:                                               ; preds = %37
  %41 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %42

42:                                               ; preds = %40, %37, %35, %29, %27
  %43 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.4)
  ret i32 0
}

; Function Attrs: nofree nounwind
declare dso_local i32 @rand() local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare !intel.dtrans.func.type !26 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #0

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #5

attributes #0 = { nofree nounwind }
attributes #1 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(read, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nofree norecurse nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #6 = { nounwind }
attributes #7 = { nounwind allocsize(0) }

!intel.dtrans.types = !{!2}
!llvm.ident = !{!4}
!llvm.module.flags = !{!5, !6, !7, !8, !9}

!0 = !{!"A", i32 1, !1}
!1 = !{i8 0, i32 1}
!2 = !{!"S", %struct._ZTS1S.S zeroinitializer, i32 3, !3, !3, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.1.0 (2024.x.0.YYYYMMDD)"}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 1, !"Virtual Function Elim", i32 0}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 1, !"ThinLTO", i32 0}
!9 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!10 = distinct !{!11, !11}
!11 = !{%struct._ZTS1S.S zeroinitializer, i32 1}
!12 = !{!13, !14, i64 8}
!13 = !{!"struct@_ZTS1S", !14, i64 0, !14, i64 4, !14, i64 8}
!14 = !{!"int", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C++ TBAA"}
!17 = !{!13, !14, i64 0}
!18 = distinct !{!18, !19}
!19 = !{!"llvm.loop.mustprogress"}
!20 = distinct !{!20, !19}
!21 = !{!13, !14, i64 4}
!22 = distinct !{!22, !19}
!23 = distinct !{!23, !19}
!24 = distinct !{!24, !19}
!25 = distinct !{!25, !19}
!26 = distinct !{!1}

; end INTEL_FEATURE_SW_DTRANS
