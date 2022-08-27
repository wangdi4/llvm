; RUN: opt < %s -opaque-pointers -whole-program-assume -dtrans-paddedmallocop -dtrans-test-paddedmalloc -S 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -passes=dtrans-paddedmallocop -dtrans-test-paddedmalloc  -S 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -dtrans-paddedmallocop -padded-pointer-prop-op -S 2>&1 | FileCheck %s --check-prefix=CHECK-PROP
; RUN: opt < %s -opaque-pointers -whole-program-assume -passes="dtrans-paddedmallocop,padded-pointer-prop-op" -S 2>&1 | FileCheck %s --check-prefix=CHECK-PROP

; Check for padded malloc counter

; CHECK: @__Intel_PaddedMallocCounter = internal global i32 0

; Check for padding

; CHECK-PROP: [[PADD:@.*]] = private unnamed_addr constant [16 x i8] c"padded 32 bytes\00"

; Check transformed form of @mallocFunc

; CHECK-LABEL: @mallocFunc(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP2:%.*]] = icmp ult i64 [[TMP0:%.*]], 4294967295
; CHECK-NEXT:    br i1 [[TMP2:%.*]], label [[TMP3:%.*]], label %MaxBB
; CHECK-LABEL: 1:
; CHECK-NEXT:    [[TMP4:%.*]] = load i32, ptr @__Intel_PaddedMallocCounter
; CHECK-NEXT:    [[TMP5:%.*]] = icmp ult i32 [[TMP4]], 250
; CHECK-NEXT:    br i1 [[TMP5]], label [[BBIF:%.*]], label [[BBELSE:%.*]]
; CHECK-LABEL: MaxBB:
; CHECK-NEXT:    store i32 250, ptr @__Intel_PaddedMallocCounter
; CHECK-NEXT:    br label %BBelse
; CHECK-LABEL: BBif:
; CHECK-NEXT:    [[TMP6:%.*]] = add i64 [[TMP0:%.*]], 32
; CHECK-NEXT:    [[TMP7:%.*]] = tail call noalias align 16 ptr @malloc(i64 [[TMP6]])
; CHECK-NEXT:    [[TMP8:%.*]] = add i32 1, [[TMP4]]
; CHECK-NEXT:    store i32 [[TMP8]], ptr @__Intel_PaddedMallocCounter
; CHECK-NEXT:    br label [[TMP10:%.*]]
; CHECK-LABEL: BBelse:
; CHECK-NEXT:    [[TMP9:%.*]] = tail call noalias align 16 ptr @malloc(i64 [[TMP0]])
; CHECK-NEXT:    br label [[TMP10]]
; CHECK-LABEL: 8:
; CHECK-NEXT:    [[TMP11:%.*]] = phi ptr [ [[TMP7]], [[BBIF]] ], [ [[TMP9]], [[BBELSE]] ]
; CHECK-NEXT:    ret ptr [[TMP11]]

; Check transformed form of @main (without propagation)

; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP1:%.*]] = tail call ptr @mallocFunc(i64 100)
; CHECK-NEXT:    store ptr [[TMP1]], ptr @globalStruct, align 8
; CHECK-NEXT:    [[T:%.*]] = load ptr, ptr @globalStruct, align 8
; CHECK-NEXT:    tail call void @free(ptr [[TMP1]])
; CHECK-NEXT:    store ptr null, ptr @globalStruct, align 8
; CHECK-NEXT:    [[TMP2:%.*]] = call zeroext i1 @searchloop()
; CHECK-NEXT:    ret i32 0

; Check transformed form of @main (with propagation)

; CHECK-PROP-LABEL: @main(
; CHECK-PROP-NEXT:  entry:
; CHECK-PROP-NEXT:    [[TMP1:%.*]] = tail call ptr @mallocFunc(i64 100)
; CHECK-PROP-NEXT:    store ptr [[TMP1]], ptr @globalStruct, align 8
; CHECK-PROP-NEXT:    [[T:%.*]] = load ptr, ptr @globalStruct, align 8
; CHECK-PROP-NEXT:    [[T1:%.*]] = call ptr @llvm.ptr.annotation.p0(ptr [[T]], ptr [[PADD]], ptr @1, i32 0, ptr null)
; CHECK-PROP-NEXT:    tail call void @free(ptr [[TMP1]])
; CHECK-PROP-NEXT:    store ptr null, ptr @globalStruct, align 8
; CHECK-PROP-NEXT:    [[TMP2:%.*]] = call zeroext i1 @searchloop()
; CHECK-PROP-NEXT:    ret i32 0

; Check that the interface was created correctly

; CHECK: define i1 @__Intel_PaddedMallocInterface() !dtrans.paddedmallocsize ![[M0:[0-9]+]] {
; CHECK: entry:
; CHECK:   [[TMP0:%.*]] = load i32, ptr @__Intel_PaddedMallocCounter
; CHECK:   [[TMP1:%.*]] = icmp ult i32 [[TMP0:%.*]], 250
; CHECK:   ret i1 [[TMP1:%.*]]
; CHECK: }
; CHECK: ![[M0]] = !{i32 32}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10testStruct.testStruct = type { ptr }

@globalStruct = internal local_unnamed_addr global %struct._ZTS10testStruct.testStruct zeroinitializer, align 8
@arr1 = internal local_unnamed_addr global [10 x i32] zeroinitializer, align 16, !intel_dtrans_type !0
@arr2 = internal local_unnamed_addr global [10 x i32] zeroinitializer, align 16, !intel_dtrans_type !0

; Function Attrs: mustprogress nofree noinline nounwind uwtable willreturn
define internal noalias "intel_dtrans_func_index"="1" ptr @mallocFunc(i64 %size) local_unnamed_addr #0 !intel.dtrans.func.type !10 {
entry:
  %call = tail call noalias align 16 ptr @malloc(i64 %size) #5
  ret ptr %call
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !11 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: nofree norecurse nosync nounwind readonly uwtable
define internal zeroext i1 @searchloop() local_unnamed_addr #2 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.cond ]
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr @arr1, i64 0, i64 %idxprom, !intel-tbaa !12
  %i = load i32, ptr %arrayidx, align 4, !tbaa !12
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr @arr2, i64 0, i64 %idxprom, !intel-tbaa !12
  %i1 = load i32, ptr %arrayidx2, align 4, !tbaa !12
  %cmp = icmp eq i32 %i, %i1
  %inc = add i32 %i.0, 1
  br i1 %cmp, label %if.then, label %for.cond

if.then:                                          ; preds = %for.cond
  ret i1 true
}

; Function Attrs: mustprogress nounwind uwtable willreturn
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %call = tail call ptr @mallocFunc(i64 100)
  store ptr %call, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8
  %t = load ptr, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8
  tail call void @free(ptr %call) #5
  store ptr null, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8, !tbaa !17
  call zeroext i1 @searchloop()
  ret i32 0
}

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn
declare !intel.dtrans.func.type !20 dso_local void @free(ptr nocapture noundef "intel_dtrans_func_index"="1") local_unnamed_addr #4

attributes #0 = { mustprogress nofree noinline nounwind uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nofree norecurse nosync nounwind readonly uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { mustprogress nounwind uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!2, !3, !4, !5, !6}
!intel.dtrans.types = !{!7}
!llvm.ident = !{!9}

!0 = !{!"A", i32 10, !1}
!1 = !{i32 0, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 1, !"Virtual Function Elim", i32 0}
!4 = !{i32 7, !"uwtable", i32 1}
!5 = !{i32 1, !"ThinLTO", i32 0}
!6 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!7 = !{!"S", %struct._ZTS10testStruct.testStruct zeroinitializer, i32 1, !8}
!8 = !{i8 0, i32 1}
!9 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!10 = distinct !{!8}
!11 = distinct !{!8}
!12 = !{!13, !14, i64 0}
!13 = !{!"array@_ZTSA10_i", !14, i64 0}
!14 = !{!"int", !15, i64 0}
!15 = !{!"omnipotent char", !16, i64 0}
!16 = !{!"Simple C/C++ TBAA"}
!17 = !{!18, !19, i64 0}
!18 = !{!"struct@", !19, i64 0}
!19 = !{!"pointer@_ZTSPv", !15, i64 0}
!20 = distinct !{!8}
