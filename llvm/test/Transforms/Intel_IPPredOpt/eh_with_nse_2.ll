; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test is same as eh_with_nse_1.ll except condition in the
; following line in "check" function is different.
;       for (i = 0; i < A[0].size; i++) { // i != A[0].size
;
; 
; This test makes sure IPPredOpt is triggered even though "getElement"
; function has EH code. Can prove at compile-time that EH code is never
; executed.

; RUN: opt < %s -S -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=ippredopt 2>&1 | FileCheck %s

; CHECK:  %25 = load i32, ptr @GV.body
; CHECK:  %26 = icmp ne i32 %25, 0
; CHECK:  %27 = load i32, ptr getelementptr inbounds ([16 x i8], ptr @GV.body, i64 0, i64 4)
; CHECK:  %28 = icmp eq i32 %27, 0
; CHECK:  %29 = select i1 %26, i1 %28, i1 false
; CHECK:  %30 = load i32, ptr getelementptr inbounds ([16 x i8], ptr @GV.body, i64 0, i64 8)
; CHECK:  %31 = icmp ne i32 %30, 0
; CHECK:  %32 = select i1 %29, i1 %31, i1 false
; CHECK:  %33 = load i32, ptr @GV.body
; CHECK:  %34 = icmp ne i32 %33, 0
; CHECK:  %35 = load i32, ptr getelementptr inbounds ([16 x i8], ptr @GV.body, i64 0, i64 4)
; CHECK:  %36 = icmp ne i32 %35, 0
; CHECK:  %37 = select i1 %34, i1 %36, i1 false
; CHECK:  %38 = select i1 %32, i1 true, i1 %37
; CHECK:  br i1 %38, label %39, label %54

; CHECK: 39:                                               ; preds = %24
; CHECK:  %40 = tail call fastcc noundef zeroext i1 @_Z5checkP1SS0_(ptr noundef nonnull %6, ptr noundef nonnull %7)
; CHECK:  br i1 %40, label %41, label %54


; Source program corresponding to this lit test.
;
; #include <stdio.h>
; #include <stdlib.h>
; #include <stdexcept>
; 
; #define ASIZE 1000
; 
; using namespace std;
; 
; struct S {
;   int field1;
;   int field2;
;   int field3;
;   unsigned size;
; };
; 
; struct S *GV;
; 
; int  __attribute__((noinline)) getElement3(struct S *P, unsigned Pos) {
;   if (Pos >= P[0].size)
;     throw;
;     //throw std::out_of_range("Pos: Incorrect argument");
;   return P[Pos].field3;
; }
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
;       return true;
;   }
;   for (i = 0; i < ASIZE; i++) {
;     if (A[i].field1 < B[i].field1)
;       return true;
;   }
;   for (i = 0; i < ASIZE; i++) {
;     if (A[i].field2 < B[i].field3)
;       return true;
;   }
;   for (i = 0; i < A[0].size; i++) {
;     if (getElement3(A, i) == 200)
;       return true;
;   }
;   return false;
; }
; 
; int main() {
;   unsigned i = 0;
;   struct S *A, *B;
;   GV = (struct S*)calloc(1, sizeof(struct S));
;   if (rand()) {
;     GV->field1 = 10;
;     GV->field2 = 20;
;     GV->field3 = 30;
;   } else {
;     GV->field1 = 40;
;     GV->field2 = 50;
;     GV->field3 = 60;
;   }
;   A = (struct S*)malloc(ASIZE * sizeof(struct S));
;   B = (struct S*)malloc(ASIZE * sizeof(struct S));
;   for (i = 0; i < ASIZE; i++) {
;     A[i].field1 = rand();
;     A[i].field2 = rand();
;     A[i].field3 = rand();
;     B[i].field1 = rand();
;     B[i].field2 = rand();
;     B[i].field3 = rand();
;   }
;   if (check(A, B)) {
;       if (GV->field1) {
;         if (GV->field2)
;           printf("Hello 1\n");
;         else if (GV->field3)
;           printf("Hello 2\n");
;       }
;   }
;   printf("Hello 3\n");
;   return 0;
; }
; 

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS1S.S = type { i32, i32, i32, i32 }

@str = private unnamed_addr constant [8 x i8] c"Hello 2\00", align 1
@str.3 = private unnamed_addr constant [8 x i8] c"Hello 1\00", align 1
@str.4 = private unnamed_addr constant [8 x i8] c"Hello 3\00", align 1
@GV.body = internal unnamed_addr global [16 x i8] undef
@llvm.compiler.used = appending global [1 x ptr] [ptr @__intel_new_feature_proc_init], section "llvm.metadata", !intel_dtrans_type !0

; Function Attrs: nofree nounwind
declare dso_local void @__intel_new_feature_proc_init(i32, i64) #0

; Function Attrs: mustprogress nofree noinline norecurse uwtable
define internal fastcc noundef i32 @_Z11getElement3P1Sj(ptr nocapture noundef readonly "intel_dtrans_func_index"="1" %0, i32 noundef %1) unnamed_addr #1 !intel.dtrans.func.type !10 {
  %3 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 0, i32 3, !intel-tbaa !12
  %4 = load i32, ptr %3, align 4, !tbaa !12
  %5 = icmp ugt i32 %4, %1
  br i1 %5, label %7, label %6

6:                                                ; preds = %2
  tail call void @__cxa_rethrow() #7
  unreachable

7:                                                ; preds = %2
  %8 = zext i32 %1 to i64
  %9 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %8, i32 2
  %10 = load i32, ptr %9, align 4, !tbaa !17
  ret i32 %10
}

; Function Attrs: nofree
declare dso_local void @__cxa_rethrow() local_unnamed_addr #2

; Function Attrs: mustprogress nofree noinline norecurse uwtable
define internal fastcc noundef zeroext i1 @_Z5checkP1SS0_(ptr nocapture noundef readonly "intel_dtrans_func_index"="1" %0, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %1) unnamed_addr #1 !intel.dtrans.func.type !18 {
  %3 = load i32, ptr getelementptr inbounds ([16 x i8], ptr @GV.body, i64 0, i64 8), align 4, !tbaa !17
  %4 = load i32, ptr @GV.body, align 4, !tbaa !19
  %5 = icmp sgt i32 %3, %4
  br i1 %5, label %68, label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, 1000
  br i1 %8, label %19, label %9, !llvm.loop !20

9:                                                ; preds = %6, %2
  %10 = phi i64 [ %7, %6 ], [ 0, %2 ]
  %11 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %10, i32 0
  %12 = load i32, ptr %11, align 4, !tbaa !19
  %13 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %10, i32 0
  %14 = load i32, ptr %13, align 4, !tbaa !19
  %15 = icmp eq i32 %12, %14
  br i1 %15, label %68, label %6

16:                                               ; preds = %19
  %17 = add nuw nsw i64 %20, 1
  %18 = icmp eq i64 %17, 1000
  br i1 %18, label %29, label %19, !llvm.loop !22

19:                                               ; preds = %16, %6
  %20 = phi i64 [ %17, %16 ], [ 0, %6 ]
  %21 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %20, i32 1
  %22 = load i32, ptr %21, align 4, !tbaa !23
  %23 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %20, i32 1
  %24 = load i32, ptr %23, align 4, !tbaa !23
  %25 = icmp eq i32 %22, %24
  br i1 %25, label %68, label %16

26:                                               ; preds = %29
  %27 = add nuw nsw i64 %30, 1
  %28 = icmp eq i64 %27, 1000
  br i1 %28, label %39, label %29, !llvm.loop !24

29:                                               ; preds = %26, %16
  %30 = phi i64 [ %27, %26 ], [ 0, %16 ]
  %31 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %30, i32 2
  %32 = load i32, ptr %31, align 4, !tbaa !17
  %33 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %30, i32 2
  %34 = load i32, ptr %33, align 4, !tbaa !17
  %35 = icmp eq i32 %32, %34
  br i1 %35, label %68, label %26

36:                                               ; preds = %39
  %37 = add nuw nsw i64 %40, 1
  %38 = icmp eq i64 %37, 1000
  br i1 %38, label %53, label %39, !llvm.loop !25

39:                                               ; preds = %36, %26
  %40 = phi i64 [ %37, %36 ], [ 0, %26 ]
  %41 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %40, i32 0
  %42 = load i32, ptr %41, align 4, !tbaa !19
  %43 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %40, i32 0
  %44 = load i32, ptr %43, align 4, !tbaa !19
  %45 = icmp slt i32 %42, %44
  br i1 %45, label %68, label %36

46:                                               ; preds = %53
  %47 = add nuw nsw i64 %54, 1
  %48 = icmp eq i64 %47, 1000
  br i1 %48, label %49, label %53, !llvm.loop !26

49:                                               ; preds = %46
  %50 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 0, i32 3, !intel-tbaa !12
  %51 = load i32, ptr %50, align 4, !tbaa !12
  %52 = icmp eq i32 %51, 0
  br i1 %52, label %68, label %64

53:                                               ; preds = %46, %36
  %54 = phi i64 [ %47, %46 ], [ 0, %36 ]
  %55 = getelementptr inbounds %struct._ZTS1S.S, ptr %0, i64 %54, i32 1
  %56 = load i32, ptr %55, align 4, !tbaa !23
  %57 = getelementptr inbounds %struct._ZTS1S.S, ptr %1, i64 %54, i32 2
  %58 = load i32, ptr %57, align 4, !tbaa !17
  %59 = icmp slt i32 %56, %58
  br i1 %59, label %68, label %46

60:                                               ; preds = %64
  %61 = add nuw nsw i32 %65, 1
  %62 = load i32, ptr %50, align 4, !tbaa !12
  %63 = icmp ult i32 %61, %62
  br i1 %63, label %64, label %68, !llvm.loop !27

64:                                               ; preds = %60, %49
  %65 = phi i32 [ %61, %60 ], [ 0, %49 ]
  %66 = tail call fastcc noundef i32 @_Z11getElement3P1Sj(ptr noundef nonnull %0, i32 noundef %65)
  %67 = icmp eq i32 %66, 200
  br i1 %67, label %68, label %60

68:                                               ; preds = %64, %60, %53, %49, %39, %29, %19, %9, %2
  %69 = phi i1 [ false, %2 ], [ false, %49 ], [ false, %60 ], [ true, %64 ], [ true, %53 ], [ true, %39 ], [ true, %29 ], [ true, %19 ], [ true, %9 ]
  ret i1 %69
}

; Function Attrs: mustprogress nofree norecurse uwtable
define dso_local noundef i32 @main() local_unnamed_addr #3 {
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 1 dereferenceable(16) @GV.body, i8 0, i64 16, i1 false)
  %1 = tail call i32 @rand() #8
  %2 = icmp eq i32 %1, 0
  %3 = select i1 %2, i32 40, i32 10
  %4 = select i1 %2, i32 50, i32 20
  %5 = select i1 %2, i32 60, i32 30
  store i32 %3, ptr @GV.body, align 4
  store i32 %4, ptr getelementptr inbounds ([16 x i8], ptr @GV.body, i64 0, i64 4), align 4
  store i32 %5, ptr getelementptr inbounds ([16 x i8], ptr @GV.body, i64 0, i64 8), align 4
  %6 = tail call noalias dereferenceable_or_null(16000) ptr @malloc(i64 noundef 16000) #9
  %7 = tail call noalias dereferenceable_or_null(16000) ptr @malloc(i64 noundef 16000) #9
  br label %8

8:                                                ; preds = %8, %0
  %9 = phi i64 [ 0, %0 ], [ %22, %8 ]
  %10 = tail call i32 @rand() #8
  %11 = getelementptr inbounds %struct._ZTS1S.S, ptr %6, i64 %9, i32 0
  store i32 %10, ptr %11, align 4, !tbaa !19
  %12 = tail call i32 @rand() #8
  %13 = getelementptr inbounds %struct._ZTS1S.S, ptr %6, i64 %9, i32 1
  store i32 %12, ptr %13, align 4, !tbaa !23
  %14 = tail call i32 @rand() #8
  %15 = getelementptr inbounds %struct._ZTS1S.S, ptr %6, i64 %9, i32 2
  store i32 %14, ptr %15, align 4, !tbaa !17
  %16 = tail call i32 @rand() #8
  %17 = getelementptr inbounds %struct._ZTS1S.S, ptr %7, i64 %9, i32 0
  store i32 %16, ptr %17, align 4, !tbaa !19
  %18 = tail call i32 @rand() #8
  %19 = getelementptr inbounds %struct._ZTS1S.S, ptr %7, i64 %9, i32 1
  store i32 %18, ptr %19, align 4, !tbaa !23
  %20 = tail call i32 @rand() #8
  %21 = getelementptr inbounds %struct._ZTS1S.S, ptr %7, i64 %9, i32 2
  store i32 %20, ptr %21, align 4, !tbaa !17
  %22 = add nuw nsw i64 %9, 1
  %23 = icmp eq i64 %22, 1000
  br i1 %23, label %24, label %8, !llvm.loop !28

24:                                               ; preds = %8
  %25 = tail call fastcc noundef zeroext i1 @_Z5checkP1SS0_(ptr noundef nonnull %6, ptr noundef nonnull %7)
  br i1 %25, label %26, label %39

26:                                               ; preds = %24
  %27 = load i32, ptr @GV.body, align 4, !tbaa !19
  %28 = icmp eq i32 %27, 0
  br i1 %28, label %39, label %29

29:                                               ; preds = %26
  %30 = load i32, ptr getelementptr inbounds ([16 x i8], ptr @GV.body, i64 0, i64 4), align 4, !tbaa !23
  %31 = icmp eq i32 %30, 0
  br i1 %31, label %34, label %32

32:                                               ; preds = %29
  %33 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.3)
  br label %39

34:                                               ; preds = %29
  %35 = load i32, ptr getelementptr inbounds ([16 x i8], ptr @GV.body, i64 0, i64 8), align 4, !tbaa !17
  %36 = icmp eq i32 %35, 0
  br i1 %36, label %39, label %37

37:                                               ; preds = %34
  %38 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %39

39:                                               ; preds = %37, %34, %32, %26, %24
  %40 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.4)
  ret i32 0
}

; Function Attrs: nofree nounwind
declare dso_local i32 @rand() local_unnamed_addr #4

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare !intel.dtrans.func.type !29 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #5

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #0

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #6

attributes #0 = { nofree nounwind }
attributes #1 = { mustprogress nofree noinline norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nofree }
attributes #3 = { mustprogress nofree norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #7 = { noreturn }
attributes #8 = { nounwind }
attributes #9 = { nounwind allocsize(0) }

!intel.dtrans.types = !{!2}
!llvm.ident = !{!4}
!llvm.module.flags = !{!5, !6, !7, !8, !9}

!0 = !{!"A", i32 1, !1}
!1 = !{i8 0, i32 1}
!2 = !{!"S", %struct._ZTS1S.S zeroinitializer, i32 4, !3, !3, !3, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.1.0 (2024.x.0.YYYYMMDD)"}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 1, !"Virtual Function Elim", i32 0}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 1, !"ThinLTO", i32 0}
!9 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!10 = distinct !{!11}
!11 = !{%struct._ZTS1S.S zeroinitializer, i32 1}
!12 = !{!13, !14, i64 12}
!13 = !{!"struct@_ZTS1S", !14, i64 0, !14, i64 4, !14, i64 8, !14, i64 12}
!14 = !{!"int", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C++ TBAA"}
!17 = !{!13, !14, i64 8}
!18 = distinct !{!11, !11}
!19 = !{!13, !14, i64 0}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.mustprogress"}
!22 = distinct !{!22, !21}
!23 = !{!13, !14, i64 4}
!24 = distinct !{!24, !21}
!25 = distinct !{!25, !21}
!26 = distinct !{!26, !21}
!27 = distinct !{!27, !21}
!28 = distinct !{!28, !21}
!29 = distinct !{!1}

; end INTEL_FEATURE_SW_DTRANS
