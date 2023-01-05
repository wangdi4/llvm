; REQUIRES: asserts

; Test that identifies that the DTrans padded malloc optimization didn't
; apply the optimization since it didn't find a malloc function, even
; though a search loop is available.

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed < %s -passes=dtrans-paddedmallocop -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s

; CHECK: dtrans-paddedmalloc: Trace for DTrans Padded Malloc
; CHECK: dtrans-paddedmalloc: Identifying alloc functions
; CHECK: No alloc functions found

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10testStruct.testStruct = type { ptr }

@globalStruct = dso_local local_unnamed_addr global %struct._ZTS10testStruct.testStruct zeroinitializer, align 8
@arr1 = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16, !intel_dtrans_type !0
@arr2 = dso_local local_unnamed_addr global [10 x i32] zeroinitializer, align 16, !intel_dtrans_type !0

; Function Attrs: mustprogress uwtable
define dso_local "intel_dtrans_func_index"="1" ptr @_Z10mallocFunclPFPclE(i64 %size, ptr "intel_dtrans_func_index"="2" %func) local_unnamed_addr #0 !intel.dtrans.func.type !10 {
entry:
  %tobool.not = icmp eq ptr %func, null
  br i1 %tobool.not, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  %call = tail call ptr %func(i64 %size), !intel_dtrans_type !12
  br label %if.end

if.else:                                          ; preds = %entry
  %call1 = tail call ptr @_Z6mallocl(i64 %size)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %myptr.0 = phi ptr [ %call, %if.then ], [ %call1, %if.else ]
  ret ptr %myptr.0
}

declare !intel.dtrans.func.type !14 dso_local "intel_dtrans_func_index"="1" ptr @_Z6mallocl(i64) local_unnamed_addr #1

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define dso_local zeroext i1 @_Z10searchloopv() local_unnamed_addr #2 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.cond ]
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr @arr1, i64 0, i64 %idxprom, !intel-tbaa !15
  %i = load i32, ptr %arrayidx, align 4, !tbaa !15
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr @arr2, i64 0, i64 %idxprom, !intel-tbaa !15
  %i1 = load i32, ptr %arrayidx2, align 4, !tbaa !15
  %cmp = icmp eq i32 %i, %i1
  %inc = add i32 %i.0, 1
  br i1 %cmp, label %if.then, label %for.cond, !llvm.loop !20

if.then:                                          ; preds = %for.cond
  ret i1 true
}

; Function Attrs: mustprogress norecurse uwtable
define dso_local i32 @main() local_unnamed_addr #3 {
entry:
  %call1.i = tail call ptr @_Z6mallocl(i64 100)
  store ptr %call1.i, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8, !tbaa !15
  tail call void @_Z4freePc(ptr %call1.i)
  store ptr null, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8, !tbaa !15
  ret i32 0
}

declare !intel.dtrans.func.type !20 dso_local void @_Z4freePc(ptr "intel_dtrans_func_index"="1") local_unnamed_addr #1

attributes #0 = { mustprogress uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { mustprogress norecurse uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

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
!10 = distinct !{!8, !11}
!11 = !{!12, i32 1}
!12 = !{!"F", i1 false, i32 1, !8, !13}
!13 = !{i64 0, i32 0}
!14 = distinct !{!8}
!15 = !{!16, !17, i64 0}
!16 = !{!"struct@_ZTS10testStruct", !17, i64 0}
!17 = !{!"pointer@_ZTSPc", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C++ TBAA"}
!20 = distinct !{!8}
