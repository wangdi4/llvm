; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test is same as eh_with_nse_2.ll except ASIZE is used instead of
; A[0].size and P[0].size in conditions in the following lines in
; "check" and "getElement"  functions.
;       if (Pos >= ASIZE) // Pos >= P[0].size
;
;       for (i = 0; i < ASIZE; i++) {  // i < A[0].size
; 
; This test makes sure IPPredOpt is triggered even though "getElement"
; function has EH code. Can prove at compile-time that EH code is never
; executed.

; RUN: opt < %s -S -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=ippredopt 2>&1 | FileCheck %s

; CHECK:  %25 = load i32, ptr @GV.body
; CHECK:  %26 = icmp ne i32 %25, 0
; CHECK:  %27 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4)
; CHECK:  %28 = icmp eq i32 %27, 0
; CHECK:  %29 = select i1 %26, i1 %28, i1 false
; CHECK:  %30 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 8)
; CHECK:  %31 = icmp ne i32 %30, 0
; CHECK:  %32 = select i1 %29, i1 %31, i1 false
; CHECK:  %33 = load i32, ptr @GV.body
; CHECK:  %34 = icmp ne i32 %33, 0
; CHECK:  %35 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4)
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
;   if (Pos >= ASIZE)
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
;   for (i = 0; i < ASIZE; i++) {
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

%__DFT_struct._ZTS1S.S = type { i32, i32, i32 }

@str = private unnamed_addr constant [8 x i8] c"Hello 2\00", align 1
@str.3 = private unnamed_addr constant [8 x i8] c"Hello 1\00", align 1
@str.4 = private unnamed_addr constant [8 x i8] c"Hello 3\00", align 1
@GV.body = internal unnamed_addr global [12 x i8] undef
@llvm.compiler.used = appending global [1 x ptr] [ptr @__intel_new_feature_proc_init], section "llvm.metadata", !intel_dtrans_type !0

; Function Attrs: nofree nounwind
declare dso_local void @__intel_new_feature_proc_init(i32, i64) #0

; Function Attrs: mustprogress nofree noinline norecurse uwtable
define internal fastcc noundef i32 @_Z11getElement3P1Sj(ptr nocapture noundef readonly "intel_dtrans_func_index"="1" %0, i32 noundef %1) unnamed_addr #1 !intel.dtrans.func.type !10 {
  %3 = icmp ugt i32 %1, 999
  br i1 %3, label %4, label %5

4:                                                ; preds = %2
  tail call void @__cxa_rethrow() #7
  unreachable

5:                                                ; preds = %2
  %6 = zext i32 %1 to i64
  %7 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %0, i64 %6, i32 2
  %8 = load i32, ptr %7, align 4, !tbaa !12
  ret i32 %8
}

; Function Attrs: nofree
declare dso_local void @__cxa_rethrow() local_unnamed_addr #2

; Function Attrs: mustprogress nofree noinline norecurse uwtable
define internal fastcc noundef zeroext i1 @_Z5checkP1SS0_(ptr nocapture noundef readonly "intel_dtrans_func_index"="1" %0, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %1) unnamed_addr #1 !intel.dtrans.func.type !17 {
  %3 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 8), align 4, !tbaa !12
  %4 = load i32, ptr @GV.body, align 4, !tbaa !18
  %5 = icmp sgt i32 %3, %4
  br i1 %5, label %63, label %9

6:                                                ; preds = %9
  %7 = add nuw nsw i64 %10, 1
  %8 = icmp eq i64 %7, 1000
  br i1 %8, label %19, label %9, !llvm.loop !19

9:                                                ; preds = %6, %2
  %10 = phi i64 [ %7, %6 ], [ 0, %2 ]
  %11 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %0, i64 %10, i32 0
  %12 = load i32, ptr %11, align 4, !tbaa !18
  %13 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %1, i64 %10, i32 0
  %14 = load i32, ptr %13, align 4, !tbaa !18
  %15 = icmp eq i32 %12, %14
  br i1 %15, label %63, label %6

16:                                               ; preds = %19
  %17 = add nuw nsw i64 %20, 1
  %18 = icmp eq i64 %17, 1000
  br i1 %18, label %29, label %19, !llvm.loop !21

19:                                               ; preds = %16, %6
  %20 = phi i64 [ %17, %16 ], [ 0, %6 ]
  %21 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %0, i64 %20, i32 1
  %22 = load i32, ptr %21, align 4, !tbaa !22
  %23 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %1, i64 %20, i32 1
  %24 = load i32, ptr %23, align 4, !tbaa !22
  %25 = icmp eq i32 %22, %24
  br i1 %25, label %63, label %16

26:                                               ; preds = %29
  %27 = add nuw nsw i64 %30, 1
  %28 = icmp eq i64 %27, 1000
  br i1 %28, label %39, label %29, !llvm.loop !23

29:                                               ; preds = %26, %16
  %30 = phi i64 [ %27, %26 ], [ 0, %16 ]
  %31 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %0, i64 %30, i32 2
  %32 = load i32, ptr %31, align 4, !tbaa !12
  %33 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %1, i64 %30, i32 2
  %34 = load i32, ptr %33, align 4, !tbaa !12
  %35 = icmp eq i32 %32, %34
  br i1 %35, label %63, label %26

36:                                               ; preds = %39
  %37 = add nuw nsw i64 %40, 1
  %38 = icmp eq i64 %37, 1000
  br i1 %38, label %49, label %39, !llvm.loop !24

39:                                               ; preds = %36, %26
  %40 = phi i64 [ %37, %36 ], [ 0, %26 ]
  %41 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %0, i64 %40, i32 0
  %42 = load i32, ptr %41, align 4, !tbaa !18
  %43 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %1, i64 %40, i32 0
  %44 = load i32, ptr %43, align 4, !tbaa !18
  %45 = icmp slt i32 %42, %44
  br i1 %45, label %63, label %36

46:                                               ; preds = %49
  %47 = add nuw nsw i64 %50, 1
  %48 = icmp eq i64 %47, 1000
  br i1 %48, label %59, label %49, !llvm.loop !25

49:                                               ; preds = %46, %36
  %50 = phi i64 [ %47, %46 ], [ 0, %36 ]
  %51 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %0, i64 %50, i32 1
  %52 = load i32, ptr %51, align 4, !tbaa !22
  %53 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %1, i64 %50, i32 2
  %54 = load i32, ptr %53, align 4, !tbaa !12
  %55 = icmp slt i32 %52, %54
  br i1 %55, label %63, label %46

56:                                               ; preds = %59
  %57 = add nuw nsw i32 %60, 1
  %58 = icmp eq i32 %57, 1000
  br i1 %58, label %63, label %59, !llvm.loop !26

59:                                               ; preds = %56, %46
  %60 = phi i32 [ %57, %56 ], [ 0, %46 ]
  %61 = tail call fastcc noundef i32 @_Z11getElement3P1Sj(ptr noundef %0, i32 noundef %60)
  %62 = icmp eq i32 %61, 200
  br i1 %62, label %63, label %56

63:                                               ; preds = %59, %56, %49, %39, %29, %19, %9, %2
  %64 = phi i1 [ false, %2 ], [ true, %59 ], [ false, %56 ], [ true, %49 ], [ true, %39 ], [ true, %29 ], [ true, %19 ], [ true, %9 ]
  ret i1 %64
}

; Function Attrs: mustprogress nofree norecurse uwtable
define dso_local noundef i32 @main() local_unnamed_addr #3 {
  tail call void @llvm.memset.p0.i64(ptr noundef nonnull align 1 dereferenceable(12) @GV.body, i8 0, i64 12, i1 false)
  %1 = tail call i32 @rand() #8
  %2 = icmp eq i32 %1, 0
  %3 = select i1 %2, i32 40, i32 10
  %4 = select i1 %2, i32 50, i32 20
  %5 = select i1 %2, i32 60, i32 30
  store i32 %3, ptr @GV.body, align 4
  store i32 %4, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4), align 4
  store i32 %5, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 8), align 4
  %6 = tail call noalias dereferenceable_or_null(12000) ptr @malloc(i64 noundef 12000) #9
  %7 = tail call noalias dereferenceable_or_null(12000) ptr @malloc(i64 noundef 12000) #9
  br label %8

8:                                                ; preds = %8, %0
  %9 = phi i64 [ 0, %0 ], [ %22, %8 ]
  %10 = tail call i32 @rand() #8
  %11 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %6, i64 %9, i32 0
  store i32 %10, ptr %11, align 4, !tbaa !18
  %12 = tail call i32 @rand() #8
  %13 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %6, i64 %9, i32 1
  store i32 %12, ptr %13, align 4, !tbaa !22
  %14 = tail call i32 @rand() #8
  %15 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %6, i64 %9, i32 2
  store i32 %14, ptr %15, align 4, !tbaa !12
  %16 = tail call i32 @rand() #8
  %17 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %7, i64 %9, i32 0
  store i32 %16, ptr %17, align 4, !tbaa !18
  %18 = tail call i32 @rand() #8
  %19 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %7, i64 %9, i32 1
  store i32 %18, ptr %19, align 4, !tbaa !22
  %20 = tail call i32 @rand() #8
  %21 = getelementptr inbounds %__DFT_struct._ZTS1S.S, ptr %7, i64 %9, i32 2
  store i32 %20, ptr %21, align 4, !tbaa !12
  %22 = add nuw nsw i64 %9, 1
  %23 = icmp eq i64 %22, 1000
  br i1 %23, label %24, label %8, !llvm.loop !27

24:                                               ; preds = %8
  %25 = load i32, ptr @GV.body, align 4, !tbaa !18
  %26 = icmp ne i32 %25, 0
  %27 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4), align 4, !tbaa !22
  %28 = icmp eq i32 %27, 0
  %29 = select i1 %26, i1 %28, i1 false
  %30 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 8), align 4, !tbaa !12
  %31 = icmp ne i32 %30, 0
  %32 = select i1 %29, i1 %31, i1 false
  %33 = load i32, ptr @GV.body, align 4, !tbaa !18
  %34 = icmp ne i32 %33, 0
  %35 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4), align 4, !tbaa !22
  %36 = icmp ne i32 %35, 0
  %37 = select i1 %34, i1 %36, i1 false
  %38 = select i1 %32, i1 true, i1 %37
  br i1 %38, label %39, label %54

39:                                               ; preds = %24
  %40 = tail call fastcc noundef zeroext i1 @_Z5checkP1SS0_(ptr noundef nonnull %6, ptr noundef nonnull %7)
  br i1 %40, label %41, label %54

41:                                               ; preds = %39
  %42 = load i32, ptr @GV.body, align 4, !tbaa !18
  %43 = icmp eq i32 %42, 0
  br i1 %43, label %54, label %44

44:                                               ; preds = %41
  %45 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 4), align 4, !tbaa !22
  %46 = icmp eq i32 %45, 0
  br i1 %46, label %49, label %47

47:                                               ; preds = %44
  %48 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.3)
  br label %54

49:                                               ; preds = %44
  %50 = load i32, ptr getelementptr inbounds ([12 x i8], ptr @GV.body, i64 0, i64 8), align 4, !tbaa !12
  %51 = icmp eq i32 %50, 0
  br i1 %51, label %54, label %52

52:                                               ; preds = %49
  %53 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %54

54:                                               ; preds = %24, %52, %49, %47, %41, %39
  %55 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.4)
  ret i32 0
}

; Function Attrs: nofree nounwind
declare dso_local i32 @rand() local_unnamed_addr #4

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare !intel.dtrans.func.type !28 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #5

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
!2 = !{!"S", %__DFT_struct._ZTS1S.S zeroinitializer, i32 3, !3, !3, !3}
!3 = !{i32 0, i32 0}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.1.0 (2024.x.0.YYYYMMDD)"}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 1, !"Virtual Function Elim", i32 0}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 1, !"ThinLTO", i32 0}
!9 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!10 = distinct !{!11}
!11 = !{%__DFT_struct._ZTS1S.S zeroinitializer, i32 1}
!12 = !{!13, !14, i64 8}
!13 = !{!"struct@_ZTS1S", !14, i64 0, !14, i64 4, !14, i64 8, !14, i64 12}
!14 = !{!"int", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C++ TBAA"}
!17 = distinct !{!11, !11}
!18 = !{!13, !14, i64 0}
!19 = distinct !{!19, !20}
!20 = !{!"llvm.loop.mustprogress"}
!21 = distinct !{!21, !20}
!22 = !{!13, !14, i64 4}
!23 = distinct !{!23, !20}
!24 = distinct !{!24, !20}
!25 = distinct !{!25, !20}
!26 = distinct !{!26, !20}
!27 = distinct !{!27, !20}
!28 = distinct !{!1}

; end INTEL_FEATURE_SW_DTRANS
