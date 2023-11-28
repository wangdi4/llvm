; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans

; This test makes sure IPPredOpt is triggered when some of the possible
; targets of a virtual call have side-effects. Runtime checks are generated
; to avoid possible targets with side-effects. Indirect call is in a loop
; with non-constant trip count so additional runtime checks are needed
; to prove that object is non-null pointer at runtime before generating
; other runtime checks.

; RUN: opt < %s -S -whole-program-assume -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=ippredopt 2>&1 | FileCheck %s

; CHECK: 38:                                               ; preds = %36
; CHECK:  %39 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 0
; CHECK:  %40 = load i32, ptr %39
; CHECK:  %callee.check = icmp ne i32 %40, 0
; CHECK:  br i1 %callee.check, label %41, label %76

; CHECK:41:                                               ; preds = %38
; CHECK:  %42 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 3
; CHECK:  %43 = load i32, ptr %42
; CHECK:  %callee.check1 = icmp ne i32 %43, 0
; CHECK:  br i1 %callee.check1, label %44, label %76

; CHECK:44:                                               ; preds = %41
; CHECK:  %45 = zext i32 %43 to i64
; CHECK:  %callee.check2 = icmp uge i64 %45, 1
; CHECK:  br i1 %callee.check2, label %46, label %76

; CHECK:46:                                               ; preds = %44
; CHECK:  %47 = getelementptr inbounds ptr, ptr %37, i64 0
; CHECK:  %nunull = icmp ne ptr %47, null
; CHECK:  br i1 %nunull, label %48, label %76

; CHECK:48:                                               ; preds = %46
; CHECK:  %49 = load ptr, ptr %47
; CHECK:  %50 = getelementptr %class._ZTS2S2.S2, ptr %49, i64 0, i32 0
; CHECK:  %51 = load ptr, ptr %50
; CHECK:  %52 = load ptr, ptr %51
; CHECK:  %callee.check3 = icmp eq ptr %52, @_ZN2T14testEv
; CHECK:  %callee.check4 = icmp eq ptr %52, @_ZN2T24testEv
; CHECK:  %53 = select i1 %callee.check3, i1 true, i1 %callee.check4
; CHECK:  %callee.check5 = icmp eq ptr %52, @_ZN2T34testEv
; CHECK:  %54 = select i1 %53, i1 true, i1 %callee.check5
; CHECK:  %callee.check6 = icmp eq ptr %52, @_ZN2T44testEv
; CHECK:  %55 = select i1 %54, i1 true, i1 %callee.check6
; CHECK:  br i1 %55, label %56, label %76

; CHECK:56:                                               ; preds = %48
; CHECK:  %57 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 0
; CHECK:  %58 = load i32, ptr %57
; CHECK:  %59 = icmp ne i32 %58, 0
; CHECK:  %60 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 1
; CHECK:  %61 = load i32, ptr %60
; CHECK:  %62 = icmp eq i32 %61, 0
; CHECK:  %63 = select i1 %59, i1 %62, i1 false
; CHECK:  %64 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 2
; CHECK:  %65 = load i32, ptr %64
; CHECK:  %66 = icmp ne i32 %65, 0
; CHECK:  %67 = select i1 %63, i1 %66, i1 false
; CHECK:  %68 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 0
; CHECK:  %69 = load i32, ptr %68
; CHECK:  %70 = icmp ne i32 %69, 0
; CHECK:  %71 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 1
; CHECK:  %72 = load i32, ptr %71
; CHECK:  %73 = icmp ne i32 %72, 0
; CHECK:  %74 = select i1 %70, i1 %73, i1 false
; CHECK:  %75 = select i1 %67, i1 true, i1 %74
; CHECK:  br i1 %75, label %76, label %94

; CHECK:76:                                               ; preds = %48, %46, %44, %41, %38, %56
; CHECK:  %77 = call fastcc noundef zeroext i1 @_Z5checkP2S1S0_PP2S2(ptr noundef nonnull %18, ptr noundef nonnull %19, ptr noundef nonnull %37)
; CHECK:  br i1 %77, label %78, label %94

; #include <stdio.h>
; #include <stdlib.h>
;
; #define ASIZE 1000
;
; int count = 0;
;
; struct S1 {
;   int field1;
;   int field2;
;   int field3;
;   int size;
; };
;
; class S2 {
; public:
;   int field4;
;   int field5;
;   virtual bool test() = 0;
; };
;
; class T1 : public S2 {
;   virtual bool test() {
;     return field4 > field5;
;   };
; };
;
; class T2 : public S2 {
;   virtual bool test() {
;     return field5 > field4 + 10;
;   };
; };
;
; class T3 : public S2 {
;   virtual bool test() {
;     return field4 + 2 > field5;
;   };
; };
;
; class T4 : public S2 {
;   virtual bool test() {
;     return field5 - 20 > field4;
;   };
; };
;
; class T5 : public S2 {
;   virtual bool test() {
;     count++;
;     return field5 - 30 > field4;
;   };
; };
;
; bool __attribute__((noinline)) check(struct S1 *A, struct S1 *B, struct S2 **C) {
;   unsigned i;
;   unsigned size = A[0].size; 
;   if (A[0].field1 == 0)
;     return false;
;   for (i = 0; i != size; i++) {
;      if (C[i]->test())
;        return true;
;   }
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
;   return false;
; }
;
; int main() {
;   int i = 0;
;   struct S1 *A, *B;
;   S2 *E[ASIZE];
;
;   for (i = 0; i < ASIZE; i++) {
;     int k = rand() % 5;
;     if (k == 0) {
;       E[i] = new T1;
;     } else if (k == 2) {
;       E[i] = new T2;
;     } else if (k == 3) {
;       E[i] = new T3;
;     } else if (k == 4) {
;       E[i] = new T4;
;     } else {
;       E[i] = new T5;
;     }
;   }
;
;   A = (struct S1*)malloc(ASIZE * sizeof(struct S1));
;   B = (struct S1*)malloc(ASIZE * sizeof(struct S1));
;
;   for (i = 0; i < ASIZE; i++) {
;     A[i].field1 = rand();
;     A[i].field2 = rand();
;     A[i].field3 = rand();
;     B[i].field1 = rand();
;     B[i].field2 = rand();
;     B[i].field3 = rand();
;   }
;   if (check(A, B, E)) {
;       if (A[0].field1) {
;         if (A[0].field2)
;           printf("Hello 1\n");
;         else if (A[0].field3)
;           printf("Hello 2\n");
;       }
;   }
;   printf("Hello 3\n");
;   printf("Count: %d \n", count);
;   return 0;
; }
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS2S1.S1 = type { i32, i32, i32, i32 }
%class._ZTS2S2.S2 = type { ptr, i32, i32 }
%class._ZTS2T1.T1 = type { %class._ZTS2S2.S2 }
%class._ZTS2T2.T2 = type { %class._ZTS2S2.S2 }
%class._ZTS2T3.T3 = type { %class._ZTS2S2.S2 }
%class._ZTS2T4.T4 = type { %class._ZTS2S2.S2 }
%class._ZTS2T5.T5 = type { %class._ZTS2S2.S2 }

$_ZN2T54testEv = comdat any

$_ZN2T44testEv = comdat any

$_ZN2T34testEv = comdat any

$_ZN2T24testEv = comdat any

$_ZN2T14testEv = comdat any

$_ZTS2T1 = comdat any

$_ZTS2S2 = comdat any

$_ZTI2S2 = comdat any

$_ZTI2T1 = comdat any

$_ZTS2T2 = comdat any

$_ZTI2T2 = comdat any

$_ZTS2T3 = comdat any

$_ZTI2T3 = comdat any

$_ZTS2T4 = comdat any

$_ZTI2T4 = comdat any

$_ZTS2T5 = comdat any

$_ZTI2T5 = comdat any

@count = internal unnamed_addr global i32 0, align 4
@.str.3 = private unnamed_addr constant [12 x i8] c"Count: %d \0A\00", align 1
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global [0 x ptr], !intel_dtrans_type !0
@_ZTS2T1 = internal constant [4 x i8] c"2T1\00", comdat, align 1
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global [0 x ptr], !intel_dtrans_type !0
@_ZTS2S2 = internal constant [4 x i8] c"2S2\00", comdat, align 1
@_ZTI2S2 = internal constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS2S2 }, comdat, align 8, !intel_dtrans_type !2
@_ZTI2T1 = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS2T1, ptr @_ZTI2S2 }, comdat, align 8, !intel_dtrans_type !3
@_ZTS2T2 = internal constant [4 x i8] c"2T2\00", comdat, align 1
@_ZTI2T2 = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS2T2, ptr @_ZTI2S2 }, comdat, align 8, !intel_dtrans_type !3
@_ZTS2T3 = internal constant [4 x i8] c"2T3\00", comdat, align 1
@_ZTI2T3 = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS2T3, ptr @_ZTI2S2 }, comdat, align 8, !intel_dtrans_type !3
@_ZTS2T4 = internal constant [4 x i8] c"2T4\00", comdat, align 1
@_ZTI2T4 = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS2T4, ptr @_ZTI2S2 }, comdat, align 8, !intel_dtrans_type !3
@_ZTS2T5 = internal constant [4 x i8] c"2T5\00", comdat, align 1
@_ZTI2T5 = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS2T5, ptr @_ZTI2S2 }, comdat, align 8, !intel_dtrans_type !3
@str = private unnamed_addr constant [8 x i8] c"Hello 2\00", align 1
@str.4 = private unnamed_addr constant [8 x i8] c"Hello 1\00", align 1
@str.5 = private unnamed_addr constant [8 x i8] c"Hello 3\00", align 1
@_ZTV2T1.0 = private constant [3 x ptr] [ptr null, ptr @_ZTI2T1, ptr @_ZN2T14testEv], !type !4, !type !5, !type !6, !type !7, !intel_dtrans_type !8
@_ZTV2T2.0 = private constant [3 x ptr] [ptr null, ptr @_ZTI2T2, ptr @_ZN2T24testEv], !type !4, !type !5, !type !9, !type !10, !intel_dtrans_type !8
@_ZTV2T3.0 = private constant [3 x ptr] [ptr null, ptr @_ZTI2T3, ptr @_ZN2T34testEv], !type !4, !type !5, !type !11, !type !12, !intel_dtrans_type !8
@_ZTV2T4.0 = private constant [3 x ptr] [ptr null, ptr @_ZTI2T4, ptr @_ZN2T44testEv], !type !4, !type !5, !type !13, !type !14, !intel_dtrans_type !8
@_ZTV2T5.0 = private constant [3 x ptr] [ptr null, ptr @_ZTI2T5, ptr @_ZN2T54testEv], !type !4, !type !5, !type !15, !type !16, !intel_dtrans_type !8
@llvm.compiler.used = appending global [1 x ptr] [ptr @__intel_new_feature_proc_init], section "llvm.metadata", !intel_dtrans_type !17

; Function Attrs: nofree nounwind
declare dso_local void @__intel_new_feature_proc_init(i32, i64) #0

; Function Attrs: mustprogress noinline norecurse uwtable
define internal fastcc noundef zeroext i1 @_Z5checkP2S1S0_PP2S2(ptr nocapture noundef readonly "intel_dtrans_func_index"="1" %0, ptr nocapture noundef readonly "intel_dtrans_func_index"="2" %1, ptr nocapture noundef readonly "intel_dtrans_func_index"="3" %2) unnamed_addr #1 !intel.dtrans.func.type !35 {
  %4 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %0, i64 0, i32 3, !intel-tbaa !38
  %5 = load i32, ptr %4, align 4, !tbaa !38
  %6 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %0, i64 0, i32 0, !intel-tbaa !43
  %7 = load i32, ptr %6, align 4, !tbaa !43
  %8 = icmp eq i32 %7, 0
  br i1 %8, label %76, label %9

9:                                                ; preds = %3
  %10 = icmp eq i32 %5, 0
  br i1 %10, label %16, label %11

11:                                               ; preds = %9
  %12 = zext i32 %5 to i64
  br label %17

13:                                               ; preds = %17
  %14 = add nuw nsw i64 %18, 1
  %15 = icmp eq i64 %14, %12
  br i1 %15, label %16, label %17, !llvm.loop !44

16:                                               ; preds = %13, %9
  br label %29

17:                                               ; preds = %13, %11
  %18 = phi i64 [ 0, %11 ], [ %14, %13 ]
  %19 = getelementptr inbounds ptr, ptr %2, i64 %18
  %20 = load ptr, ptr %19, align 8, !tbaa !46
  %21 = getelementptr %class._ZTS2S2.S2, ptr %20, i64 0, i32 0
  %22 = load ptr, ptr %21, align 8, !tbaa !48
  %23 = tail call i1 @llvm.type.test(ptr %22, metadata !"_ZTS2S2")
  tail call void @llvm.assume(i1 %23)
  %24 = load ptr, ptr %22, align 8
  %25 = tail call noundef zeroext i1 %24(ptr noundef nonnull align 8 dereferenceable(16) %20), !intel_dtrans_type !50
  br i1 %25, label %76, label %13

26:                                               ; preds = %29
  %27 = add nuw nsw i64 %30, 1
  %28 = icmp eq i64 %27, 1000
  br i1 %28, label %39, label %29, !llvm.loop !53

29:                                               ; preds = %26, %16
  %30 = phi i64 [ 0, %16 ], [ %27, %26 ]
  %31 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %0, i64 %30, i32 0
  %32 = load i32, ptr %31, align 4, !tbaa !43
  %33 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %1, i64 %30, i32 0
  %34 = load i32, ptr %33, align 4, !tbaa !43
  %35 = icmp eq i32 %32, %34
  br i1 %35, label %76, label %26

36:                                               ; preds = %39
  %37 = add nuw nsw i64 %40, 1
  %38 = icmp eq i64 %37, 1000
  br i1 %38, label %49, label %39, !llvm.loop !54

39:                                               ; preds = %36, %26
  %40 = phi i64 [ %37, %36 ], [ 0, %26 ]
  %41 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %0, i64 %40, i32 1
  %42 = load i32, ptr %41, align 4, !tbaa !55
  %43 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %1, i64 %40, i32 1
  %44 = load i32, ptr %43, align 4, !tbaa !55
  %45 = icmp eq i32 %42, %44
  br i1 %45, label %76, label %36

46:                                               ; preds = %49
  %47 = add nuw nsw i64 %50, 1
  %48 = icmp eq i64 %47, 1000
  br i1 %48, label %59, label %49, !llvm.loop !56

49:                                               ; preds = %46, %36
  %50 = phi i64 [ %47, %46 ], [ 0, %36 ]
  %51 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %0, i64 %50, i32 2
  %52 = load i32, ptr %51, align 4, !tbaa !57
  %53 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %1, i64 %50, i32 2
  %54 = load i32, ptr %53, align 4, !tbaa !57
  %55 = icmp eq i32 %52, %54
  br i1 %55, label %76, label %46

56:                                               ; preds = %59
  %57 = add nuw nsw i64 %60, 1
  %58 = icmp eq i64 %57, 1000
  br i1 %58, label %69, label %59, !llvm.loop !58

59:                                               ; preds = %56, %46
  %60 = phi i64 [ %57, %56 ], [ 0, %46 ]
  %61 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %0, i64 %60, i32 0
  %62 = load i32, ptr %61, align 4, !tbaa !43
  %63 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %1, i64 %60, i32 0
  %64 = load i32, ptr %63, align 4, !tbaa !43
  %65 = icmp slt i32 %62, %64
  br i1 %65, label %76, label %56

66:                                               ; preds = %69
  %67 = add nuw nsw i64 %70, 1
  %68 = icmp eq i64 %67, 1000
  br i1 %68, label %76, label %69, !llvm.loop !59

69:                                               ; preds = %66, %56
  %70 = phi i64 [ %67, %66 ], [ 0, %56 ]
  %71 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %0, i64 %70, i32 1
  %72 = load i32, ptr %71, align 4, !tbaa !55
  %73 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %1, i64 %70, i32 2
  %74 = load i32, ptr %73, align 4, !tbaa !57
  %75 = icmp slt i32 %72, %74
  br i1 %75, label %76, label %66

76:                                               ; preds = %69, %66, %59, %49, %39, %29, %17, %3
  %77 = phi i1 [ false, %3 ], [ true, %69 ], [ false, %66 ], [ true, %59 ], [ true, %49 ], [ true, %39 ], [ true, %29 ], [ true, %17 ]
  ret i1 %77
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write)
declare void @llvm.assume(i1 noundef) #2

; Function Attrs: norecurse uwtable
define dso_local noundef i32 @main() local_unnamed_addr #3 {
  %1 = alloca [1000 x ptr], align 16, !intel_dtrans_type !60
  call void @llvm.lifetime.start.p0(i64 8000, ptr nonnull %1) #13
  br label %2

2:                                                ; preds = %14, %0
  %3 = phi i64 [ 0, %0 ], [ %15, %14 ]
  %4 = tail call i32 @rand() #13
  %5 = srem i32 %4, 5
  %6 = tail call noalias noundef nonnull dereferenceable(16) ptr @_Znwm(i64 noundef 16) #14
  %7 = getelementptr inbounds %class._ZTS2S2.S2, ptr %6, i64 0, i32 0
  %8 = getelementptr inbounds [1000 x ptr], ptr %1, i64 0, i64 %3
  switch i32 %5, label %13 [
    i32 0, label %9
    i32 2, label %10
    i32 3, label %11
    i32 4, label %12
  ]

9:                                                ; preds = %2
  store ptr getelementptr inbounds ([3 x ptr], ptr @_ZTV2T1.0, i64 0, i64 2), ptr %7, align 8, !tbaa !48
  store ptr %6, ptr %8, align 8, !tbaa !61
  br label %14

10:                                               ; preds = %2
  store ptr getelementptr inbounds ([3 x ptr], ptr @_ZTV2T2.0, i64 0, i64 2), ptr %7, align 8, !tbaa !48
  store ptr %6, ptr %8, align 8, !tbaa !61
  br label %14

11:                                               ; preds = %2
  store ptr getelementptr inbounds ([3 x ptr], ptr @_ZTV2T3.0, i64 0, i64 2), ptr %7, align 8, !tbaa !48
  store ptr %6, ptr %8, align 8, !tbaa !61
  br label %14

12:                                               ; preds = %2
  store ptr getelementptr inbounds ([3 x ptr], ptr @_ZTV2T4.0, i64 0, i64 2), ptr %7, align 8, !tbaa !48
  store ptr %6, ptr %8, align 8, !tbaa !61
  br label %14

13:                                               ; preds = %2
  store ptr getelementptr inbounds ([3 x ptr], ptr @_ZTV2T5.0, i64 0, i64 2), ptr %7, align 8, !tbaa !48
  store ptr %6, ptr %8, align 8, !tbaa !61
  br label %14

14:                                               ; preds = %13, %12, %11, %10, %9
  %15 = add nuw nsw i64 %3, 1
  %16 = icmp eq i64 %15, 1000
  br i1 %16, label %17, label %2, !llvm.loop !63

17:                                               ; preds = %14
  %18 = tail call noalias dereferenceable_or_null(16000) ptr @malloc(i64 noundef 16000) #15
  %19 = tail call noalias dereferenceable_or_null(16000) ptr @malloc(i64 noundef 16000) #15
  br label %20

20:                                               ; preds = %20, %17
  %21 = phi i64 [ 0, %17 ], [ %34, %20 ]
  %22 = tail call i32 @rand() #13
  %23 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 %21, i32 0
  store i32 %22, ptr %23, align 4, !tbaa !43
  %24 = tail call i32 @rand() #13
  %25 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 %21, i32 1
  store i32 %24, ptr %25, align 4, !tbaa !55
  %26 = tail call i32 @rand() #13
  %27 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 %21, i32 2
  store i32 %26, ptr %27, align 4, !tbaa !57
  %28 = tail call i32 @rand() #13
  %29 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %19, i64 %21, i32 0
  store i32 %28, ptr %29, align 4, !tbaa !43
  %30 = tail call i32 @rand() #13
  %31 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %19, i64 %21, i32 1
  store i32 %30, ptr %31, align 4, !tbaa !55
  %32 = tail call i32 @rand() #13
  %33 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %19, i64 %21, i32 2
  store i32 %32, ptr %33, align 4, !tbaa !57
  %34 = add nuw nsw i64 %21, 1
  %35 = icmp eq i64 %34, 1000
  br i1 %35, label %36, label %20, !llvm.loop !64

36:                                               ; preds = %20
  %37 = getelementptr inbounds [1000 x ptr], ptr %1, i64 0, i64 0
  %38 = call fastcc noundef zeroext i1 @_Z5checkP2S1S0_PP2S2(ptr noundef nonnull %18, ptr noundef nonnull %19, ptr noundef nonnull %37)
  br i1 %38, label %39, label %55

39:                                               ; preds = %36
  %40 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 0, !intel-tbaa !43
  %41 = load i32, ptr %40, align 4, !tbaa !43
  %42 = icmp eq i32 %41, 0
  br i1 %42, label %55, label %43

43:                                               ; preds = %39
  %44 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 1, !intel-tbaa !55
  %45 = load i32, ptr %44, align 4, !tbaa !55
  %46 = icmp eq i32 %45, 0
  br i1 %46, label %49, label %47

47:                                               ; preds = %43
  %48 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.4)
  br label %55

49:                                               ; preds = %43
  %50 = getelementptr inbounds %struct._ZTS2S1.S1, ptr %18, i64 0, i32 2, !intel-tbaa !57
  %51 = load i32, ptr %50, align 4, !tbaa !57
  %52 = icmp eq i32 %51, 0
  br i1 %52, label %55, label %53

53:                                               ; preds = %49
  %54 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %55

55:                                               ; preds = %53, %49, %47, %39, %36
  %56 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.5)
  %57 = load i32, ptr @count, align 4, !tbaa !65
  %58 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.3, i32 noundef %57)
  call void @llvm.lifetime.end.p0(i64 8000, ptr nonnull %1) #13
  ret i32 0
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #4

; Function Attrs: nofree nounwind
declare dso_local i32 @rand() local_unnamed_addr #5

; Function Attrs: nobuiltin allocsize(0)
declare !intel.dtrans.func.type !66 dso_local noundef nonnull "intel_dtrans_func_index"="1" ptr @_Znwm(i64 noundef) local_unnamed_addr #6

; Function Attrs: mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite)
declare !intel.dtrans.func.type !67 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #7

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #0

; Function Attrs: nofree nounwind
declare !intel.dtrans.func.type !68 dso_local noundef i32 @printf(ptr nocapture noundef readonly "intel_dtrans_func_index"="1", ...) local_unnamed_addr #5

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #4

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, argmem: read, inaccessiblemem: none) uwtable
define internal noundef zeroext i1 @_ZN2T54testEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %0) unnamed_addr #8 comdat align 2 !intel.dtrans.func.type !69 {
  %2 = load i32, ptr @count, align 4, !tbaa !65
  %3 = add nsw i32 %2, 1
  store i32 %3, ptr @count, align 4, !tbaa !65
  %4 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 2, !intel-tbaa !71
  %5 = load i32, ptr %4, align 4, !tbaa !71
  %6 = add nsw i32 %5, -30
  %7 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 1, !intel-tbaa !73
  %8 = load i32, ptr %7, align 8, !tbaa !73
  %9 = icmp sgt i32 %6, %8
  ret i1 %9
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define internal noundef zeroext i1 @_ZN2T44testEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %0) unnamed_addr #9 comdat align 2 !intel.dtrans.func.type !74 {
  %2 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 2, !intel-tbaa !71
  %3 = load i32, ptr %2, align 4, !tbaa !71
  %4 = add nsw i32 %3, -20
  %5 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 1, !intel-tbaa !73
  %6 = load i32, ptr %5, align 8, !tbaa !73
  %7 = icmp sgt i32 %4, %6
  ret i1 %7
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define internal noundef zeroext i1 @_ZN2T34testEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %0) unnamed_addr #9 comdat align 2 !intel.dtrans.func.type !76 {
  %2 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 1, !intel-tbaa !73
  %3 = load i32, ptr %2, align 8, !tbaa !73
  %4 = add nsw i32 %3, 2
  %5 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 2, !intel-tbaa !71
  %6 = load i32, ptr %5, align 4, !tbaa !71
  %7 = icmp sgt i32 %4, %6
  ret i1 %7
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define internal noundef zeroext i1 @_ZN2T24testEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %0) unnamed_addr #9 comdat align 2 !intel.dtrans.func.type !78 {
  %2 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 2, !intel-tbaa !71
  %3 = load i32, ptr %2, align 4, !tbaa !71
  %4 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 1, !intel-tbaa !73
  %5 = load i32, ptr %4, align 8, !tbaa !73
  %6 = add nsw i32 %5, 10
  %7 = icmp sgt i32 %3, %6
  ret i1 %7
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define internal noundef zeroext i1 @_ZN2T14testEv(ptr nocapture noundef nonnull readonly align 8 dereferenceable(16) "intel_dtrans_func_index"="1" %0) unnamed_addr #9 comdat align 2 !intel.dtrans.func.type !80 {
  %2 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 1, !intel-tbaa !73
  %3 = load i32, ptr %2, align 8, !tbaa !73
  %4 = getelementptr inbounds %class._ZTS2S2.S2, ptr %0, i64 0, i32 2, !intel-tbaa !71
  %5 = load i32, ptr %4, align 4, !tbaa !71
  %6 = icmp sgt i32 %3, %5
  ret i1 %6
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.type.test(ptr, metadata) #10

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn
define hidden void @__typeid__ZTS2S2_0_branch_funnel(ptr nest %0, ...) local_unnamed_addr #11 {
  musttail call void (...) @llvm.icall.branch.funnel(ptr %0, ptr nonnull getelementptr inbounds ([3 x ptr], ptr @_ZTV2T1.0, i64 0, i64 2), ptr nonnull @_ZN2T14testEv, ptr nonnull getelementptr inbounds ([3 x ptr], ptr @_ZTV2T2.0, i64 0, i64 2), ptr nonnull @_ZN2T24testEv, ptr nonnull getelementptr inbounds ([3 x ptr], ptr @_ZTV2T3.0, i64 0, i64 2), ptr nonnull @_ZN2T34testEv, ptr nonnull getelementptr inbounds ([3 x ptr], ptr @_ZTV2T4.0, i64 0, i64 2), ptr nonnull @_ZN2T44testEv, ptr nonnull getelementptr inbounds ([3 x ptr], ptr @_ZTV2T5.0, i64 0, i64 2), ptr nonnull @_ZN2T54testEv, ...)
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn
declare void @llvm.icall.branch.funnel(...) #12

attributes #0 = { nofree nounwind }
attributes #1 = { mustprogress noinline norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: write) }
attributes #3 = { norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #5 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { nobuiltin allocsize(0) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #7 = { mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) memory(inaccessiblemem: readwrite) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #8 = { mustprogress nofree norecurse nosync nounwind willreturn memory(readwrite, argmem: read, inaccessiblemem: none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #9 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cmov,+crc32,+cx16,+cx8,+evex512,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #10 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #11 = { mustprogress nofree norecurse nosync nounwind willreturn }
attributes #12 = { nocallback nofree nosync nounwind willreturn }
attributes #13 = { nounwind }
attributes #14 = { builtin allocsize(0) }
attributes #15 = { nounwind allocsize(0) }

!intel.dtrans.types = !{!18, !20, !23, !25, !26, !27, !28}
!llvm.ident = !{!29}
!llvm.module.flags = !{!30, !31, !32, !33, !34}

!0 = !{!"A", i32 0, !1}
!1 = !{i8 0, i32 1}
!2 = !{!"L", i32 2, !1, !1}
!3 = !{!"L", i32 3, !1, !1, !1}
!4 = !{i32 16, !"_ZTS2S2"}
!5 = !{i32 16, !"_ZTSM2S2FbvE.virtual"}
!6 = !{i32 16, !"_ZTS2T1"}
!7 = !{i32 16, !"_ZTSM2T1FbvE.virtual"}
!8 = !{!"A", i32 3, !1}
!9 = !{i32 16, !"_ZTS2T2"}
!10 = !{i32 16, !"_ZTSM2T2FbvE.virtual"}
!11 = !{i32 16, !"_ZTS2T3"}
!12 = !{i32 16, !"_ZTSM2T3FbvE.virtual"}
!13 = !{i32 16, !"_ZTS2T4"}
!14 = !{i32 16, !"_ZTSM2T4FbvE.virtual"}
!15 = !{i32 16, !"_ZTS2T5"}
!16 = !{i32 16, !"_ZTSM2T5FbvE.virtual"}
!17 = !{!"A", i32 1, !1}
!18 = !{!"S", %struct._ZTS2S1.S1 zeroinitializer, i32 4, !19, !19, !19, !19}
!19 = !{i32 0, i32 0}
!20 = !{!"S", %class._ZTS2S2.S2 zeroinitializer, i32 3, !21, !19, !19}
!21 = !{!22, i32 2}
!22 = !{!"F", i1 true, i32 0, !19}
!23 = !{!"S", %class._ZTS2T1.T1 zeroinitializer, i32 1, !24}
!24 = !{%class._ZTS2S2.S2 zeroinitializer, i32 0}
!25 = !{!"S", %class._ZTS2T2.T2 zeroinitializer, i32 1, !24}
!26 = !{!"S", %class._ZTS2T3.T3 zeroinitializer, i32 1, !24}
!27 = !{!"S", %class._ZTS2T4.T4 zeroinitializer, i32 1, !24}
!28 = !{!"S", %class._ZTS2T5.T5 zeroinitializer, i32 1, !24}
!29 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.1.0 (2024.x.0.YYYYMMDD)"}
!30 = !{i32 1, !"wchar_size", i32 4}
!31 = !{i32 1, !"Virtual Function Elim", i32 0}
!32 = !{i32 7, !"uwtable", i32 2}
!33 = !{i32 1, !"ThinLTO", i32 0}
!34 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!35 = distinct !{!36, !36, !37}
!36 = !{%struct._ZTS2S1.S1 zeroinitializer, i32 1}
!37 = !{%class._ZTS2S2.S2 zeroinitializer, i32 2}
!38 = !{!39, !40, i64 12}
!39 = !{!"struct@_ZTS2S1", !40, i64 0, !40, i64 4, !40, i64 8, !40, i64 12}
!40 = !{!"int", !41, i64 0}
!41 = !{!"omnipotent char", !42, i64 0}
!42 = !{!"Simple C++ TBAA"}
!43 = !{!39, !40, i64 0}
!44 = distinct !{!44, !45}
!45 = !{!"llvm.loop.mustprogress"}
!46 = !{!47, !47, i64 0}
!47 = !{!"pointer@_ZTSP2S2", !41, i64 0}
!48 = !{!49, !49, i64 0}
!49 = !{!"vtable pointer", !42, i64 0}
!50 = !{!"F", i1 false, i32 1, !51, !52}
!51 = !{i1 false, i32 0}
!52 = !{%class._ZTS2S2.S2 zeroinitializer, i32 1}
!53 = distinct !{!53, !45}
!54 = distinct !{!54, !45}
!55 = !{!39, !40, i64 4}
!56 = distinct !{!56, !45}
!57 = !{!39, !40, i64 8}
!58 = distinct !{!58, !45}
!59 = distinct !{!59, !45}
!60 = !{!"A", i32 1000, !52}
!61 = !{!62, !47, i64 0}
!62 = !{!"array@_ZTSA1000_P2S2", !47, i64 0}
!63 = distinct !{!63, !45}
!64 = distinct !{!64, !45}
!65 = !{!40, !40, i64 0}
!66 = distinct !{!1}
!67 = distinct !{!1}
!68 = distinct !{!1}
!69 = distinct !{!70}
!70 = !{%class._ZTS2T5.T5 zeroinitializer, i32 1}
!71 = !{!72, !40, i64 12}
!72 = !{!"struct@_ZTS2S2", !40, i64 8, !40, i64 12}
!73 = !{!72, !40, i64 8}
!74 = distinct !{!75}
!75 = !{%class._ZTS2T4.T4 zeroinitializer, i32 1}
!76 = distinct !{!77}
!77 = !{%class._ZTS2T3.T3 zeroinitializer, i32 1}
!78 = distinct !{!79}
!79 = !{%class._ZTS2T2.T2 zeroinitializer, i32 1}
!80 = distinct !{!81}
!81 = !{%class._ZTS2T1.T1 zeroinitializer, i32 1}

; end INTEL_FEATURE_SW_DTRANS
