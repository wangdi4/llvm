; Verify that GVN-PRE splits a partially redundant load, for the case in
; CMPLRLLVM-10341.
; The correctness of the transformation is covered by other GVN tests. The
; commit in this case is a cost model change only.

; RUN: opt < %s -gvn -S | FileCheck %s

; CHECK-LABEL: 26:
; CHECK: icmp ult
; CHECK: [[SEL:%[0-9]+]] = select i1
; CHECK-NEXT: [[ZEXT:%[0-9]+]] = zext i32 [[SEL]]
; CHECK: %.pre-phi = phi{{.*}}[[ZEXT]]
; CHECK-DAG: [[GEP1:%[0-9]+]] = getelementptr{{.*}}%.pre-phi
; CHECK-DAG: [[GEP2:%[0-9]+]] = getelementptr{{.*}}%.pre-phi
; CHECK: load{{.*}}[[GEP2]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.lzma_match = type { i32, i32 }

@0 = private unnamed_addr constant [16 x i8] c"padded 32 bytes\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"ld-temp.o\00", align 1

@__Intel_PaddedMallocCounter = unnamed_addr global i32 0

; This function's existence enables the PRE cost model change.
define dso_local i1 @__Intel_PaddedMallocInterface() local_unnamed_addr {
  %1 = load i32, i32* @__Intel_PaddedMallocCounter, align 4
  %2 = icmp ult i32 %1, 250
  ret i1 %2
}

; Function Attrs: nofree noinline norecurse nounwind uwtable
define dso_local fastcc %struct.lzma_match* @bt_find_func(i32 %0, i32 %1, i8* nocapture readonly %2, i32 %3, i32 %4, i32* nocapture %5, i32 %6, i32 %7, %struct.lzma_match* %8, i32 %9) unnamed_addr #0 {
  %11 = tail call i8* @llvm.ptr.annotation.p0i8(i8* %2, i8* getelementptr inbounds ([16 x i8], [16 x i8]* @0, i64 0, i64 0), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @1, i64 0, i64 0), i32 0)
  %12 = shl i32 %6, 1
  %13 = zext i32 %12 to i64
  %14 = getelementptr inbounds i32, i32* %5, i64 %13, !intel-tbaa !5
  %15 = getelementptr inbounds i32, i32* %14, i64 1, !intel-tbaa !5
  %16 = sub i32 %1, %3
  %17 = icmp ne i32 %4, 0
  %18 = icmp ult i32 %16, %7
  %19 = and i1 %17, %18
  br i1 %19, label %20, label %25

20:                                               ; preds = %10
  br label %29

21:                                               ; preds = %106
  %22 = phi i32* [ %107, %106 ]
  %23 = phi i32* [ %108, %106 ]
  %24 = phi %struct.lzma_match* [ %94, %106 ]
  br label %25

25:                                               ; preds = %21, %10
  %26 = phi %struct.lzma_match* [ %8, %10 ], [ %24, %21 ]
  %27 = phi i32* [ %15, %10 ], [ %22, %21 ]
  %28 = phi i32* [ %14, %10 ], [ %23, %21 ]
  store i32 0, i32* %27, align 4, !tbaa !5
  store i32 0, i32* %28, align 4, !tbaa !5
  br label %117

29:                                               ; preds = %106, %20
  %30 = phi i32 [ %39, %106 ], [ %4, %20 ]
  %31 = phi i32 [ %113, %106 ], [ %16, %20 ]
  %32 = phi i32 [ %112, %106 ], [ %3, %20 ]
  %33 = phi i32 [ %110, %106 ], [ 0, %20 ]
  %34 = phi i32 [ %109, %106 ], [ 0, %20 ]
  %35 = phi i32* [ %108, %106 ], [ %14, %20 ]
  %36 = phi i32* [ %107, %106 ], [ %15, %20 ]
  %37 = phi i32 [ %95, %106 ], [ %9, %20 ]
  %38 = phi %struct.lzma_match* [ %94, %106 ], [ %8, %20 ]
  %39 = add i32 %30, -1
  %40 = sub i32 %6, %31
  %41 = icmp ugt i32 %31, %6
  %42 = select i1 %41, i32 %7, i32 0
  %43 = add i32 %40, %42
  %44 = shl i32 %43, 1
  %45 = zext i32 %44 to i64
  %46 = getelementptr inbounds i32, i32* %5, i64 %45, !intel-tbaa !5
  %47 = zext i32 %31 to i64
  %48 = sub nsw i64 0, %47
  %49 = getelementptr inbounds i8, i8* %11, i64 %48, !intel-tbaa !9
  %50 = icmp ult i32 %34, %33
  %51 = select i1 %50, i32 %34, i32 %33
  %52 = zext i32 %51 to i64
  %53 = getelementptr inbounds i8, i8* %49, i64 %52
  %54 = load i8, i8* %53, align 1, !tbaa !9
  %55 = getelementptr inbounds i8, i8* %11, i64 %52
  %56 = load i8, i8* %55, align 1, !tbaa !9
  %57 = icmp eq i8 %54, %56
  br i1 %57, label %58, label %93

58:                                               ; preds = %29
  %59 = add i32 %51, 1
  %60 = icmp eq i32 %59, %0
  br i1 %60, label %76, label %61

61:                                               ; preds = %58
  br label %65

62:                                               ; preds = %65
  %63 = add i32 %66, 1
  %64 = icmp eq i32 %63, %0
  br i1 %64, label %73, label %65

65:                                               ; preds = %62, %61
  %66 = phi i32 [ %63, %62 ], [ %59, %61 ]
  %67 = zext i32 %66 to i64
  %68 = getelementptr inbounds i8, i8* %49, i64 %67
  %69 = load i8, i8* %68, align 1, !tbaa !9
  %70 = getelementptr inbounds i8, i8* %11, i64 %67
  %71 = load i8, i8* %70, align 1, !tbaa !9
  %72 = icmp eq i8 %69, %71
  br i1 %72, label %62, label %73

73:                                               ; preds = %65, %62
  %74 = phi i32 [ %66, %65 ], [ %0, %62 ]
  %75 = phi i1 [ false, %65 ], [ true, %62 ]
  br label %76

76:                                               ; preds = %73, %58
  %77 = phi i32 [ %0, %58 ], [ %74, %73 ]
  %78 = phi i1 [ true, %58 ], [ %75, %73 ]
  %79 = icmp ult i32 %37, %77
  br i1 %79, label %80, label %93

80:                                               ; preds = %76
  %81 = getelementptr inbounds %struct.lzma_match, %struct.lzma_match* %38, i64 0, i32 0, !intel-tbaa !10
  store i32 %77, i32* %81, align 4, !tbaa !10
  %82 = add i32 %31, -1
  %83 = getelementptr inbounds %struct.lzma_match, %struct.lzma_match* %38, i64 0, i32 1, !intel-tbaa !12
  store i32 %82, i32* %83, align 4, !tbaa !12
  %84 = getelementptr inbounds %struct.lzma_match, %struct.lzma_match* %38, i64 1
  br i1 %78, label %85, label %93

85:                                               ; preds = %80
  %86 = phi i32* [ %35, %80 ]
  %87 = phi i32* [ %36, %80 ]
  %88 = phi i32* [ %46, %80 ]
  %89 = phi %struct.lzma_match* [ %84, %80 ]
  %90 = load i32, i32* %88, align 4, !tbaa !5
  store i32 %90, i32* %86, align 4, !tbaa !5
  %91 = getelementptr inbounds i32, i32* %88, i64 1
  %92 = load i32, i32* %91, align 4, !tbaa !5
  store i32 %92, i32* %87, align 4, !tbaa !5
  br label %117

93:                                               ; preds = %80, %76, %29
  %94 = phi %struct.lzma_match* [ %84, %80 ], [ %38, %76 ], [ %38, %29 ]
  %95 = phi i32 [ %77, %80 ], [ %37, %76 ], [ %37, %29 ]
  %96 = phi i32 [ %77, %80 ], [ %77, %76 ], [ %51, %29 ]
  %97 = zext i32 %96 to i64
  %98 = getelementptr inbounds i8, i8* %49, i64 %97
  %99 = load i8, i8* %98, align 1, !tbaa !9
  %100 = getelementptr inbounds i8, i8* %11, i64 %97
  %101 = load i8, i8* %100, align 1, !tbaa !9
  %102 = icmp ult i8 %99, %101
  br i1 %102, label %103, label %105

103:                                              ; preds = %93
  store i32 %32, i32* %35, align 4, !tbaa !5
  %104 = getelementptr inbounds i32, i32* %46, i64 1, !intel-tbaa !5
  br label %106

105:                                              ; preds = %93
  store i32 %32, i32* %36, align 4, !tbaa !5
  br label %106

106:                                              ; preds = %105, %103
  %107 = phi i32* [ %36, %103 ], [ %46, %105 ]
  %108 = phi i32* [ %104, %103 ], [ %35, %105 ]
  %109 = phi i32 [ %34, %103 ], [ %96, %105 ]
  %110 = phi i32 [ %96, %103 ], [ %33, %105 ]
  %111 = phi i32* [ %104, %103 ], [ %46, %105 ]
  %112 = load i32, i32* %111, align 4, !tbaa !5
  %113 = sub i32 %1, %112
  %114 = icmp ne i32 %39, 0
  %115 = icmp ult i32 %113, %7
  %116 = and i1 %114, %115
  br i1 %116, label %29, label %21

117:                                              ; preds = %85, %25
  %118 = phi %struct.lzma_match* [ %26, %25 ], [ %89, %85 ]
  ret %struct.lzma_match* %118
}

; Function Attrs: nounwind willreturn
declare i8* @llvm.ptr.annotation.p0i8(i8*, i8*, i8*, i32) #1

attributes #0 = { nofree noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind willreturn }


!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!7, !7, i64 0}
!10 = !{!11, !6, i64 0}
!11 = !{!"struct@", !6, i64 0, !6, i64 4}
!12 = !{!11, !6, i64 4}
